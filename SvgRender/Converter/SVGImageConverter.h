#ifndef SVGRENDER_CONVERTER_SVGIMAGECONVERTER_H_
#define SVGRENDER_CONVERTER_SVGIMAGECONVERTER_H_

#include <d2d1_3.h>
#include <d2d1svg.h>
#include <wincodec.h>
#include <Gdiplus.h>
#include <wrl.h>

class ISVGRenderer;
class ISVGAttributeReader;

/**
 * @brief SVG image converter
 *
 * Responsible for converting SVG rendering results to different output formats
 */
class CSVGImageConverter
{
public:
    /**
     * @brief Constructor
     *
     * @param renderer The SVG renderer instance
     * @param attributeReader The SVG attribute reader instance
     */
    CSVGImageConverter(
        ISVGRenderer* renderer,
        ISVGAttributeReader* attributeReader);
    virtual ~CSVGImageConverter() = default;

    /**
     * @brief Convert to HBITMAP
     *
     * @param dpi The DPI value for rendering
     * @return HBITMAP handle
     */
    HBITMAP ConvertToBitmap(UINT dpi);

    /**
     * @brief Convert to GDI+ Bitmap
     *
     * @param dpi The DPI value for rendering
     * @return Gdiplus::Bitmap pointer, caller owns the returned object
     */
    Gdiplus::Bitmap* ConvertToGdiPlusBitmap(UINT dpi);

    /**
     * @brief Convert to HICON
     *
     * @param dpi The DPI value for rendering
     * @return HICON handle
     */
    HICON ConvertToIcon(UINT dpi);

private:
    /**
     * @brief Render SVG to a WIC bitmap
     *
     * @param[out] outputBitmap The resulting WIC bitmap
     * @param[out] width The width of the rendered bitmap
     * @param[out] height The height of the rendered bitmap
     * @param dpi The DPI value for rendering
     * @return HRESULT indicating success or failure
     */
    HRESULT RenderToWICBitmap(
        IWICBitmap** outputBitmap,
        UINT* width,
        UINT* height,
        UINT dpi);

    /**
     * @brief Copy a WIC bitmap to an HBITMAP
     *
     * @param wicBitmap The source WIC bitmap
     * @param[out] outputBitmap The resulting HBITMAP
     * @return HRESULT indicating success or failure
     */
    HRESULT CopyWICBitmapToHBITMAP(
        IWICBitmap* wicBitmap,
        HBITMAP* outputBitmap);

    ISVGRenderer* m_renderer;
    ISVGAttributeReader* m_attributeReader;
    Microsoft::WRL::ComPtr<IWICImagingFactory> m_wicFactory;
};

#endif // SVGRENDER_CONVERTER_SVGIMAGECONVERTER_H_

