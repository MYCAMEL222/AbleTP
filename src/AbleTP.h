#ifndef ABLETP_H
#define ABLETP_H

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

/// datums for canvasPrintText function
#define T_L 0 // Top left (default)
#define T_C 1 // Top centre
#define T_R 2 // Top right
#define M_L 3 // Middle left
#define M_C 4 // Middle centre
#define M_R 5 // Middle right
#define B_L 6 // Bottom left
#define B_C 7 // Bottom centre
#define B_R 8 // Bottom right

class AbleTP
{
public:
    AbleTP(); // Constructor without parameters
    ~AbleTP();

    void begin();                                  // Initialize without BLE
    void begin(BLECharacteristic &characteristic); // Initialize with BLE setup
    /**
     * Reset the printer to default settings
     */
    void reset(); // Reset the printer
    /**
     * 1 for BOLD, 0 for normal
     */
    void setBold(bool enable); // Set text to bold
    /**
     * 0: Font A, 1: Font B, 2: Font C (if available)
     */
    void setFont(uint8_t font);
    /**
     * 0: Normal, 1: Invert color
     */
    void setInvertColor(bool enable);
    /**
     * Sets the line spacing to n × (vertical motion unit).
     */
    void setLineSpace(uint8_t space);
    /**
     * Sets the line spacing to the "default line spacing."
     */
    void resetLineSpace();
    /**
     * 0 = OFF, 1 = ON (1-dot thick), 2 = ON (2-dots thick)
     */
    void setUnderline(uint8_t mode);
    /**
     * 0 = Left, 1 = Center, 2 = Right
     */
    void setAlignment(uint8_t align);
    /**
     *Scale text size (width(x),height(x)) 8x max. Some printers might not handle all settings.
     */
    void setTextScale(uint8_t width, uint8_t height); // Set text scaling
    /**
     * Moves the print position (hpos) horiz pixles
     * from the left edge of the print area.
     * Works with normal and custom text.
     */
    void setHorizPos(uint16_t hpos);
    /**
     * Prints text string in quotes. Previous settings determine font, size, etc.
     */
    void printText(const char *text); // Print text to the printer
    /**
     * Prints a custom image to the printer. Image must be in 1-bit format.
     * Width must be divisible by 8. File stored in *.h file.
     * invert: true for black background, false for white background.
     */
    void printImage(const uint8_t *image, uint16_t width, uint16_t height, bool invert);
    /**
     * Prints custom text string, (x) horiz pixels from left, in &font.
     * setTextScale() will affect text size.
     */
    void printCustomText(const char *text, uint16_t hpos, const GFXfont *font);
    /**
     * qrModel: 1 or 2. Many printers default to 1.
     * qrSize 1: 21x21 modules, 2: 25x25, 3: 29x29, 4: 33x33, ... qrModel 2 = Max 16
     * Error correction level: 0 = Low, 1 = Medium, 2 = Quartile, 3 = High.
     */
    void printQRCode(const char *qrData, uint8_t qrModel, uint8_t qrSize, uint8_t qrErrorLevel);
    /**
     * Data should be 11 to 12 numbers long. (0 - 9, numbers only)
     * The 12th checksum digit will be added or changed by printer.
     */
    void printUPCA(const char *data, uint8_t width, uint8_t height, uint8_t hriFont, uint8_t hriPosition);
    /**
     * Data should be 12 OR 13 numbers long. (0 - 9, numbers only)
     * The 13th checksum digit will be added or changed by printer.
     */
    void printEAN13(const char *data, uint8_t width, uint8_t height, uint8_t hriFont, uint8_t hriPosition);
    /**
     * Includes all numeric values (0-9),
     * alphabetic characters (A-Z, a-z)
     * punctuation marks.
     */
    void printCODE128(const char *data, uint8_t width, uint8_t height, uint8_t hriFont, uint8_t hriPosition);
    /** All numeric digits (0-9), All uppercase letters (A-Z), %, +, $, /, ., -
     * Some printers may print a checksum digit in barcode, not in HRI
     */
    void printCODE39(const char *data, uint8_t width, uint8_t height, uint8_t hriFont, uint8_t hriPosition);

    // Canvas Commands
    /** Set the size of the canvas (buffer area) for drawing text, shapes, and graphics
     */
    void canvasSetSize(int width, int height); // Ensure this is uncommented and visible
    /** Set the text datum for canvas text printing.
     * Text will justify to this point.
     * Values: T_L, T_C, T_R, M_L, M_C, M_R, B_L, B_C, B_R
     * T=top, M=middle, B=bottom, L=left, C=center, R=right
     * (Top left) is default)
     */
    void canvasTextDatum(uint8_t datum); // Use the TextDatum type
    /**
     * Draw a single pixel at x, y on the canvas in (0 or 1) color.
     */
    void canvasDrawPixel(int16_t x, int16_t y, uint16_t color);
    /**
     * Draw a line from x0, y0 to x1, y1 on the canvas in (0 or 1) color.
     */
    void canvasDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    /**
     * Draw a horiz line from x0, y0, width of w on the canvas in (0 or 1) color.
     */
    void canvasDrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    /*!
     * Draw a vert line from x0, y0, height of h on the canvas in (0 or 1) color.
     */
    void canvasDrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    /**
     * Draw a rectangle at x, y (upper left)
     * with width w, height h on the canvas in (0 or 1) color.
     */
    void canvasDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    /**
     * Fill a rectangle at x, y (upper left)
     * with width w, height h on the canvas in (0 or 1) color.
     */
    void canvasFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    /**
     * Draw a circle at x, y (center)
     * with radius r on the canvas in (0 or 1) color.
     */
    void canvasDrawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    /**
     * Fill a circle at x, y (center)
     * with radius r on the canvas in (0 or 1) color.
     */
    void canvasFillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    /**
     * Draw a triangle from point x0, y0 to point x1, y1
     * to point x2, y2 on the canvas in (0 or 1) color.
     */
    void canvasDrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    /**
     * Fill a triangle from point x0, y0 to point x1, y1
     * to point x2, y2 on the canvas in (0 or 1) color.
     */
    void canvasFillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    /**
     * Draw a rounded rectangle at x, y (upper left)
     * with width w, height h and radius r
     * on the canvas in (0 or 1) color.
     */
    void canvasDrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    /**
     * Fill a rounded rectangle at x, y (upper left)
     * with width w, height h and radius r
     * on the canvas in (0 or 1) color.
     */
    void canvasFillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    /**
     * Print a custom image to the canvas. Image must be in 1-bit format.
     * Width must be divisible by 8.
     * invert: true for black background, false for white background.
     */
    void canvasPrintImage(const uint8_t *image, uint16_t width, uint16_t height, int16_t startX, int16_t startY, bool invert);
    /**
     * Scale text size (width(x),height(x)) 8x max.
     * Some printers might not handle all settings.
     */
    void canvasSetTextScale(uint8_t sx, uint8_t sy);
    /**
     * Set the cursor position for text printing on the canvas.
     * Text DATUM will affect justification.
     */
    void canvasSetCursor(int16_t x, int16_t y);
    /**
     * Set the font for text printing on the canvas.
     */
    void canvasSetFont(const GFXfont *f);
    /**
     * Set the text color (0 or1) for text printing on the canvas.
     */
    void canvasSetTextColor(uint16_t c);
    /**
     * Set the text wrap (0 or 1) for text printing on the canvas.
     */
    void canvasSetTextWrap(bool w);
    /**
     * Print text string on the canvas.
     * Previous settings determine font, size, etc.
     */
    void canvasPrintText(const char *text);
    /**
     * Print the canvas to the printer.
     * This will send the canvas data to the printer.
     * Canvas (buffer) will be cleared after printing.
     */
    void printCanvas();
    /**
     * Send a user defined command to the printer, size_t of command.
     */
    void sendCommand(const uint8_t *command, size_t length); // Send ESC/POS command

private:
    BLECharacteristic *TPChar; // BLE characteristic for communication
    GFXcanvas1 *gfx = nullptr;
    int canvasWidth = 0, canvasHeight = 0;
    uint16_t horizPos = 0;     // Default to 0
    uint8_t mytextdatum = T_L; // Initialize with default value (Top Left)
    uint8_t *imageData = nullptr;
    uint8_t tptextsize_x = 1;
    uint8_t tptextsize_y = 1;
    uint8_t getLineSpace() const; // gets last used linespace, 8 is delault
    uint8_t tplinespace = 8;      // default printer line space
    uint8_t getTextDatum();       // Corrected to be a getter without parameters
    void PrepareImageData();      // Method to prepare image data from canvas
};

#endif // ABLETP_H
