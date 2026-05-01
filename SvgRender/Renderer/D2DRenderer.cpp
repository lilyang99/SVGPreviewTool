#include "pch.h"
#include "D2DRenderer.h"
#include "../Attribute/D2DAttributeReader.h"

bool CD2DRenderer::Initialize()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Create D2D factory
    if (!m_factory)
    {
        HRESULT hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(ID2D1Factory1),
            nullptr,
            &m_factory
        );
        if (FAILED(hr))
            return false;
    }

    // Create WIC factory
    if (!m_wic)
    {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_wic)
        );
        if (FAILED(hr))
            return false;
    }

    m_initialized = true;
    return true;
}

void CD2DRenderer::Cleanup()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_svg.Reset();
    m_context.Reset();
    m_renderTarget.Reset();
    m_wicBitmap.Reset();
    m_currentDpiX = 0;
    m_currentDpiY = 0;
    m_loaded = false;
}

HRESULT CD2DRenderer::LoadFromStream(IStream* stream)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return LoadFromStreamInternal(stream);
}

HRESULT CD2DRenderer::LoadFromFile(LPCWSTR filePath)
{
    if (!filePath)
        return E_POINTER;

    HANDLE hFile = ::CreateFile(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    LARGE_INTEGER fileSize;
    if (!::GetFileSizeEx(hFile, &fileSize))
    {
        ::CloseHandle(hFile);
        return E_FAIL;
    }

    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, fileSize.LowPart);
    if (!hGlobal)
    {
        ::CloseHandle(hFile);
        return E_OUTOFMEMORY;
    }

    BYTE* pGlobal = (BYTE*)::GlobalLock(hGlobal);
    DWORD bytesRead;
    BOOL success = ::ReadFile(hFile, pGlobal, fileSize.LowPart, &bytesRead, nullptr) &&
        bytesRead == fileSize.LowPart;

    ::GlobalUnlock(hGlobal);
    ::CloseHandle(hFile);

    if (!success)
    {
        ::GlobalFree(hGlobal);
        return E_FAIL;
    }

    IStream* pStream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
    if (SUCCEEDED(hr) && pStream)
    {
        hr = LoadFromStreamInternal(pStream);
        pStream->Release();
    }

    return hr;
}

HRESULT CD2DRenderer::LoadFromResource(UINT resourceID, LPCWSTR type)
{
    HMODULE hModule = GetModuleHandle(nullptr);
    if (!hModule)
        return E_FAIL;

    HRSRC hResource = ::FindResource(hModule, MAKEINTRESOURCE(resourceID), type);
    if (!hResource)
        return E_FAIL;

    DWORD imageSize = ::SizeofResource(hModule, hResource);
    if (imageSize == 0)
        return E_FAIL;

    HGLOBAL hGlobal = ::LoadResource(hModule, hResource);
    if (!hGlobal)
        return E_FAIL;

    LPVOID pResourceData = ::LockResource(hGlobal);
    if (!pResourceData)
        return E_FAIL;

    HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (!hBuffer)
        return E_OUTOFMEMORY;

    LPVOID pBuffer = ::GlobalLock(hBuffer);
    if (pBuffer)
    {
        CopyMemory(pBuffer, pResourceData, imageSize);
        ::GlobalUnlock(hBuffer);

        IStream* pStream = nullptr;
        HRESULT hr = CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);
        if (SUCCEEDED(hr) && pStream)
        {
            hr = LoadFromStreamInternal(pStream);
            pStream->Release();
        }

        ::GlobalFree(hBuffer);
        return hr;
    }

    ::GlobalUnlock(hBuffer);
    ::GlobalFree(hBuffer);
    return E_FAIL;
}

HRESULT CD2DRenderer::RenderToWICBitmap(
    IWICBitmap* destination,
    FLOAT dpiX,
    FLOAT dpiY)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_initialized || !m_loaded || !destination)
        return E_FAIL;

    // Always recreate render target from the destination bitmap, since each
    // call to this method may pass a different destination WIC bitmap.
    m_context.Reset();
    m_renderTarget.Reset();

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    HRESULT hr = m_factory->CreateWicBitmapRenderTarget(destination, &props, &m_renderTarget);
    if (FAILED(hr))
        return hr;

    ID2D1DeviceContext5* context = nullptr;
    hr = m_renderTarget->QueryInterface(__uuidof(ID2D1DeviceContext5), reinterpret_cast<void**>(&context));
    if (SUCCEEDED(hr) && context)
        m_context.Attach(context);
    else
        return hr;

    m_currentDpiX = dpiX;
    m_currentDpiY = dpiY;

    m_context->BeginDraw();
    m_context->Clear();
    m_context->SetDpi(m_currentDpiX, m_currentDpiY);
    m_context->DrawSvgDocument(m_svg.Get());

    hr = m_context->EndDraw();
    // WIC bitmap render target is software-only — EndDraw commits
    // synchronously to the bitmap. Flush() is for GPU command queues
    // and is neither needed nor supported here.
    if (FAILED(hr))
        return hr;

    return S_OK;
}

FLOAT CD2DRenderer::GetCurrentDPI() const
{
    return m_currentDpiX;
}

bool CD2DRenderer::IsValid() const
{
    return m_initialized && m_loaded && m_svg != nullptr;
}

ID2D1SvgDocument* CD2DRenderer::GetSvgDocument() const
{
    return m_svg.Get();
}

HRESULT CD2DRenderer::GetWICBitmap(IWICBitmap** bitmap)
{
    if (!bitmap || !m_wicBitmap)
        return E_POINTER;

    m_wicBitmap->AddRef();
    *bitmap = m_wicBitmap.Get();
    return S_OK;
}

HRESULT CD2DRenderer::LoadFromStreamInternal(IStream* stream)
{
    if (!stream)
        return E_POINTER;

    // Clean up old resources
    m_svg.Reset();
    m_context.Reset();
    m_renderTarget.Reset();
    m_wicBitmap.Reset();
    m_currentDpiX = 0;
    m_currentDpiY = 0;

    // First pass: use a temporary viewport to parse the SVG
    D2D1_SIZE_F tempViewportSize = D2D1::SizeF(100, 100);

    // Create WIC bitmap with temporary size
    HRESULT hr = m_wic->CreateBitmap(
        static_cast<UINT>(tempViewportSize.width),
        static_cast<UINT>(tempViewportSize.height),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapCacheOnLoad,
        &m_wicBitmap
    );
    if (FAILED(hr))
        return hr;

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    hr = m_factory->CreateWicBitmapRenderTarget(m_wicBitmap.Get(), &props, &m_renderTarget);
    if (FAILED(hr))
        return hr;

    hr = m_renderTarget.As(&m_context);
    if (FAILED(hr) || !m_context)
        return hr;

    // Create SVG document with temporary viewport (just for parsing)
    hr = m_context->CreateSvgDocument(stream, tempViewportSize, &m_svg);
    if (FAILED(hr))
        return hr;

    // Read actual SVG size from the document
    D2D1_SIZE_F actualViewportSize = tempViewportSize;
    {
        CD2DAttributeReader attributeReader;
        hr = attributeReader.Initialize(m_svg);
        if (FAILED(hr))
            return hr;

        SIZE actualSize = attributeReader.GetOriginalSize();
        if (actualSize.cx > 0 && actualSize.cy > 0)
        {
            actualViewportSize = D2D1::SizeF(
                static_cast<FLOAT>(actualSize.cx),
                static_cast<FLOAT>(actualSize.cy));
        }
    } // attributeReader destroyed here — releases SVG ref while context is alive

    // If actual size differs, recreate everything with correct viewport
    if (actualViewportSize.width != tempViewportSize.width ||
        actualViewportSize.height != tempViewportSize.height)
    {
        // Rewind stream to beginning
        LARGE_INTEGER seekPos = {};
        hr = stream->Seek(seekPos, STREAM_SEEK_SET, nullptr);
        if (FAILED(hr))
            return hr;

        // Reset all resources in correct order: SVG first, then context/render target/WIC
        m_svg.Reset();
        m_context.Reset();
        m_renderTarget.Reset();
        m_wicBitmap.Reset();

        // Recreate WIC bitmap with correct size
        hr = m_wic->CreateBitmap(
            static_cast<UINT>(actualViewportSize.width),
            static_cast<UINT>(actualViewportSize.height),
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnLoad,
            &m_wicBitmap
        );
        if (FAILED(hr))
            return hr;

        // Recreate RenderTarget from correct-size bitmap
        hr = m_factory->CreateWicBitmapRenderTarget(m_wicBitmap.Get(), &props, &m_renderTarget);
        if (FAILED(hr))
            return hr;

        hr = m_renderTarget.As(&m_context);
        if (FAILED(hr) || !m_context)
            return hr;

        // Recreate SVG document with correct viewport size
        hr = m_context->CreateSvgDocument(stream, actualViewportSize, &m_svg);
        if (FAILED(hr))
            return hr;
    }

    m_loaded = true;
    return S_OK;
}
