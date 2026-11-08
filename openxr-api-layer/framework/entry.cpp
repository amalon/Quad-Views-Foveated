// MIT License
//
// Copyright(c) 2021-2023 Matthieu Bucchianeri
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pch.h"

#include <layer.h>

#include "dispatch.h"
#include "log.h"
#include "version.h"
#include "config.h"

#if defined(_WIN32)
#define OPENXR_API_LAYER_EXPORT __declspec(dllexport)
#else
#define OPENXR_API_LAYER_EXPORT
#endif

namespace openxr_api_layer {
    // The path where the DLL is loaded from (eg: to load data files).
    std::filesystem::path dllHome;

    // The paths where user configuration is stored
    std::vector<std::filesystem::path> configDirs;
    // The paths to search for data files
    std::vector<std::filesystem::path> dataDirs;
    // The path that is writable (eg: to store logs).
    std::filesystem::path stateHome;

    namespace log {
        // The file logger.
        std::ofstream logStream;
    } // namespace log

    const std::string VersionString = fmt::format("v{}.{}.{}", LayerVersionMajor, LayerVersionMinor, LayerVersionPatch);

} // namespace openxr_api_layer

using namespace openxr_api_layer;
using namespace openxr_api_layer::log;

extern "C" {

// Entry point for the loader.
XrResult OPENXR_API_LAYER_EXPORT XRAPI_CALL
    xrNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo* const loaderInfo,
                                       const char* const apiLayerName,
                                       XrNegotiateApiLayerRequest* const apiLayerRequest) {
    TraceLoggingWrite(g_traceProvider, "xrNegotiateLoaderApiLayerInterface");

#ifdef _WIN32
    // Retrieve the path of the DLL.
    if (dllHome.empty()) {
        HMODULE module;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)&dllHome,
                               &module)) {
            char path[_MAX_PATH];
            GetModuleFileNameA(module, path, sizeof(path));
            dllHome = std::filesystem::path(path).parent_path();
        }
    }

    if (stateHome.empty())
        stateHome = std::filesystem::path(getenv("LOCALAPPDATA")) / LayerPrettyName;
    if (dataDirs.empty())
        dataDirs.push_back(dllHome);
    if (configDirs.empty())
        configDirs.push_back(stateHome);
#else
    // Get a path from an environment variable, falling back to fallback
    const auto getEnvPath = [&](const char* envName,
                                const std::filesystem::path& fallback) {
        const char* val = getenv(envName);
        if (val && val[0] != '\0')
            return std::filesystem::path{val};
        return fallback;
    };
    // Get a list of paths from an environment variable, separated by ':'
    const auto getEnvPaths = [&](const char* envName,
                                 const std::vector<std::filesystem::path>& fallback) {
        const char* val = getenv(envName);
        if (val && val[0] != '\0') {
            std::vector<std::filesystem::path> ret;
            std::stringstream ss(val);
            std::string path;
            while (getline(ss, path, ':')) {
                ret.push_back(std::filesystem::path(path));
            }
            return ret;
        }
        return fallback;
    };
    // Push a path to the back, but only if its not already there
    const auto pushUnique = [&](std::vector<std::filesystem::path>& list,
                                const std::filesystem::path& item) {
        if (std::find(list.begin(), list.end(), item) == list.end())
            list.push_back(item);
    };

    // https://specifications.freedesktop.org/basedir/latest/
    const auto home = getEnvPath("HOME", "");

    // Configuration paths
    if (configDirs.empty()) {
        // $XDG_CONFIG_HOME, then $XDG_CONFIG_DIRS
        const auto xdgConfigHome = getEnvPath("XDG_CONFIG_HOME",
                                              home / ".config");
        pushUnique(configDirs, xdgConfigHome / LayerPrettyName);
        for (auto& path: getEnvPaths("XDG_CONFIG_DIRS",
                                     { "/etc/xdg/" })) {
            pushUnique(configDirs, path / LayerPrettyName);
        }
    }

    // Data paths
    if (dataDirs.empty()) {
        // First the install directory from the build system
        const auto dataDir = std::filesystem::path(CONFIG_DATADIR);
        pushUnique(dataDirs, dataDir / LayerPrettyName);
        // Then $XDG_DATA_HOME and $XDG_DATA_DIRS
        const auto xdgDataHome = getEnvPath("XDG_DATA_HOME",
                                            home / ".local/share");
        pushUnique(dataDirs, xdgDataHome / LayerPrettyName);
        for (auto& path: getEnvPaths("XDG_DATA_DIRS",
                                     { "/usr/local/share/", "/usr/share/" }))
            pushUnique(dataDirs, path / LayerPrettyName);
    }

    // State path
    if (stateHome.empty()) {
        // Use $XDG_STATE_HOME
        const auto xdgStateHome = getEnvPath("XDG_STATE_HOME",
                                             home / ".local" / "state");
        stateHome = xdgStateHome / LayerPrettyName;
    }
#endif

    std::filesystem::create_directory(stateHome.string());

    // Start logging to file.
    if (!logStream.is_open()) {
        std::string logFile = (stateHome / (LayerPrettyName + ".log")).string();
        logStream.open(logFile, std::ios_base::ate);
    }

    DebugLog("--> xrNegotiateLoaderApiLayerInterface\n");

    if (apiLayerName && std::string_view(apiLayerName) != LAYER_NAME) {
        ErrorLog(fmt::format("Invalid apiLayerName \"{}\"\n", apiLayerName));
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    if (!loaderInfo || !apiLayerRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION ||
        loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        apiLayerRequest->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST ||
        apiLayerRequest->structVersion != XR_API_LAYER_INFO_STRUCT_VERSION ||
        apiLayerRequest->structSize != sizeof(XrNegotiateApiLayerRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxApiVersion < XR_CURRENT_API_VERSION || loaderInfo->minApiVersion > XR_CURRENT_API_VERSION) {
        ErrorLog("xrNegotiateLoaderApiLayerInterface validation failed\n");
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    // Setup our layer to intercept OpenXR calls.
    apiLayerRequest->layerInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
    apiLayerRequest->layerApiVersion = XR_CURRENT_API_VERSION;
    apiLayerRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(openxr_api_layer::xrGetInstanceProcAddr);
    apiLayerRequest->createApiLayerInstance = reinterpret_cast<PFN_xrCreateApiLayerInstance>(openxr_api_layer::xrCreateApiLayerInstance);

    DebugLog("<-- xrNegotiateLoaderApiLayerInterface\n");

    Log(fmt::format("{} layer ({}) is active\n", LayerName, VersionString));

    TraceLoggingWrite(g_traceProvider, "xrNegotiateLoaderApiLayerInterface_Complete");

    return XR_SUCCESS;
}
}
