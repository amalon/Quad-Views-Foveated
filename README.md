# Linux/OpenGL port by James Hogan

This is a port of mbucchia's Quad Views OpenXR layer to Linux + OpenGL (from
Windows + Direct3D).

To configure, build and install to the current user's home directory:
```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local .
$ cmake --build build -j$(nproc)
$ cmake --install build
```

## Features

Working features:
- Basic compositing of quad views using OpenGL
- `debug_focus_view`
- Logging to `$XDG_RUNTIME_DIR/Quad-Views-Foveated/Quad-Views-Foveated.log`

New features of this port:
- Stereo with Foveated Inset (OpenXR 1.1)
- Handle mutable FOV

Features not yet working:
- Handling of subimage rect and array index
- Reading settings.cfg from a sensible location (for now, create a symlink at
  `$XDG_RUNTIME_DIR/Quad-Views-Foveated/settings.cfg`)

Features missing and unlikely:
- Sharpening of focus view

## Platforms and Graphics APIs

The following platforms should work:
- Linux + OpenGL via Xlib (GLX)
- Linux + OpenGL via EGL

The following platforms have stubs and still need the compositor ported:
- Linux + Vulkan

The following platforms are untested, and are most certainly broken by this
port (though some effort has been made to allow the code to support multiple
graphics APIs simultaneously):
- Windows + D3D11
- Windows + D3D12
- Windows + OpenGL via wgl

## Testing status

Tested with:
- WiVRn (Steam Frame streamed)
- SteamVR (Steam Frame standalone)

# Foveated Rendering via Quad Views

In layperson's terms:

This software lets you use Eye-Tracked Foveated Rendering (sometimes referred to as Dynamic Foveated Rendering) with your Pimax Crystal, Meta Quest Pro, and other headsets supporting eye tracking via OpenXR in games using the [quad views rendering](https://github.com/mbucchia/Quad-Views-Foveated/wiki/What-is-Quad-Views-rendering%3F) technique like Digital Combat Simulation (DCS) and Pavlov VR.

In technical terms:

This software enables OpenXR apps developed with [`XR_VARJO_quad_views`](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_VARJO_quad_views) and optionally [`XR_VARJO_foveated_rendering`](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_VARJO_foveated_rendering) to be used on platforms that do not typically support those extensions. It composes each quad view projection layer into a stereo projection layer, and uses the eye tracking support on the device to make the inner projection views follow the eye gaze.

DISCLAIMER: This software is distributed as-is, without any warranties or conditions of any kind. Use at your own risks.

# Details and instructions on the [the wiki](https://github.com/mbucchia/Quad-Views-Foveated/wiki)!

## Setup

Download the latest version from the [Releases page](https://github.com/mbucchia/Quad-Views-Foveated/releases). Find the installer program under **Assets**, file `Quad-Views-Foveated-<version>.msi`.

More information on the [the wiki](https://github.com/mbucchia/Quad-Views-Foveated/wiki)!

For troubleshooting, the log file can be found at `%LocalAppData%\Quad-Views-Foveated\Quad-Views-Foveated.log`.

## Donate

Donations are welcome and totally optional. Please use [my GitHub sponsorship page](https://github.com/sponsors/mbucchia) to make one-time or recurring donations!

Thank you!

## Special thanks

Thanks to my beta testers for helping throughout development and release (in alphabetical order):

- BARRACUDAS
- edmuss
- MastahFR
- mfrisby
- Omniwhatever
- xMcCARYx
