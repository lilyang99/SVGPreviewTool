# SVGPreviewTool

A Windows desktop application for previewing SVG (Scalable Vector Graphics) files. Built with MFC and Direct2D, it supports loading SVG images from files, streams, and embedded resources, and can convert them to HBITMAP, GDI+ Bitmap, and HICON formats.

## Features

- Load and render SVG files via Direct2D SVG API (`d2d1svg`)
- DPI-aware rendering with adjustable DPI settings
- Convert SVG to multiple output formats:
  - `HBITMAP` (Windows bitmap handle)
  - `Gdiplus::Bitmap` (GDI+ bitmap)
  - `HICON` (Windows icon handle)
- MFC-based MDI (Multiple Document Interface) application
- SVG attribute reading and inspection

## Project Structure

| Module | Description |
|--------|-------------|
| `SvgAPI/` | Abstract interfaces (`ISVGImage`, `ISVGRenderer`, `ISVGAttributeReader`) |
| `SvgRender/` | SVG rendering engine using Direct2D |
| `ImagePreview/` | MFC application UI |

## Requirements

- Windows 10 or later
- Visual Studio 2022 (v17)
- Windows SDK with Direct2D SVG support

## Building

1. Open `SvgPreivewTool.sln` in Visual Studio 2022
2. Select the desired configuration (Debug/Release, Win32)
3. Build the solution (Ctrl+Shift+B)

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
