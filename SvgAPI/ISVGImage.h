#ifndef SVGAPI_ISVGIMAGE_H_
#define SVGAPI_ISVGIMAGE_H_

#include <windows.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <d2d1svg.h>
#include <msxml6.h>
#include <ppltasks.h>
#include <memory>
#include <string>
#include <Gdiplus.h>
#include "ISVGAttributeReader.h"
#include "ISVGRenderer.h"

/**
 * @brief SVG image interface
 *
 * Main user interface for SVG images.
 * Composes Renderer, AttributeReader, and Converter components.
 */
class ISVGImage
{
public:
    virtual ~ISVGImage() = default;

    /**
     * @brief Initialize the SVG image
     *
     * @param dpi The DPI value for rendering (default 96)
     * @return true on success, false on failure
     */
    virtual bool Initialize(UINT dpi = USER_DEFAULT_SCREEN_DPI) = 0;

    /**
     * @brief Cleanup all resources
     */
    virtual void Cleanup() = 0;

    /**
     * @brief Load SVG from a stream
     *
     * @param stream The input stream containing SVG data
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT LoadFromStream(IStream* stream) = 0;

    /**
     * @brief Load SVG from a file
     *
     * @param filePath Path to the SVG file
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT LoadFromFile(LPCWSTR filePath) = 0;

    /**
     * @brief Load SVG from an embedded resource
     *
     * @param resourceID The resource identifier
     * @param type The resource type
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT LoadFromResource(UINT resourceID, LPCWSTR type) = 0;

    /**
     * @brief Get the original SVG size
     *
     * @return SIZE structure with width and height
     */
    virtual SIZE GetOriginalSize() const = 0;

    /**
     * @brief Get the current DPI value
     *
     * @return Current DPI (dots per inch)
     */
    virtual UINT GetCurrentDPI() const = 0;

    /**
     * @brief Set the DPI value
     *
     * @param dpi The new DPI value
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT SetDPI(UINT dpi) = 0;

    /**
     * @brief Convert SVG to HBITMAP
     *
     * @return HBITMAP handle, or nullptr on failure
     */
    virtual HBITMAP ToHBITMAP() = 0;

    /**
     * @brief Convert SVG to GDI+ Bitmap
     *
     * @return Gdiplus::Bitmap pointer, caller owns the returned object
     */
    virtual Gdiplus::Bitmap* ToGdiPlusBitmap() = 0;

    /**
     * @brief Convert SVG to HICON
     *
     * @return HICON handle, or nullptr on failure
     */
    virtual HICON ToHICON() = 0;

    /**
     * @brief Check if the SVG image is in a valid state
     *
     * @return true if valid, false otherwise
     */
    virtual bool IsValid() const = 0;

    /**
     * @brief Get the attribute reader
     *
     * @return Pointer to ISVGAttributeReader
     */
    virtual ISVGAttributeReader* GetAttributeReader() = 0;
};

#endif // SVGAPI_ISVGIMAGE_H_
