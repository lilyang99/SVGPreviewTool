#include "pch.h"
#include "D2DAttributeReader.h"

HRESULT CD2DAttributeReader::Initialize(Microsoft::WRL::ComPtr<ID2D1SvgDocument> svgDocument)
{
    m_svgDocument = svgDocument;
    m_valid = (svgDocument != nullptr);
    return m_valid ? S_OK : E_FAIL;
}

HRESULT CD2DAttributeReader::GetViewBox(D2D1_SVG_VIEWBOX* viewBox) const
{
    if (!viewBox || !m_valid)
        return E_FAIL;

    Microsoft::WRL::ComPtr<ID2D1SvgElement> root;
    m_svgDocument->GetRoot(&root);
    if (!root)
        return E_FAIL;

    HRESULT hr = root->GetAttributeValue(
        L"viewBox",
        D2D1_SVG_ATTRIBUTE_POD_TYPE_VIEWBOX,
        viewBox,
        sizeof(*viewBox)
    );

    return hr;
}

SIZE CD2DAttributeReader::GetOriginalSize() const
{
    if (!m_valid)
        return SIZE{0, 0};

    // Priority 1: viewBox defines the coordinate system
    D2D1_SVG_VIEWBOX viewBox = {};
    if (SUCCEEDED(GetViewBox(&viewBox)) && viewBox.width > 0 && viewBox.height > 0)
    {
        return SIZE{static_cast<INT>(viewBox.width), static_cast<INT>(viewBox.height)};
    }

    // Priority 2: width/height attributes from the root <svg> element
    Microsoft::WRL::ComPtr<ID2D1SvgElement> root;
    m_svgDocument->GetRoot(&root);
    if (root)
    {
        D2D1_SVG_LENGTH svgWidth = {}, svgHeight = {};
        if (SUCCEEDED(root->GetAttributeValue(L"width", D2D1_SVG_ATTRIBUTE_POD_TYPE_LENGTH, &svgWidth, sizeof(svgWidth))) &&
            SUCCEEDED(root->GetAttributeValue(L"height", D2D1_SVG_ATTRIBUTE_POD_TYPE_LENGTH, &svgHeight, sizeof(svgHeight))) &&
            svgWidth.value > 0 && svgHeight.value > 0)
        {
            return SIZE{static_cast<INT>(svgWidth.value), static_cast<INT>(svgHeight.value)};
        }
    }

    // Priority 3: fallback to viewport size
    D2D1_SIZE_F viewportSize = m_svgDocument->GetViewportSize();
    if (viewportSize.width > 0 && viewportSize.height > 0)
    {
        return SIZE{static_cast<INT>(viewportSize.width), static_cast<INT>(viewportSize.height)};
    }

    return SIZE{0, 0};
}

bool CD2DAttributeReader::IsValid() const
{
    return m_valid && m_svgDocument != nullptr;
}
