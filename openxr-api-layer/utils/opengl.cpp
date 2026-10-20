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

#ifdef XR_USE_GRAPHICS_API_OPENGL

#include "log.h"

namespace {

    using namespace openxr_api_layer::log;
    using namespace openxr_api_layer::utils::graphics;

    constexpr bool PreferNtHandle = false;

    struct OpenGLContextStore;

    // Base OpenGL context, with virtual functions for managing context
    struct OpenGLContext {
        virtual void makeCurrent(OpenGLContextStore* store) const = 0;
        virtual void restoreContext(const OpenGLContextStore* store) const = 0;
    };

    // Union of each platform's context, which can be safely stored on the stack
    struct OpenGLContextStore {
        union {
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
        };
    };

    // Scoped context, restored on destruction
    struct OpenGLContextScoped {
        OpenGLContextScoped(OpenGLContext* context) : m_context(context) {
            m_context->makeCurrent(&m_ctx);
        }

        ~OpenGLContextScoped() {
            m_context->restoreContext(&m_ctx);
        }

        OpenGLContextStore m_ctx;
        OpenGLContext* m_context;
    };

    // Platform traits provide a way to access the appropriate member of the
    // union in OpenGLContextStore, based on the OpenXR binding struct.
    template <typename GraphicsBinding>
    struct OpenGLPlatformTraits;

    // Platform specific OpenGL context state
    template <typename GraphicsBinding>
    struct OpenGLPlatformContext : OpenGLContext {
        using Traits = OpenGLPlatformTraits<GraphicsBinding>;

        OpenGLPlatformContext(const GraphicsBinding* binding) {
            Traits::getPlatformContext(&m_context)->fromBinding(binding);
        }

        virtual void makeCurrent(OpenGLContextStore* store) const {
            Traits::getPlatformContext(store)->fromCurrent();
            Traits::getPlatformContext(&m_context)->makeCurrent();
        }
        virtual void restoreContext(const OpenGLContextStore* store) const {
            Traits::getPlatformContext(store)->makeCurrent();
        }

        OpenGLContextStore m_context;
    };

#ifdef XR_USE_PLATFORM_WIN32
    template <>
    struct OpenGLPlatformTraits<XrGraphicsBindingOpenGLWin32KHR> {
        static auto* getPlatformContext(OpenGLContextStore* store) {
            return &store->win32;
        }
        static auto* getPlatformContext(const OpenGLContextStore* store) {
            return &store->win32;
        }
    };
    using OpenGLContextWin32 = OpenGLPlatformContext<XrGraphicsBindingOpenGLWin32KHR>;
#endif

#ifdef XR_USE_PLATFORM_XLIB
    template <>
    struct OpenGLPlatformTraits<XrGraphicsBindingOpenGLXlibKHR> {
        static auto* getPlatformContext(OpenGLContextStore* store) {
            return &store->xlib;
        }
        static auto* getPlatformContext(const OpenGLContextStore* store) {
            return &store->xlib;
        }
    };
    using OpenGLContextXlib = OpenGLPlatformContext<XrGraphicsBindingOpenGLXlibKHR>;
#endif

#ifdef XR_USE_PLATFORM_XCB
    template <>
    struct OpenGLPlatformTraits<XrGraphicsBindingOpenGLXcbKHR> {
        static auto* getPlatformContext(OpenGLContextStore* store) {
            return &store->xcb;
        }
        static auto* getPlatformContext(const OpenGLContextStore* store) {
            return &store->xcb;
        }
    };
    using OpenGLContextXcb = OpenGLPlatformContext<XrGraphicsBindingOpenGLXcbKHR>;
#endif

#ifdef XR_USE_PLATFORM_EGL
    template <>
    struct OpenGLPlatformTraits<XrGraphicsBindingEGLMNDX> {
        static auto* getPlatformContext(OpenGLContextStore* store) {
            return &store->egl;
        }
        static auto* getPlatformContext(const OpenGLContextStore* store) {
            return &store->egl;
        }
    };
    using OpenGLContextEGL= OpenGLPlatformContext<XrGraphicsBindingEGLMNDX>;
#endif

    struct OpenGLGraphicsDevice : IGraphicsDevice {
        // Takes ownership of context
        OpenGLGraphicsDevice(OpenGLContext* context) : m_context(context) {
            TraceLocalActivity(local);
            TraceLoggingWriteStart(local, "OpenGLGraphicsDevice_Create", TLPArg(device, "OpenGLDevice"));

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
                    "OpenGLGraphicsDevice_Create",
                    TLArg(desc.Description, "Adapter"),
                    TLArg(fmt::format("{}:{}", m_adapterLuid.HighPart, m_adapterLuid.LowPart).c_str(), " Luid"));
#endif
            }

            TraceLoggingWriteStop(local, "OpenGLGraphicsDevice_Create", TLPArg(this, "Device"));
        }

        ~OpenGLGraphicsDevice() override {
            TraceLocalActivity(local);
            TraceLoggingWriteStart(local, "OpenGLGraphicsDevice_Destroy", TLPArg(this, "Device"));
            TraceLoggingWriteStop(local, "OpenGLGraphicsDevice_Destroy");
        }

        Api getApi() const override {
            return Api::OpenGL;
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
            return format == GL_SRGB8 || format == GL_SRGB8_ALPHA8;
        }

        bool isDepthFormat(GenericFormat format) const override {
            return format == GL_DEPTH_COMPONENT16 || format == GL_DEPTH_COMPONENT24 ||
                   format == GL_DEPTH_COMPONENT32 || format == GL_DEPTH_COMPONENT32F ||
                   format == GL_DEPTH24_STENCIL8 || format == GL_DEPTH32F_STENCIL8;
        }

        const std::unique_ptr<OpenGLContext> m_context;
    };

    template <typename Binding>
    std::shared_ptr<IGraphicsDevice> wrapApplicationDeviceOpenGL(const Binding& binding) {
        OpenGLContext* context = new OpenGLPlatformContext<Binding>(&binding);
        return std::make_shared<OpenGLGraphicsDevice>(context);
    }
} // namespace

namespace openxr_api_layer::utils::graphics::internal {

#ifdef XR_USE_PLATFORM_WIN32
    std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingOpenGLWin32KHR& bindings) {
        return wrapApplicationDeviceOpenGL(bindings);
    }
#endif
#ifdef XR_USE_PLATFORM_XLIB
    std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingOpenGLXlibKHR& bindings) {
        return wrapApplicationDeviceOpenGL(bindings);
    }
#endif
#ifdef XR_USE_PLATFORM_XCB
    std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingOpenGLXcbKHR& bindings) {
        return wrapApplicationDeviceOpenGL(bindings);
    }
#endif
#ifdef XR_USE_PLATFORM_EGL
    std::shared_ptr<IGraphicsDevice> wrapApplicationDevice(const XrGraphicsBindingEGLMNDX& bindings) {
        return wrapApplicationDeviceOpenGL(bindings);
    }
#endif

} // namespace openxr_api_layer::utils::graphics::internal

#endif
