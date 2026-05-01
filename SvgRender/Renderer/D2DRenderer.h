#ifndef SVGRENDER_RENDERER_D2DRENDERER_H_
#define SVGRENDER_RENDERER_D2DRENDERER_H_

#include "../../SvgAPI/ISVGRenderer.h"
#include <wrl.h>
#include <d2d1_3.h>
#include <d2d1svg.h>
#include <wincodec.h>
#include <mutex>

/**
 * @brief Direct2D SVG renderer implementation
 *
 * Responsible for rendering SVG to Direct2D render target
 */
class CD2DRenderer : public ISVGRenderer
{
public:
    CD2DRenderer() = default;
    virtual ~CD2DRenderer() = default;

    /**
     * @brief Initialize the renderer
     *
     * @return true on success, false on failure
     */
    virtual bool Initialize() override;

    /**
     * @brief Cleanup all resources
     */
    virtual void Cleanup() override;

    /**
     * @brief Load SVG from a stream
     *
     * @param stream The input stream containing SVG data
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT LoadFromStream(IStream* stream) override;

    /**
     * @brief Load SVG from a file
     *
     * @param filePath Path to the SVG file
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT LoadFromFile(LPCWSTR filePath) override;

    /**
     * @brief Load SVG from an embedded resource
     *
     * @param resourceID The resource identifier
     * @param type The resource type
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT LoadFromResource(UINT resourceID, LPCWSTR type) override;

    /**
     * @brief Render SVG to a WIC bitmap
     *
     * @param destination The destination WIC bitmap
     * @param dpiX The horizontal DPI for rendering
     * @param dpiY The vertical DPI for rendering
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT RenderToWICBitmap(
        IWICBitmap* destination,
        FLOAT dpiX,
        FLOAT dpiY) override;

    /**
     * @brief Get the current DPI value
     *
     * @return Current DPI (dots per inch)
     */
    virtual FLOAT GetCurrentDPI() const override;

    /**
     * @brief Check if the renderer is in a valid state
     *
     * @return true if valid, false otherwise
     */
    virtual bool IsValid() const override;

    /**
     * @brief Get the underlying SVG document
     *
     * @return Pointer to ID2D1SvgDocument, or nullptr if not loaded
     */
    virtual ID2D1SvgDocument* GetSvgDocument() const override;

    /**
     * @brief Get the rendered WIC bitmap
     *
     * @param[out] bitmap Receives the WIC bitmap pointer
     * @return HRESULT indicating success or failure
     */
    virtual HRESULT GetWICBitmap(IWICBitmap** bitmap) override;

protected:
    /**
     * @brief Internal stream loading implementation
     *
     * @param stream The input stream
     * @return HRESULT indicating success or failure
     */
    HRESULT LoadFromStreamInternal(IStream* stream);

private:
    mutable std::mutex m_mutex;
    Microsoft::WRL::ComPtr<ID2D1Factory1> m_factory;
    Microsoft::WRL::ComPtr<IWICImagingFactory> m_wic;
    Microsoft::WRL::ComPtr<ID2D1SvgDocument> m_svg;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext5> m_context;
    Microsoft::WRL::ComPtr<ID2D1RenderTarget> m_renderTarget;
    Microsoft::WRL::ComPtr<IWICBitmap> m_wicBitmap;

    FLOAT m_currentDpiX = 0;
    FLOAT m_currentDpiY = 0;
    bool m_initialized = false;
    bool m_loaded = false;
};

#endif // SVGRENDER_RENDERER_D2DRENDERER_H_
