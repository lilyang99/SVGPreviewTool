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

    // Reject files larger than 4 GB (SVG files are typically KB–MB)
    if (fileSize.HighPart > 0)
    {
        ::CloseHandle(hFile);
        return E_INVALIDARG;
    }

    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, fileSize.LowPart);
    if (!hGlobal)
    {
        ::CloseHandle(hFile);
        return E_OUTOFMEMORY;
    }

    BYTE* pGlobal = static_cast<BYTE*>(::GlobalLock(hGlobal));
    if (!pGlobal)
    {
        ::CloseHandle(hFile);
        ::GlobalFree(hGlobal);
        return E_OUTOFMEMORY;
    }

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
    if (!pBuffer)
    {
        ::GlobalFree(hBuffer);
        return E_FAIL;
    }

    CopyMemory(pBuffer, pResourceData, imageSize);
    ::GlobalUnlock(hBuffer);

    IStream* pStream = nullptr;
    // fDeleteOnRelease = TRUE: pStream->Release() will call GlobalFree(hBuffer)
    HRESULT hr = CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);
    if (FAILED(hr) || !pStream)
    {
        ::GlobalFree(hBuffer);
        return FAILED(hr) ? hr : E_FAIL;
    }

    hr = LoadFromStreamInternal(pStream);
    pStream->Release();  // Also frees hBuffer (fDeleteOnRelease = TRUE)
    return hr;
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

    hr = m_renderTarget.As(&m_context);
    if (FAILED(hr) || !m_context)
        return FAILED(hr) ? hr : E_FAIL;

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
    std::lock_guard<std::mutex> lock(m_mutex);
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

    if (!m_wic || !m_factory)
        return E_FAIL;

    // Build new state in local ComPtrs. Member variables are NOT modified
    // until the entire load succeeds — provides strong exception safety.
    D2D1_SIZE_F tempViewportSize = D2D1::SizeF(100, 100);

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    Microsoft::WRL::ComPtr<IWICBitmap> newWicBitmap;
    HRESULT hr = m_wic->CreateBitmap(
        static_cast<UINT>(tempViewportSize.width),
        static_cast<UINT>(tempViewportSize.height),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapCacheOnLoad,
        &newWicBitmap
    );
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID2D1RenderTarget> newRenderTarget;
    hr = m_factory->CreateWicBitmapRenderTarget(newWicBitmap.Get(), &props, &newRenderTarget);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ID2D1DeviceContext5> newContext;
    hr = newRenderTarget.As(&newContext);
    if (FAILED(hr) || !newContext)
        return FAILED(hr) ? hr : E_FAIL;

    Microsoft::WRL::ComPtr<ID2D1SvgDocument> newSvg;
    hr = newContext->CreateSvgDocument(stream, tempViewportSize, &newSvg);
    if (FAILED(hr))
        return hr;

    // Read actual SVG size from the document
    D2D1_SIZE_F actualViewportSize = tempViewportSize;
    {
        CD2DAttributeReader attributeReader;
        hr = attributeReader.Initialize(newSvg);
        if (FAILED(hr))
            return hr;

        SIZE actualSize = attributeReader.GetOriginalSize();
        if (actualSize.cx > 0 && actualSize.cy > 0)
        {
            actualViewportSize = D2D1::SizeF(
                static_cast<FLOAT>(actualSize.cx),
                static_cast<FLOAT>(actualSize.cy));
        }
    } // attributeReader destroyed — releases its SVG ref while context is alive

    // If actual size differs from temp, recreate with correct viewport
    if (actualViewportSize.width != tempViewportSize.width ||
        actualViewportSize.height != tempViewportSize.height)
    {
        LARGE_INTEGER seekPos = {};
        hr = stream->Seek(seekPos, STREAM_SEEK_SET, nullptr);
        if (FAILED(hr))
            return hr;

        // Release temp objects before rebuilding
        newSvg.Reset();
        newContext.Reset();
        newRenderTarget.Reset();
        newWicBitmap.Reset();

        hr = m_wic->CreateBitmap(
            static_cast<UINT>(actualViewportSize.width),
            static_cast<UINT>(actualViewportSize.height),
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnLoad,
            &newWicBitmap
        );
        if (FAILED(hr))
            return hr;

        hr = m_factory->CreateWicBitmapRenderTarget(newWicBitmap.Get(), &props, &newRenderTarget);
        if (FAILED(hr))
            return hr;

        hr = newRenderTarget.As(&newContext);
        if (FAILED(hr) || !newContext)
            return FAILED(hr) ? hr : E_FAIL;

        hr = newContext->CreateSvgDocument(stream, actualViewportSize, &newSvg);
        if (FAILED(hr))
            return hr;
    }

    // All steps succeeded — commit to members (old state released by ComPtr =)
    m_svg = newSvg;
    m_context = newContext;
    m_renderTarget = newRenderTarget;
    m_wicBitmap = newWicBitmap;
    m_currentDpiX = 0;
    m_currentDpiY = 0;
    m_loaded = true;

    return S_OK;
}
