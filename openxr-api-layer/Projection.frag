R"(
#version 430 core

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

uniform sampler2D sourceStereoTexture;
uniform sampler2D sourceFocusTexture;

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

void main() {
    // Convert to texcoord and pick the pixel from each layer.
    vec4 color0 = texture(sourceStereoTexture, texcoord);

    vec2 layer1ProjectedCoordNdc = projectedFocusCoord.xy / projectedFocusCoord.z;
    vec2 layer1TexCoord = layer1ProjectedCoordNdc * 0.5f + 0.5f;
    // For pixels outside of the focus view, the sampler will give us a fully transparent pixel.
    vec4 color1 = texture(sourceFocusTexture, layer1TexCoord);

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
        color1.a = isInside * max(0.5, s.x * s.y);
    } else {
        color1.a = isInside;
    }

    color0 = premultiplyAlpha(color0);
    color1 = premultiplyAlpha(color1);

    // Blend the two pixels.
    vec4 color;
    if (!debugFocusView) {
        color = color1 + color0 * (1 - color1.a);
    } else {
        color = color1;
    }

    fragColor = vec4(color.rgb, color0.a);

    if (debugFocusView) {
        vec2 delta = gl_FragCoord.xy - eyeGaze;
        if (dot(delta, delta) < 100) {
            fragColor = vec4(0.5f, 0.0f, 0.5f, 1.0f);
        }
    }
}
)"
