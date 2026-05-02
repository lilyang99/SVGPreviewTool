#include "pch.h"
#include "SVGImageConverter.h"
#include "../../SvgAPI/ISVGRenderer.h"
#include "../../SvgAPI/ISVGAttributeReader.h"
#include <vector>
#include <wrl.h>

CSVGImageConverter::CSVGImageConverter(
    ISVGRenderer* renderer, 
    ISVGAttributeReader* attributeReader)
    : m_renderer(renderer)
    , m_attributeReader(attributeReader)
{
}

HBITMAP CSVGImageConverter::ConvertToBitmap(UINT dpi)
{
    if (!m_renderer || !m_attributeReader)
        return nullptr;

    UINT width = 0, height = 0;
    IWICBitmap* wicBitmap = nullptr;

    HRESULT hr = RenderToWICBitmap(&wicBitmap, &width, &height, dpi);
    if (FAILED(hr))
        return nullptr;

    HBITMAP hBitmap = nullptr;
    hr = CopyWICBitmapToHBITMAP(wicBitmap, &hBitmap);

    wicBitmap->Release();
    if (FAILED(hr))
        return nullptr;
    return hBitmap;
}

Gdiplus::Bitmap* CSVGImageConverter::ConvertToGdiPlusBitmap(UINT dpi)
{
    if (!m_renderer || !m_attributeReader)
        return nullptr;

    UINT width = 0, height = 0;
    IWICBitmap* wicBitmap = nullptr;

    HRESULT hr = RenderToWICBitmap(&wicBitmap, &width, &height, dpi);
    if (FAILED(hr))
        return nullptr;

    hr = wicBitmap->GetSize(&width, &height);
    if (FAILED(hr))
    {
        wicBitmap->Release();
        return nullptr;
    }

    WICRect rect = {0, 0, static_cast<INT>(width), static_cast<INT>(height)};
    Microsoft::WRL::ComPtr<IWICBitmapLock> lock;
    hr = wicBitmap->Lock(&rect, WICBitmapLockRead, &lock);
    Gdiplus::Bitmap* pGdiBitmap = nullptr;

    if (SUCCEEDED(hr))
    {
        UINT wicStride = 0;
        hr = lock->GetStride(&wicStride);
        if (SUCCEEDED(hr))
        {
            UINT dataCount = 0;
            WICInProcPointer data = nullptr;
            hr = lock->GetDataPointer(&dataCount, &data);
            if (SUCCEEDED(hr) && data)
            {
                pGdiBitmap = new Gdiplus::Bitmap(
                    static_cast<INT>(width),
                    static_cast<INT>(height),
                    PixelFormat32bppPARGB);  // Match WIC PBGRA premultiplied alpha

                if (pGdiBitmap && pGdiBitmap->GetLastStatus() == Gdiplus::Ok)
                {
                    Gdiplus::BitmapData bitmapData;
                    Gdiplus::Rect rectGdi(0, 0, static_cast<INT>(width), static_cast<INT>(height));
                    Gdiplus::Status lockStatus = pGdiBitmap->LockBits(
                        &rectGdi,
                        Gdiplus::ImageLockModeWrite,
                        PixelFormat32bppPARGB,
                        &bitmapData);

                    if (lockStatus == Gdiplus::Ok)
                    {
                        const BYTE* srcPtr = static_cast<const BYTE*>(data);
                        BYTE* dstPtr = static_cast<BYTE*>(bitmapData.Scan0);
                        const UINT copyBytes = (std::min)(wicStride, static_cast<UINT>(bitmapData.Stride));
                        for (UINT y = 0; y < height; y++)
                        {
                            CopyMemory(dstPtr + y * bitmapData.Stride,
                                       srcPtr + y * wicStride,
                                       copyBytes);
                        }
                        pGdiBitmap->UnlockBits(&bitmapData);
                    }
                    else
                    {
                        delete pGdiBitmap;
                        pGdiBitmap = nullptr;
                    }
                }
                else if (pGdiBitmap)
                {
                    delete pGdiBitmap;
                    pGdiBitmap = nullptr;
                }
            }
        }
    }

    wicBitmap->Release();
    return pGdiBitmap;
}

HICON CSVGImageConverter::ConvertToIcon(UINT dpi)
{
    if (!m_renderer || !m_attributeReader)
        return nullptr;

    UINT width = 0, height = 0;
    IWICBitmap* wicBitmap = nullptr;

    HRESULT hr = RenderToWICBitmap(&wicBitmap, &width, &height, dpi);
    if (FAILED(hr))
        return nullptr;

    hr = wicBitmap->GetSize(&width, &height);
    if (FAILED(hr))
    {
        wicBitmap->Release();
        return nullptr;
    }

    WICRect rect = {0, 0, static_cast<INT>(width), static_cast<INT>(height)};
    Microsoft::WRL::ComPtr<IWICBitmapLock> lock;
    hr = wicBitmap->Lock(&rect, WICBitmapLockRead, &lock);

    HICON hIcon = nullptr;
    if (SUCCEEDED(hr))
    {
        UINT wicStride = 0;
        hr = lock->GetStride(&wicStride);
        if (SUCCEEDED(hr))
        {
            UINT dataCount = 0;
            WICInProcPointer data = nullptr;
            hr = lock->GetDataPointer(&dataCount, &data);

            if (SUCCEEDED(hr) && data)
            {
                Gdiplus::Bitmap* pGdiBitmap = new Gdiplus::Bitmap(
                    static_cast<INT>(width),
                    static_cast<INT>(height),
                    PixelFormat32bppPARGB);

                if (pGdiBitmap && pGdiBitmap->GetLastStatus() == Gdiplus::Ok)
                {
                    Gdiplus::BitmapData bitmapData;
                    Gdiplus::Rect rectGdi(0, 0, static_cast<INT>(width), static_cast<INT>(height));
                    Gdiplus::Status lockStatus = pGdiBitmap->LockBits(
                        &rectGdi,
                        Gdiplus::ImageLockModeWrite,
                        PixelFormat32bppPARGB,
                        &bitmapData);

                    if (lockStatus == Gdiplus::Ok)
                    {
                        const BYTE* srcPtr = static_cast<const BYTE*>(data);
                        BYTE* dstPtr = static_cast<BYTE*>(bitmapData.Scan0);
                        const UINT copyBytes = (std::min)(wicStride, static_cast<UINT>(bitmapData.Stride));
                        for (UINT y = 0; y < height; y++)
                        {
                            CopyMemory(dstPtr + y * bitmapData.Stride,
                                       srcPtr + y * wicStride,
                                       copyBytes);
                        }
                        pGdiBitmap->UnlockBits(&bitmapData);

                        if (pGdiBitmap->GetLastStatus() == Gdiplus::Ok)
                        {
                            pGdiBitmap->GetHICON(&hIcon);
                        }
                    }
                    delete pGdiBitmap;
                }
                else if (pGdiBitmap)
                {
                    delete pGdiBitmap;
                }
            }
        }
    }

    wicBitmap->Release();
    return hIcon;
}

HRESULT CSVGImageConverter::RenderToWICBitmap(
    IWICBitmap** outputBitmap, 
    UINT* width, 
    UINT* height, 
    UINT dpi)
{
    if (!outputBitmap || !width || !height)
        return E_POINTER;

    // Check if renderer and attribute reader are valid
    if (!m_renderer || !m_renderer->IsValid())
        return E_FAIL;

    if (!m_attributeReader || !m_attributeReader->IsValid())
        return E_FAIL;

    // Get original size
    SIZE originalSize = m_attributeReader->GetOriginalSize();
    
    // Calculate target size (scaled by DPI)
    *width = static_cast<UINT>(static_cast<FLOAT>(originalSize.cx) * dpi / USER_DEFAULT_SCREEN_DPI);
    *height = static_cast<UINT>(static_cast<FLOAT>(originalSize.cy) * dpi / USER_DEFAULT_SCREEN_DPI);

    if (*width == 0 || *height == 0)
        return E_FAIL;

    // Create WIC factory if not already created
    if (!m_wicFactory)
    {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_wicFactory));
        if (FAILED(hr))
            return hr;
    }

    HRESULT hr = m_wicFactory->CreateBitmap(
        *width,
        *height,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapCacheOnLoad,
        outputBitmap);
    if (FAILED(hr))
        return hr;

    // Render to target bitmap using renderer
    FLOAT dpiX = static_cast<FLOAT>(dpi);
    FLOAT dpiY = dpiX;
    
    hr = m_renderer->RenderToWICBitmap(*outputBitmap, dpiX, dpiY);
    if (FAILED(hr))
    {
        (*outputBitmap)->Release();
        *outputBitmap = nullptr;
        return hr;
    }

    return S_OK;
}

HRESULT CSVGImageConverter::CopyWICBitmapToHBITMAP(
    IWICBitmap* wicBitmap,
    HBITMAP* outputBitmap)
{
    if (!wicBitmap || !outputBitmap)
        return E_POINTER;

    UINT width = 0, height = 0;
    HRESULT hr = wicBitmap->GetSize(&width, &height);
    if (FAILED(hr))
        return hr;

    // Calculate DIB stride (4 bytes per pixel, aligned to 4-byte boundary)
    UINT dibStride = (width * 4 + 3) & ~3;

    // Get WIC bitmap data
    WICRect rect = {0, 0, static_cast<INT>(width), static_cast<INT>(height)};
    Microsoft::WRL::ComPtr<IWICBitmapLock> lock;
    hr = wicBitmap->Lock(&rect, WICBitmapLockRead, &lock);
    if (FAILED(hr))
        return hr;

    // Get WIC bitmap stride from lock
    UINT wicStride = 0;
    hr = lock->GetStride(&wicStride);
    if (FAILED(hr))
        return hr;

    UINT dataSize = 0;
    WICInProcPointer data = nullptr;
    hr = lock->GetDataPointer(&dataSize, &data);
    if (FAILED(hr) || !data)
        return hr;

    // Create DIB (Device Independent Bitmap) - Top-down (negative height)
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(width);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(height); // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = static_cast<UINT>(dibStride * height);

    HDC hdc = GetDC(nullptr);
    if (!hdc)
        return E_FAIL;

    void* bits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!hBitmap)
    {
        ReleaseDC(nullptr, hdc);
        return E_FAIL;
    }

    if (bits && data)
    {
        // Copy data row by row (handling different stride between WIC and DIB)
        const BYTE* srcPtr = static_cast<const BYTE*>(data);
        BYTE* dstPtr = static_cast<BYTE*>(bits);
        const UINT copyBytes = (std::min)(wicStride, dibStride);
        for (UINT y = 0; y < height; y++)
        {
            CopyMemory(dstPtr + y * dibStride, srcPtr + y * wicStride, copyBytes);
        }
    }

    ReleaseDC(nullptr, hdc);
    *outputBitmap = hBitmap;
    return S_OK;
}
