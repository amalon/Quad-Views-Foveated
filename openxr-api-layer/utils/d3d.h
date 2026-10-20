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

#include "graphics.h"

namespace openxr_api_layer::utils::graphics {

#ifdef _WIN32
    struct D3DGraphicsDevice : IGraphicsDevice {
        virtual void* getNativeDevicePtr() const = 0;
        virtual void* getNativeContextPtr() const = 0;

        template <typename ApiTraits>
        typename ApiTraits::Device getNativeDevice() const {
            if (ApiTraits::api != getApi()) {
                throw std::runtime_error("Api mismatch");
            }
            return reinterpret_cast<typename ApiTraits::Device>(getNativeDevicePtr());
        }

        template <typename ApiTraits>
        typename ApiTraits::Context getNativeContext() const {
            if (ApiTraits::api != getApi()) {
                throw std::runtime_error("Api mismatch");
            }
            return reinterpret_cast<typename ApiTraits::Context>(getNativeContextPtr());
        }

        bool isSRGBFormat(GenericFormat format) const override {
            return format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB || format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
                   format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB || format == DXGI_FORMAT_BC1_UNORM_SRGB ||
                   format == DXGI_FORMAT_BC2_UNORM_SRGB || format == DXGI_FORMAT_BC3_UNORM_SRGB ||
                   format == DXGI_FORMAT_BC7_UNORM_SRGB;
        }

        bool isDepthFormat(GenericFormat format) const override {
            return format == DXGI_FORMAT_D16_UNORM || format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
                   format == DXGI_FORMAT_D32_FLOAT || format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        }

        virtual LUID getAdapterLuid() const = 0;
    };
#endif

} // namespace openxr_api_layer::utils::graphics
