#ifndef SVGAPI_ISVGATTRIBUTEREADER_H_
#define SVGAPI_ISVGATTRIBUTEREADER_H_

#include <d2d1svg.h>
#include <winerror.h>
#include <sal.h>

// SVG attribute name constants
#define SVG_ATTRIBUTE_VIEWBOX  L"viewBox"
#define SVG_ATTRIBUTE_WIDTH    L"width"
#define SVG_ATTRIBUTE_HEIGHT   L"height"

/**
 * @brief SVG attribute reader interface
 *
 * Responsible for reading SVG document attributes and metadata
 */
class ISVGAttributeReader
{
public:
    virtual ~ISVGAttributeReader() = default;

    /**
     * @brief Get the SVG viewBox
     *
     * @param[out] viewBox Receives the viewBox dimensions
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT GetViewBox(D2D1_SVG_VIEWBOX* viewBox) const = 0;

    /**
     * @brief Get the root <svg> element width and height attributes
     *
     * @param[out] width Receives the width length
     * @param[out] height Receives the height length
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT GetRootSize(D2D1_SVG_LENGTH* width, D2D1_SVG_LENGTH* height) const = 0;

    /**
     * @brief Get the original SVG size
     *
     * @return SIZE structure with width and height
     */
    virtual SIZE GetOriginalSize() const = 0;

    /**
     * @brief Check if the attribute reader is in a valid state
     *
     * @return true if valid, false otherwise
     */
    virtual bool IsValid() const = 0;
};

#endif // SVGAPI_ISVGATTRIBUTEREADER_H_
