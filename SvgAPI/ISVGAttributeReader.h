#pragma once

#include <d2d1svg.h>
#include <winerror.h>
#include <sal.h>

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
