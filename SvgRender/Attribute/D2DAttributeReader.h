#ifndef SVGRENDER_ATTRIBUTE_D2DATTRIBUTEREADER_H_
#define SVGRENDER_ATTRIBUTE_D2DATTRIBUTEREADER_H_

#include "../../SvgAPI/ISVGAttributeReader.h"
#include <wrl.h>
#include <d2d1_3.h>
#include <d2d1svg.h>

/**
 * @brief Direct2D SVG attribute reader implementation
 *
 * Implements attribute reading functionality based on ID2D1SvgDocument
 */
class CD2DAttributeReader : public ISVGAttributeReader
{
public:
    CD2DAttributeReader() = default;
    virtual ~CD2DAttributeReader() = default;

    /**
     * @brief Initialize with an SVG document
     *
     * @param svgDocument The SVG document to read attributes from
     * @return HRESULT indicating success or failure
     */
    HRESULT Initialize(const Microsoft::WRL::ComPtr<ID2D1SvgDocument>& svgDocument);

    // ISVGAttributeReader implementation
    virtual HRESULT GetViewBox(D2D1_SVG_VIEWBOX* viewBox) const override;
    virtual SIZE GetOriginalSize() const override;
    virtual bool IsValid() const override;

private:
    Microsoft::WRL::ComPtr<ID2D1SvgDocument> m_svgDocument;
    bool m_valid = false;
};

#endif // SVGRENDER_ATTRIBUTE_D2DATTRIBUTEREADER_H_

