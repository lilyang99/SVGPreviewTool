#ifndef SVGRENDER_IMAGE_CSVGIMAGE_H_
#define SVGRENDER_IMAGE_CSVGIMAGE_H_

#include "../../SvgAPI/ISVGImage.h"
#include "../Renderer/D2DRenderer.h"
#include "../Attribute/D2DAttributeReader.h"
#include "../Converter/SVGImageConverter.h"
#include <mutex>
#include <memory>

/**
 * @brief SVG image implementation class
 *
 * Composes Renderer, AttributeReader, and Converter components.
 */
class CSVGImage : public ISVGImage
{
public:
    CSVGImage();
    ~CSVGImage() override;

    /// @note Copy construction/assignment disabled
    CSVGImage(const CSVGImage&) = delete;
    CSVGImage& operator=(const CSVGImage&) = delete;

    /// @note Move construction/assignment enabled
    CSVGImage(CSVGImage&& other) noexcept;
    CSVGImage& operator=(CSVGImage&& other) noexcept;

    // ISVGImage interface implementation
    bool Initialize(UINT dpi = USER_DEFAULT_SCREEN_DPI) override;
    void Cleanup() override;
    HRESULT LoadFromStream(IStream* stream) override;
    HRESULT LoadFromFile(LPCWSTR filePath) override;
    HRESULT LoadFromResource(UINT resourceID, LPCWSTR type) override;
    SIZE GetOriginalSize() const override;
    UINT GetCurrentDPI() const override;
    HRESULT SetDPI(UINT dpi) override;
    HBITMAP ToHBITMAP() override;
    Gdiplus::Bitmap* ToGdiPlusBitmap() override;
    HICON ToHICON() override;
    bool IsValid() const override;
    ISVGAttributeReader* GetAttributeReader() override;

private:
    /**
     * @brief Recalculate rendered size after DPI change
     *
     * @return HRESULT indicating success or failure
     */
    HRESULT RecalculateSize();

    /// @name Components
    /// @{
    std::unique_ptr<CD2DRenderer> m_renderer;
    std::unique_ptr<CD2DAttributeReader> m_attributeReader;
    std::unique_ptr<CSVGImageConverter> m_converter;
    /// @}

    /// @brief Current DPI
    UINT m_currentDPI = USER_DEFAULT_SCREEN_DPI;

    /// @brief Original SVG size
    SIZE m_originalSize = {0, 0};

    /// @brief Thread synchronization lock
    mutable std::mutex m_mutex;
};

#endif // SVGRENDER_IMAGE_CSVGIMAGE_H_
