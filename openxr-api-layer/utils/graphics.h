// MIT License
//
// Copyright(c) 2022-2023 Matthieu Bucchianeri
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

#pragma once

#include "general.h"

namespace openxr_api_layer::utils::graphics {

    enum class Api {
#ifdef XR_USE_GRAPHICS_API_OPENGL
        OpenGL,
#endif
#ifdef XR_USE_GRAPHICS_API_VULKAN
        Vulkan,
#endif
#ifdef XR_USE_GRAPHICS_API_D3D11
        D3D11,
#endif
#ifdef XR_USE_GRAPHICS_API_D3D12
        D3D12,
#endif
    };
    enum class CompositionApi {
#ifdef XR_USE_GRAPHICS_API_OPENGL
        OpenGL,
#endif
#ifdef XR_USE_GRAPHICS_API_VULKAN
        Vulkan,
#endif
#ifdef XR_USE_GRAPHICS_API_D3D11
        D3D11,
#endif
    };

    // Type traits for all graphics APIs.

#ifdef XR_USE_GRAPHICS_API_OPENGL
    struct OpenGL {
        static constexpr Api api = Api::OpenGL;

        //using Device = ID3D11Device*;
        //using Context = ID3D11DeviceContext*;
        using Texture = uint32_t;
        using PtrTexture = Texture;
        //using Fence = ID3D11Fence*;
    };
#endif

#ifdef XR_USE_GRAPHICS_API_VULKAN
    struct Vulkan {
        static constexpr Api api = Api::Vulkan;

        //using Device = ID3D11Device*;
        //using Context = ID3D11DeviceContext*;
        using Texture = VkImage;
        using PtrTexture = Texture;
        //using Fence = ID3D11Fence*;
    };
#endif

#ifdef XR_USE_GRAPHICS_API_D3D11
    struct D3D11 {
        static constexpr Api api = Api::D3D11;

        using Device = ID3D11Device*;
        using Context = ID3D11DeviceContext*;
        using Texture = ID3D11Texture2D*;
        using PtrTexture = ComPtr<*Texture>;
        using Fence = ID3D11Fence*;
    };
#endif

#ifdef XR_USE_GRAPHICS_API_D3D12
    struct D3D12 {
        static constexpr Api api = Api::D3D12;

        using Device = ID3D12Device*;
        using Context = ID3D12CommandQueue*;
        using Texture = ID3D12Resource*;
        using Fence = ID3D12Fence*;
    };
#endif

    // Union of each platform's context, which can be safely stored on the stack
    struct ContextVariant {
        union {
#ifdef XR_USE_GRAPHICS_API_OPENGL
#ifdef XR_USE_PLATFORM_WIN32
            struct {
                void clear() {
                    m_hDC = NULL;
                    m_hGLRC = NULL;
                }

                void fromBinding(const XrGraphicsBindingOpenGLWin32KHR* binding) {
                    m_hDC = binding->hDC;
                    m_hGLRC = binding->hGLRC;
                }

                void fromCurrent() {
                    m_hDC = wglGetCurrentDC();
                    m_hGLRC = wglGetCurrentContext();
                }

                void makeCurrent() const {
                    return wglMakeCurrent(m_hDC, m_hGLRC);
                }

                HDC m_hDC;
                hGLRC m_hGLRC;
            } win32;
#endif
#ifdef XR_USE_PLATFORM_XLIB
            struct {
                void clear() {
                    m_display = nullptr;
                    m_drawable = None;
                    m_ctx = nullptr;
                }

                void fromBinding(const XrGraphicsBindingOpenGLXlibKHR* binding) {
                    m_display = binding->xDisplay;
                    m_drawable = binding->glxDrawable;
                    m_ctx = binding->glxContext;
                }

                void fromCurrent() {
                    m_display = glXGetCurrentDisplay();
                    m_drawable = glXGetCurrentDrawable();
                    m_ctx = glXGetCurrentContext();
                }

                bool makeCurrent() const {
                    return glXMakeCurrent(m_display, m_drawable, m_ctx);
                }

                Display* m_display;
                GLXDrawable m_drawable;
                GLXContext m_ctx;
            } xlib;
#endif
#ifdef XR_USE_PLATFORM_XCB
#error incomplete
            struct {
                void clear() {
                }

                void fromBinding(const XrGraphicsBindingOpenGLXcbKHR* binding) {
                    m_conn = binding->connection;
                    m_drawable = binding->glxDrawable;
                    m_context = binding->glxContext;
                    // m_oldContextTag
                }

                void fromCurrent() {

                }

                bool makeCurrent() const {
                    xcb_glx_make_current(m_conn, m_drawable, m_context,
                                         m_oldContextTag);
                }

                xcb_connection_t* m_conn;
                xcb_glx_drawable_t m_drawable;
                xcb_glx_context_t m_context;
                //xcb_glx_context_tag_t m_oldContextTag;
            } xcb;
#endif
#ifdef XR_USE_PLATFORM_EGL
            struct {
                void clear() {
                    // FIXME m_display?
                    m_draw = EGL_NO_SURFACE;
                    m_read = EGL_NO_SURFACE;
                    m_context = EGL_NO_CONTEXT;
                }

                void fromBinding(const XrGraphicsBindingEGLMNDX* binding) {
                    m_display = binding->display;
                    m_draw = EGL_NO_SURFACE;
                    m_read = EGL_NO_SURFACE;
                    m_context = binding->context;
                }

                void fromCurrent() {
                    m_display = eglGetCurrentDisplay();
                    m_draw = eglGetCurrentSurface(EGL_DRAW);
                    m_read = eglGetCurrentSurface(EGL_READ);
                    m_context = eglGetCurrentContext();
                }

                bool makeCurrent() const {
                    return eglMakeCurrent(m_display, m_draw, m_read, m_context);
                }

                EGLDisplay m_display;
                EGLSurface m_draw;
                EGLSurface m_read;
                EGLContext m_context;
            } egl;
#endif
#endif
        };
    };

    // Base context, with virtual functions for managing context
    struct IContext {
        virtual void makeCurrent(ContextVariant* store) const = 0;
        virtual void restoreContext(const ContextVariant* store) const = 0;
    };

    // We use int64_t as a common conversion point for all graphics APIs to
    // match OpenXR.
    using GenericFormat = int64_t;

    struct ShareableHandle {
#ifdef _WIN32
        wil::unique_handle ntHandle;
        HANDLE handle{nullptr};
        bool isNtHandle{};
#endif
        Api origin{};
    };

    // A timer on the GPU.
    struct IGraphicsTimer : openxr_api_layer::utils::general::ITimer {
        virtual ~IGraphicsTimer() = default;

        virtual Api getApi() const = 0;
    };

    // A fence.
    struct IGraphicsFence {
        virtual ~IGraphicsFence() = default;

        virtual Api getApi() const = 0;
        virtual void* getNativeFencePtr() const = 0;
        virtual ShareableHandle getFenceHandle() const = 0;

        virtual void signal(uint64_t value) = 0;
        virtual void waitOnDevice(uint64_t value) = 0;
        virtual void waitOnCpu(uint64_t value) = 0;

        virtual bool isShareable() const = 0;

        template <typename ApiTraits>
        typename ApiTraits::Fence getNativeFence() const {
            if (ApiTraits::api != getApi()) {
                throw std::runtime_error("Api mismatch");
            }
            return reinterpret_cast<typename ApiTraits::Fence>(getNativeFencePtr());
        }
    };

    // A texture.
    struct IGraphicsTexture {
        virtual ~IGraphicsTexture() = default;

        virtual Api getApi() const = 0;
        virtual void* getNativeTexturePtr() const = 0;
        virtual ShareableHandle getTextureHandle() const = 0;

        virtual const XrSwapchainCreateInfo& getInfo() const = 0;
        virtual bool isShareable() const = 0;

        template <typename ApiTraits>
        typename ApiTraits::Texture getNativeTexture() const {
            if (ApiTraits::api != getApi()) {
                throw std::runtime_error("Api mismatch");
            }
            return reinterpret_cast<typename ApiTraits::Texture>(getNativeTexturePtr());
        }
    };

    // A graphics device and execution context.
    struct IGraphicsDevice {
        virtual ~IGraphicsDevice() = default;

        virtual Api getApi() const = 0;

        virtual IContext* getContext() = 0;

        virtual std::shared_ptr<IGraphicsTimer> createTimer() = 0;
        virtual std::shared_ptr<IGraphicsFence> createFence(bool shareable = true) = 0;
        virtual std::shared_ptr<IGraphicsFence> openFence(const ShareableHandle& handle) = 0;
        virtual std::shared_ptr<IGraphicsTexture> createTexture(const XrSwapchainCreateInfo& info,
                                                                bool shareable = true) = 0;
        virtual std::shared_ptr<IGraphicsTexture> openTexture(const ShareableHandle& handle,
                                                              const XrSwapchainCreateInfo& info) = 0;
        virtual std::shared_ptr<IGraphicsTexture> openTexturePtr(void* nativeTexturePtr,
                                                                 const XrSwapchainCreateInfo& info) = 0;

        virtual void copyTexture(IGraphicsTexture* from, IGraphicsTexture* to) = 0;

        virtual GenericFormat translateToGenericFormat(int64_t format) const = 0;
        virtual int64_t translateFromGenericFormat(GenericFormat format) const = 0;

        virtual bool isSRGBFormat(GenericFormat format) const = 0;
        virtual bool isDepthFormat(GenericFormat format) const = 0;

        template <typename ApiTraits>
        std::shared_ptr<IGraphicsTexture> openTexture(typename ApiTraits::Texture nativeTexture,
                                                      const XrSwapchainCreateInfo& info) {
            if (ApiTraits::api != getApi()) {
                throw std::runtime_error("Api mismatch");
            }
            return openTexturePtr(reinterpret_cast<void*>(nativeTexture), info);
        }
    };

    // Modes of use of wrapped swapchains.
    enum class SwapchainMode {
        // The swapchain must be submittable to the upstream xrEndFrame() implementation.
        // A non-submittable swapchain does not have an XrSwapchain handle.
        Submit = (1 << 0),

        // The swapchain will be accessed for reading during composition in the layer's xrEndFrame() implementation.
        // A readable swapchain might require a copy to the composition device before composition.
        Read = (1 << 1),

        // The swapchain will be access for writing during composition in the layer's xrEndFrame() implementation.
        // A writable swapchain might require a copy from the composition device after composition.
        Write = (1 << 2),
    };
    inline SwapchainMode operator &(SwapchainMode a, SwapchainMode b)
    {
        return (SwapchainMode)((unsigned int)a & (unsigned int)b);
    }

    struct ISwapchainImage;

    // A swapchain.
    struct ISwapchain {
        virtual ~ISwapchain() = default;

        // Only for manipulating swapchains created through createSwapchain().
        virtual ISwapchainImage* acquireImage(bool wait = true) = 0;
        virtual void waitImage() = 0;
        virtual void releaseImage() = 0;

        virtual ISwapchainImage* getLastReleasedImage() const = 0;
        virtual void commitLastReleasedImage() = 0;

        virtual const XrSwapchainCreateInfo& getInfoOnCompositionDevice() const = 0;
        virtual int64_t getFormatOnApplicationDevice() const = 0;
        virtual ISwapchainImage* getImage(uint32_t index) const = 0;
        virtual uint32_t getLength() const = 0;

        // Can only be called if the swapchain is submittable.
        virtual XrSwapchain getSwapchainHandle() const = 0;
        virtual XrSwapchainSubImage getSubImage() const = 0;
    };

    // A swapchain image.
    struct ISwapchainImage {
        virtual ~ISwapchainImage() = default;

        virtual IGraphicsTexture* getApplicationTexture() const = 0;
        virtual IGraphicsTexture* getTextureForRead() const = 0;
        virtual IGraphicsTexture* getTextureForWrite() const = 0;

        virtual uint32_t getIndex() const = 0;
    };

    // A container for user session data.
    // This class is meant to be extended by a caller before use with ICompositionFramework::setSessionData() and
    // ICompositionFramework::getSessionData().
    struct ICompositionSessionData {
        virtual ~ICompositionSessionData() = default;
    };

    // A collection of hooks and utilities to perform composition in the layer.
    struct ICompositionFramework {
        virtual ~ICompositionFramework() = default;

        virtual XrSession getSessionHandle() const = 0;

        virtual void setSessionData(std::unique_ptr<ICompositionSessionData> sessionData) = 0;
        virtual ICompositionSessionData* getSessionDataPtr() const = 0;

        // Create a swapchain without an XrSwapchain handle.
        virtual std::shared_ptr<ISwapchain> createSwapchain(const XrSwapchainCreateInfo& infoOnApplicationDevice,
                                                            SwapchainMode mode) = 0;

        // Must be called at the beginning of the layer's xrEndFrame() implementation to serialize application commands
        // prior to composition.
        virtual void serializePreComposition() = 0;

        // Must be called before chaining to the upstream xrEndFrame() implementation to serialize composition commands
        // prior to submission.
        virtual void serializePostComposition() = 0;

        virtual IGraphicsDevice* getCompositionDevice() const = 0;
        virtual IGraphicsDevice* getApplicationDevice() const = 0;
        virtual int64_t getPreferredSwapchainFormatOnApplicationDevice(XrSwapchainUsageFlags usageFlags,
                                                                       bool preferSRGB = true) const = 0;

        template <typename SessionData>
        SessionData* getSessionData() const {
            return reinterpret_cast<SessionData*>(getSessionDataPtr());
        }
    };

    // A factory to create composition frameworks for each session.
    struct ICompositionFrameworkFactory {
        virtual ~ICompositionFrameworkFactory() = default;

        // Must be called after chaining to the upstream xrGetInstanceProcAddr() implementation.
        virtual void xrGetInstanceProcAddr_post(XrInstance instance,
                                                const char* name,
                                                PFN_xrVoidFunction* function) = 0;

        virtual ICompositionFramework* getCompositionFramework(XrSession session) = 0;
    };

    std::shared_ptr<ICompositionFrameworkFactory>
    createCompositionFrameworkFactory(const XrInstanceCreateInfo& info,
                                      XrInstance instance,
                                      PFN_xrGetInstanceProcAddr xrGetInstanceProcAddr,
                                      CompositionApi compositionApi);

    namespace internal {

#ifdef XR_USE_GRAPHICS_API_OPENGL
#ifdef XR_USE_PLATFORM_WIN32
        std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingOpenGLWin32KHR& bindings);
#endif
#ifdef XR_USE_PLATFORM_XLIB
        std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingOpenGLXlibKHR& bindings);
#endif
#ifdef XR_USE_PLATFORM_XCB
        std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingOpenGLXcbKHR& bindings);
#endif
#ifdef XR_USE_PLATFORM_EGL
        std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingEGLMNDX& bindings);
#endif
#endif

#ifdef XR_USE_GRAPHICS_API_VULKAN
        std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingVulkanKHR& bindings);
#endif

#ifdef XR_USE_GRAPHICS_API_D3D11
        std::shared_ptr<IGraphicsDevice> createD3D11CompositionDevice(LUID adapterLuid);
        std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingD3D11KHR& bindings);
#endif

#ifdef XR_USE_GRAPHICS_API_D3D12
        std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingD3D12KHR& bindings);
#endif

    } // namespace internal

} // namespace openxr_api_layer::utils::graphics
