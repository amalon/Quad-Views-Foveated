// MIT License
//
// Copyright(c) 2022-2023 Matthieu Bucchianeri
// Copyright(c) 2026 James Hogan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
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

#ifdef XR_USE_GRAPHICS_API_VULKAN

#include "log.h"

namespace {

    using namespace openxr_api_layer::log;
    using namespace openxr_api_layer::utils::graphics;

    struct VulkanGraphicsDevice : IGraphicsDevice {
        VulkanGraphicsDevice() {
            TraceLocalActivity(local);
            TraceLoggingWriteStart(local, "VulkanGraphicsDevice_Create", TLPArg(device, "VulkanDevice"));

            {
#if 0
                ComPtr<IDXGIDevice> dxgiDevice;
                CHECK_HRCMD(m_device->QueryInterface(IID_PPV_ARGS(dxgiDevice.ReleaseAndGetAddressOf())));
                ComPtr<IDXGIAdapter> dxgiAdapter;
                CHECK_HRCMD(dxgiDevice->GetAdapter(dxgiAdapter.ReleaseAndGetAddressOf()));
                DXGI_ADAPTER_DESC desc;
                CHECK_HRCMD(dxgiAdapter->GetDesc(&desc));
                m_adapterLuid = desc.AdapterLuid;

                TraceLoggingWriteTagged(
                    local,
                    "VulkanGraphicsDevice_Create",
                    TLArg(desc.Description, "Adapter"),
                    TLArg(fmt::format("{}:{}", m_adapterLuid.HighPart, m_adapterLuid.LowPart).c_str(), " Luid"));
#endif
            }

            TraceLoggingWriteStop(local, "VulkanGraphicsDevice_Create", TLPArg(this, "Device"));
        }

        ~VulkanGraphicsDevice() override {
            TraceLocalActivity(local);
            TraceLoggingWriteStart(local, "VulkanGraphicsDevice_Destroy", TLPArg(this, "Device"));
            TraceLoggingWriteStop(local, "VulkanGraphicsDevice_Destroy");
        }

        Api getApi() const override {
            return Api::Vulkan;
        }

        IContext* getContext() override {
            return m_context.get();
        }

        std::shared_ptr<IGraphicsTimer> createTimer() override {
#if 0
            return std::make_shared<D3D11Timer>(m_device.Get());
#endif
            return nullptr;
        }

        std::shared_ptr<IGraphicsFence> createFence(bool shareable) override {
#if 0
            ComPtr<ID3D11Fence> fence;
            CHECK_HRCMD(
                m_deviceForFencesAndNtHandles->CreateFence(0,
                                                           shareable ? D3D11_FENCE_FLAG_SHARED : D3D11_FENCE_FLAG_NONE,
                                                           IID_PPV_ARGS(fence.ReleaseAndGetAddressOf())));
            return std::make_shared<D3D11Fence>(fence.Get(), shareable);
#endif
            return nullptr;
        }

        std::shared_ptr<IGraphicsFence> openFence(const ShareableHandle& handle) override {
#if 0
            TraceLocalActivity(local);
            TraceLoggingWriteStart(local,
                                   "D3D11Fence_Import",
                                   TLArg(!handle.isNtHandle ? handle.handle : handle.ntHandle.get(), "Handle"),
                                   TLArg(handle.isNtHandle, "IsNTHandle"));

            if (!handle.isNtHandle) {
                throw std::runtime_error("Must be NTHANDLE");
            }

            ComPtr<ID3D11Fence> fence;
            CHECK_HRCMD(m_deviceForFencesAndNtHandles->OpenSharedFence(handle.isNtHandle ? handle.ntHandle.get()
                                                                                         : handle.handle,
                                                                       IID_PPV_ARGS(fence.ReleaseAndGetAddressOf())));
            std::shared_ptr<IGraphicsFence> result = std::make_shared<D3D11Fence>(fence.Get(), false /* shareable */);

            TraceLoggingWriteStop(local, "D3D11Fence_Import", TLPArg(result.get(), "Fence"));

            return result;
#endif
            return nullptr;
        }

        std::shared_ptr<IGraphicsTexture> createTexture(const XrSwapchainCreateInfo& info, bool shareable) override {
#if 0
            D3D11_TEXTURE2D_DESC desc{};
            desc.Format = (DXGI_FORMAT)info.format;
            desc.Width = info.width;
            desc.Height = info.height;
            desc.ArraySize = info.arraySize;
            desc.MipLevels = info.mipCount;
            desc.SampleDesc.Count = info.sampleCount;
            desc.Usage = D3D11_USAGE_DEFAULT;
            if (info.usageFlags & XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT) {
                desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            }
            if (info.usageFlags & XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            }
            if (info.usageFlags & XR_SWAPCHAIN_USAGE_SAMPLED_BIT) {
                desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }
            if (info.usageFlags & XR_SWAPCHAIN_USAGE_UNORDERED_ACCESS_BIT) {
                desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }
            // Mark as shareable if needed.
            desc.MiscFlags =
                shareable ? (D3D11_RESOURCE_MISC_SHARED | (PreferNtHandle ? D3D11_RESOURCE_MISC_SHARED_NTHANDLE : 0))
                          : 0;

            ComPtr<ID3D11Texture2D> texture;
            CHECK_HRCMD(m_device->CreateTexture2D(&desc, nullptr, texture.ReleaseAndGetAddressOf()));
            return std::make_shared<D3D11Texture>(texture.Get());
#endif
            return nullptr;
        }

        std::shared_ptr<IGraphicsTexture> openTexture(const ShareableHandle& handle,
                                                      const XrSwapchainCreateInfo& info) override {
#if 0
            TraceLocalActivity(local);
            TraceLoggingWriteStart(local,
                                   "D3D11Texture_Import",
                                   TLArg(!handle.isNtHandle ? handle.handle : handle.ntHandle.get(), "Handle"),
                                   TLArg(handle.isNtHandle, "IsNTHandle"));

            ComPtr<ID3D11Texture2D> texture;
            if (!handle.isNtHandle) {
                CHECK_HRCMD(
                    m_device->OpenSharedResource(handle.handle, IID_PPV_ARGS(texture.ReleaseAndGetAddressOf())));
            } else {
                CHECK_HRCMD(m_deviceForFencesAndNtHandles->OpenSharedResource1(
                    handle.ntHandle.get(), IID_PPV_ARGS(texture.ReleaseAndGetAddressOf())));
            }

            std::shared_ptr<IGraphicsTexture> result = std::make_shared<D3D11Texture>(texture.Get());

            TraceLoggingWriteStop(local, "D3D11Texture_Import", TLPArg(result.get(), "Texture"));

            return result;
#endif
            return nullptr;
        }

        std::shared_ptr<IGraphicsTexture> openTexturePtr(void* nativeTexturePtr,
                                                         const XrSwapchainCreateInfo& info) override {
#if 0
            TraceLocalActivity(local);
            TraceLoggingWriteStart(local, "D3D11Texture_Import", TLPArg(nativeTexturePtr, "D3D11Texture"));

            ID3D11Texture2D* texture = reinterpret_cast<ID3D11Texture2D*>(nativeTexturePtr);

            std::shared_ptr<IGraphicsTexture> result = std::make_shared<D3D11Texture>(texture);

            TraceLoggingWriteStop(local, "D3D11Texture_Import", TLPArg(result.get(), "Texture"));

            return result;
#endif
            return nullptr;
        }

        void copyTexture(IGraphicsTexture* from, IGraphicsTexture* to) override {
#if 0
            TraceLocalActivity(local);
            TraceLoggingWriteStart(local, "D3D11Texture_Copy", TLPArg(from, "Source"), TLPArg(to, "Destination"));

            m_context->CopyResource(to->getNativeTexture<D3D11>(), from->getNativeTexture<D3D11>());

            TraceLoggingWriteStop(local, "D3D11Texture_Copy");
#endif
        }

        GenericFormat translateToGenericFormat(int64_t format) const override {
            return (GenericFormat)format;
        }

        int64_t translateFromGenericFormat(GenericFormat format) const override {
            return (int64_t)format;
        }

        bool isSRGBFormat(GenericFormat format) const override {
            switch (format) {
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_B8G8R8_SRGB:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
                return true;
            default:
                return false;
            }
        }

        bool isDepthFormat(GenericFormat format) const override {
            switch (format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return true;
            default:
                return false;
            }
        }

        const std::unique_ptr<IContext> m_context;
    };

    template <typename Binding>
    std::shared_ptr<IGraphicsDevice> wrapApplicationDeviceVulkan(const Binding& binding) {
        return std::make_shared<VulkanGraphicsDevice>();
    }
} // namespace

namespace openxr_api_layer::utils::graphics::internal {

    std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingVulkanKHR& bindings) {
        return wrapApplicationDeviceVulkan(bindings);
    }

} // namespace openxr_api_layer::utils::graphics::internal

#endif
