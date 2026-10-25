#version 330 core

// MIT License
//
// Copyright(c) 2023 Matthieu Bucchianeri
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

// A Pixel Shader that paints the content of a layer given the specified coordinates.

uniform float smoothingArea;
uniform bool ignoreAlpha;
uniform bool isUnpremultipliedAlpha;
uniform bool debugFocusView;
uniform vec2 eyeGaze;
uniform vec4 stereoSubimageRect;
uniform int stereoSubimageArrayIndex;
uniform vec4 focusSubimageRect;
uniform int focusSubimageArrayIndex;

uniform sampler2D sourceStereoTexture;
uniform sampler2DArray sourceStereoTextureArray;
uniform sampler2D sourceFocusTexture;
uniform sampler2DArray sourceFocusTextureArray;

in vec2 texcoord;
in vec3 projectedFocusCoord;

layout(location = 0) out vec4 fragColor;

vec4 premultiplyAlpha(vec4 color) {
    return vec4(color.rgb * color.a, color.a);
}

vec4 unpremultiplyAlpha(vec4 color) {
    if (color.a != 0) {
        return vec4(color.rgb / color.a, color.a);
    } else {
        return vec4(0);
    }
}

// subimage xy=offset, zw=size
vec4 textureSubimage(sampler2D sampler, sampler2DArray samplerArray,
                     vec4 subimageRect, int subimageArrayIndex,
                     vec2 subimageCoord) {
    // Clamp coordinates to 0.5 texels from edges of subimage, so as not to
    // bleed neighbouring subimages
    ivec2 texSize = subimageArrayIndex >= 0 ? ivec2(textureSize(samplerArray, 0))
                                            : textureSize(sampler, 0);
    vec2 halfTexel = vec2(0.5) / texSize;
    vec2 clampedTexcoord = clamp(subimageCoord, halfTexel, vec2(1) - halfTexel);
    // Transform from subimage coordinates to texture coordinates
    vec2 coord = (subimageRect.xy + clampedTexcoord*subimageRect.zw) / texSize;
    return subimageArrayIndex >= 0 ? texture(samplerArray, vec3(coord, subimageArrayIndex))
                                   : texture(sampler, coord);
}

void main() {
    // Convert to texcoord and pick the pixel from each layer.
    vec4 color0 = textureSubimage(sourceStereoTexture, sourceStereoTextureArray,
                                  stereoSubimageRect, stereoSubimageArrayIndex,
                                  texcoord);
    vec2 layer1ProjectedCoordNdc = projectedFocusCoord.xy / projectedFocusCoord.z;
    vec2 layer1TexCoord = layer1ProjectedCoordNdc * 0.5f + 0.5f;
    vec4 color1 = textureSubimage(sourceFocusTexture, sourceFocusTextureArray,
                                  focusSubimageRect, focusSubimageArrayIndex,
                                  layer1TexCoord);

    if (ignoreAlpha) {
        color0.a = color1.a = 1;
    }

    if (!isUnpremultipliedAlpha) {
        color0 = unpremultiplyAlpha(color0);
        color1 = unpremultiplyAlpha(color1);
    }

    // Do a smooth transition with alpha-blending around the edges.
    float isInside = float(all(lessThan(abs(layer1ProjectedCoordNdc), vec2(1))));
    if (smoothingArea > 0.0) {
        vec2 s = smoothstep(vec2(0, 0), vec2(smoothingArea, smoothingArea), layer1TexCoord) -
                 smoothstep(vec2(1, 1) - vec2(smoothingArea, smoothingArea), vec2(1, 1), layer1TexCoord);
        float f = s.x * s.y;
        // Glow around focus when debugging
        if (debugFocusView && f < 0.5) {
            color1.r = 1;
            color1.b = 1;
        }
        color1.a = isInside * f;
    } else {
        color1.a = isInside;
    }

    color0 = premultiplyAlpha(color0);
    color1 = premultiplyAlpha(color1);

    // Blend the two pixels.
    vec4 color = color1 + color0 * (1 - color1.a);

    fragColor = vec4(color.rgb, color0.a);

    // Draw dot at focus when debugging
    if (debugFocusView) {
        vec2 delta = gl_FragCoord.xy - eyeGaze;
        if (dot(delta, delta) < 100) {
            fragColor = vec4(0.5f, 0.0f, 0.5f, 1.0f);
        }
    }
}
