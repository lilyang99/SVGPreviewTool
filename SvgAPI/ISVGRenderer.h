#pragma once

#include <d2d1_3.h>
#include <d2d1svg.h>
#include <winerror.h>

/**
 * @brief SVG renderer interface
 *
 * Responsible for rendering SVG to Direct2D render target
 */
class ISVGRenderer
{
public:
    virtual ~ISVGRenderer() = default;

    /**
     * @brief Initialize the renderer
     *
     * @return true on success, false on failure
     */
    virtual bool Initialize() = 0;

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
     * @brief Render SVG to a WIC bitmap
     *
     * @param destination The destination WIC bitmap
     * @param dpiX The horizontal DPI for rendering
     * @param dpiY The vertical DPI for rendering
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT RenderToWICBitmap(IWICBitmap* destination, FLOAT dpiX, FLOAT dpiY) = 0;

    /**
     * @brief Get the current DPI value
     *
     * @return Current DPI (dots per inch)
     */
    virtual FLOAT GetCurrentDPI() const = 0;

    /**
     * @brief Check if the renderer is in a valid state
     *
     * @return true if valid, false otherwise
     */
    virtual bool IsValid() const = 0;

protected:
    /**
     * @brief Get the underlying SVG document
     *
     * @return Pointer to ID2D1SvgDocument, or nullptr if not loaded
     */
    virtual ID2D1SvgDocument* GetSvgDocument() const = 0;

    /**
     * @brief Get the rendered WIC bitmap
     *
     * @param[out] bitmap Receives the WIC bitmap pointer
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT GetWICBitmap(IWICBitmap** bitmap) = 0;
};
