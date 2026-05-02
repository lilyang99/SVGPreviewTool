#include "pch.h"
#include "CSVGImage.h"
#include <Gdiplus.h>

CSVGImage::CSVGImage()
{
    m_renderer = std::make_unique<CD2DRenderer>();
    m_attributeReader = std::make_unique<CD2DAttributeReader>();
}

CSVGImage::~CSVGImage()
{
    Cleanup();
}

CSVGImage::CSVGImage(CSVGImage&& other) noexcept
    : m_renderer(std::move(other.m_renderer))
    , m_attributeReader(std::move(other.m_attributeReader))
    , m_converter(std::move(other.m_converter))
    , m_currentDPI(other.m_currentDPI)
    , m_originalSize(other.m_originalSize)
{
    other.m_currentDPI = USER_DEFAULT_SCREEN_DPI;
    other.m_originalSize = {0, 0};
}

CSVGImage& CSVGImage::operator=(CSVGImage&& other) noexcept
{
    if (this != &other)
    {
        Cleanup();
        m_renderer = std::move(other.m_renderer);
        m_attributeReader = std::move(other.m_attributeReader);
        m_converter = std::move(other.m_converter);
        m_currentDPI = other.m_currentDPI;
        m_originalSize = other.m_originalSize;
        other.m_currentDPI = USER_DEFAULT_SCREEN_DPI;
        other.m_originalSize = {0, 0};
    }
    return *this;
}

bool CSVGImage::Initialize(UINT dpi)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_renderer)
    {
        return false;
    }

    // Initialize renderer
    if (!m_renderer->Initialize())
    {
        return false;
    }

    m_currentDPI = dpi;

    // Initialize attribute reader
    if (!m_attributeReader)
    {
        return false;
    }

    return true;
}

void CSVGImage::Cleanup()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Destroy converter first (holds raw pointers to renderer/reader)
    m_converter.reset();

    // Destroy attribute reader BEFORE renderer cleanup:
    // attribute reader holds ComPtr<ID2D1SvgDocument> which shares the SVG
    // with renderer. If renderer releases m_context first, the SVG destructor
    // (triggered later by attribute reader) would access freed D2D context.
    m_attributeReader.reset();

    if (m_renderer)
    {
        m_renderer->Cleanup();
    }
}

// ---------------------------------------------------------------------------
// LoadInternal — common load pipeline shared by LoadFromStream/File/Resource
// ---------------------------------------------------------------------------
template <typename LoadFunc>
HRESULT CSVGImage::LoadInternal(HRESULT errorOnInvalid, LoadFunc&& loadFunc)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_renderer)
        return errorOnInvalid;

    // Save old state for rollback on failure
    auto oldReader = std::move(m_attributeReader);
    auto oldConverter = std::move(m_converter);
    m_converter.reset();
    m_attributeReader = std::make_unique<CD2DAttributeReader>();

    HRESULT hr = loadFunc();
    if (FAILED(hr))
    {
        m_attributeReader = std::move(oldReader);
        m_converter = std::move(oldConverter);
        return hr;
    }

    ID2D1SvgDocument* doc = m_renderer->GetSvgDocument();
    if (doc)
    {
        hr = m_attributeReader->Initialize(doc);
        if (FAILED(hr))
        {
            m_attributeReader = std::move(oldReader);
            m_converter = std::move(oldConverter);
            return hr;
        }
        m_originalSize = m_attributeReader->GetOriginalSize();
    }

    m_converter = std::make_unique<CSVGImageConverter>(m_renderer.get(), m_attributeReader.get());
    return S_OK;
}

HRESULT CSVGImage::LoadFromStream(IStream* stream)
{
    if (!stream)
        return E_INVALIDARG;
    return LoadInternal(E_INVALIDARG, [&]() { return m_renderer->LoadFromStream(stream); });
}

HRESULT CSVGImage::LoadFromFile(LPCWSTR filePath)
{
    return LoadInternal(E_UNEXPECTED, [&]() { return m_renderer->LoadFromFile(filePath); });
}

HRESULT CSVGImage::LoadFromResource(UINT resourceID, LPCWSTR type)
{
    return LoadInternal(E_UNEXPECTED, [&]() { return m_renderer->LoadFromResource(resourceID, type); });
}

SIZE CSVGImage::GetOriginalSize() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_originalSize;
}

UINT CSVGImage::GetCurrentDPI() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentDPI;
}

HRESULT CSVGImage::SetDPI(UINT dpi)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (dpi == m_currentDPI)
    {
        return S_OK;
    }

    m_currentDPI = dpi;
    return S_OK;
}

HBITMAP CSVGImage::ToHBITMAP()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_converter)
    {
        return nullptr;
    }

    return m_converter->ConvertToBitmap(m_currentDPI);
}

Gdiplus::Bitmap* CSVGImage::ToGdiPlusBitmap()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_converter)
    {
        return nullptr;
    }

    return m_converter->ConvertToGdiPlusBitmap(m_currentDPI);
}

HICON CSVGImage::ToHICON()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_converter)
    {
        return nullptr;
    }

    return m_converter->ConvertToIcon(m_currentDPI);
}

bool CSVGImage::IsValid() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_renderer || !m_attributeReader)
    {
        return false;
    }

    return m_attributeReader->IsValid();
}

ISVGAttributeReader* CSVGImage::GetAttributeReader()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_attributeReader.get();
}
