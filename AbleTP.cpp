#include "AbleTP.h"

AbleTP::AbleTP() : TPChar(nullptr) {}

AbleTP::~AbleTP() {}

// Initializes the AbleTP instance and prints a message to the Serial monitor.
void AbleTP::begin()
{
}

void AbleTP::begin(BLECharacteristic &characteristic)
{
    TPChar = &characteristic;
}

void AbleTP::sendCommand(const uint8_t *command, size_t length)
{
    if (TPChar != nullptr)
    {
        TPChar->writeValue(command, length); // Send the command via BLE
    }
    else
    {
        Serial.println("sendCommand Failed.");
    }
}

void AbleTP::reset()
{
    const uint8_t TP_RESET[2] = {0x1B, 0x40}; // ESC @ (Reset Printer)
    sendCommand(TP_RESET, sizeof(TP_RESET));
    // Clear the graphics buffer if it exists
    if (gfx)
    {
        gfx->fillScreen(0); // Clear the graphical buffer
    }
}

void AbleTP::printText(const char *text)
{
    if (text != nullptr && strlen(text) > 0)
    {
        sendCommand((const uint8_t *)text, strlen(text)); // Send text to printer
        sendCommand((const uint8_t[]){0x0A}, 1);          // \n (Newline)
    }
    else
    {
        Serial.println("ERROR: Empty text!");
    }
}

void AbleTP::setBold(bool enable)
{
    const uint8_t TP_BOLD_ON[3] = {0x1B, 0x45, 1};  // ESC E 1 (Bold ON)
    const uint8_t TP_BOLD_OFF[3] = {0x1B, 0x45, 0}; // ESC E 0 (Bold OFF)
    sendCommand(enable ? TP_BOLD_ON : TP_BOLD_OFF, 3);
}

void AbleTP::setFont(uint8_t font)
{
    const uint8_t TP_FONT_A[3] = {0x1B, 0x4D, 0}; // ESC M 0 (Font A)
    const uint8_t TP_FONT_B[3] = {0x1B, 0x4D, 1}; // ESC M 1 (Font B)
    const uint8_t TP_FONT_C[3] = {0x1B, 0x4D, 2}; // ESC M 2 (Font C - If available)

    switch (font)
    {
    case 0:
        sendCommand(TP_FONT_A, 3);
        break;
    case 1:
        sendCommand(TP_FONT_B, 3);
        break;
    case 2:
        sendCommand(TP_FONT_C, 3);
        break;
    default:
        sendCommand(TP_FONT_A, 3); // Default to Font A
        break;
    }
}

void AbleTP::setUnderline(uint8_t mode)
{
    if (mode > 2)
        mode = 2;                                       // Max underline mode is 2 (None, Single, Double)
    const uint8_t underlineCmd[3] = {0x1B, 0x2D, mode}; // ESC 2D N (Set underline)
    sendCommand(underlineCmd, sizeof(underlineCmd));
}

void AbleTP::setAlignment(uint8_t align)
{
    if (align > 2)
        align = 0;                                   // Default to left alignment
    const uint8_t alignCmd[3] = {0x1B, 0x61, align}; // ESC a N (Set alignment)
    sendCommand(alignCmd, sizeof(alignCmd));
}

void AbleTP::setLineSpace(uint8_t space)
{
    if (space > 255)
    {
        Serial.println("Max Line Spacing is 255!");
        space = 255; // Only set to 255 if the input is greater than 255
    }
    tplinespace = space; // Update the internal tracking variable
    const uint8_t LineSpaceCmd[3] = {0x1B, 0x33, space};
    sendCommand(LineSpaceCmd, sizeof(LineSpaceCmd));
}

void AbleTP::resetLineSpace()
{
    tplinespace = 8;
    const uint8_t LineSpaceCmd[2] = {0x1B, 0x32};
    sendCommand(LineSpaceCmd, sizeof(LineSpaceCmd));
}

uint8_t AbleTP::getLineSpace() const
{
    return tplinespace; // Return the current line space
}

void AbleTP::setInvertColor(bool invert)
{
    const uint8_t TP_INVON[3] = {0x1D, 0x42, 1};
    const uint8_t TP_INVOFF[3] = {0x1D, 0x42, 0};
    sendCommand(invert ? TP_INVON : TP_INVOFF, sizeof(TP_INVON));
}

void AbleTP::setTextScale(uint8_t width, uint8_t height)
{
    if (width > 8)
        width = 8;
    if (height > 8)
        height = 8;
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    uint8_t sizeByte = ((width - 1) << 4) | (height - 1); // Calculate size byte
    const uint8_t tsizeCmd[3] = {0x1D, 0x21, sizeByte};   // GS ! N (Set text size)
    sendCommand(tsizeCmd, sizeof(tsizeCmd));

    tptextsize_x = width;  // Store the text size for custom test printing
    tptextsize_y = height; // Store the text size
}

void AbleTP::setHorizPos(uint16_t hpos)
{
    if (hpos > 65535) // Max value for two bytes
        hpos = 0;

    horizPos = hpos; // Store the horizontal position

    uint8_t nL = hpos & 0xFF;        // Extract lower byte
    uint8_t nH = (hpos >> 8) & 0xFF; // Extract higher byte
    const uint8_t hposCmd[4] = {0x1B, 0x24, nL, nH};
    sendCommand(hposCmd, sizeof(hposCmd));
}

void AbleTP::printImage(const uint8_t *image, uint16_t width, uint16_t height, bool invert = false)
{
    if (!image)
    {
        Serial.println("ERROR: Image is null!");
        return;
    }
    if (width % 8 != 0)
    {
        Serial.println("ERROR: Image width must be divisible by 8!");
        return;
    }
    uint8_t width_bytes = width / 8; // Convert width to bytes
    uint8_t xL = width_bytes & 0xFF;
    uint8_t xH = (width_bytes >> 8) & 0xFF;
    uint8_t yL = height & 0xFF;
    uint8_t yH = (height >> 8) & 0xFF;

    uint8_t cmdDefine[] = {0x1D, 0x76, 0x30, 0x00, xL, xH, yL, yH}; // GS v 0 (Define Image)
    sendCommand(cmdDefine, sizeof(cmdDefine));

    // Allocate memory for inverted data if needed
    uint8_t *tempImage = nullptr;
    if (invert)
    {
        tempImage = (uint8_t *)malloc(width * height / 8);
        if (!tempImage)
        {
            Serial.println("ERROR: Memory allocation failed!");
            return;
        }
        for (size_t i = 0; i < (width * height / 8); i++)
        {
            tempImage[i] = ~image[i]; // Invert image data
        }
        image = tempImage; // Point to the inverted image data
    }

    // Send image data in chunks
    for (size_t i = 0; i < (width * height) / 8; i += 128)
    {
        TPChar->writeValue(image + i, min((size_t)128, (width * height) / 8 - i));
        delay(1); // Delay to prevent buffer overflow
    }

    // Clean up if tempImage was used
    if (tempImage)
    {
        free(tempImage);
    }
    // Print new lines after the image
    sendCommand((const uint8_t[]){0x0A}, 1); // \n (Newline)
    sendCommand((const uint8_t[]){0x0A}, 1); // \n (Newline)
}

void AbleTP::printCustomText(const char *text, uint16_t hpos, const GFXfont *font)
{

    if (!text || strlen(text) == 0)
    {
        Serial.println("ERROR: Empty text!");
        return;
    }
    // Create a temporary canvas to measure text size
    GFXcanvas1 tempCanvas(1, 1);
    tempCanvas.setFont(font);
    tempCanvas.setTextSize(tptextsize_x, tptextsize_y);
    tempCanvas.setTextWrap(0);
    int16_t x1, y1;
    uint16_t textWidth, textHeight;
    tempCanvas.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
    delay(10);

    if (textWidth == 0)
    {
        hpos = 1; // this sets the canvas width to 1 pixel in the event of \n
    }

    int maxOffestY = 0;
    int bottommost = 0;
    int baseln = 0;
    int advY = font->yAdvance;

    // Iterate over all glyphs in the font file
    for (int i = 0; i < (font->last - font->first + 1); i++)
    {
        char currentChar = font->first + i;
        int glyphHeight = font->glyph[i].height;
        int glyphyOff = font->glyph[i].yOffset;

        if (currentChar == 'A')
        {
            int glyphIndex = i; // Index for 'A'
            baseln = glyphyOff; // Get the yOffset for 'A'
        }
        if (glyphyOff >= maxOffestY)
        { // finds to lowest font pixel under the baseline
            maxOffestY = glyphyOff;
            if (bottommost < glyphHeight + maxOffestY)
            {
                bottommost = glyphHeight + maxOffestY;
            }
        }
    }
    maxOffestY = maxOffestY * tptextsize_y;
    bottommost = bottommost * tptextsize_y;
    baseln = baseln * tptextsize_y;
    advY = advY * tptextsize_y;
    int calcurs = advY - ((advY + baseln) / 2); // centers the text vertically on canvas
    int canvasWidth = hpos + textWidth;         // Calculate initial width including horizontal position
    canvasWidth = (canvasWidth + 7) & ~7;       // Round up to the nearest multiple of 8
    int canvasHeight = advY;                    //

    canvasWidth += (8 * tptextsize_x); // adds a little buffer padding.

    GFXcanvas1 *textCanvas = new GFXcanvas1(canvasWidth, canvasHeight);
    if (!textCanvas)
    {
        Serial.println("ERROR: Canvas memory allocation failed!");
        return;
    }
    // Allocate buffer for ESC/POS image data
    uint8_t *imageData = (uint8_t *)malloc((canvasWidth * canvasHeight) / 8);
    if (!imageData)
    {
        Serial.println("ERROR: Custom text memory allocation failed!");
        delete textCanvas;
        return;
    }
    memset(imageData, 0, (canvasWidth * canvasHeight) / 8);

    // Set the cursor at the horizontal position (hpos) and print text
    textCanvas->fillScreen(0);
    textCanvas->setFont(font);
    textCanvas->setTextSize(tptextsize_x, tptextsize_y);
    textCanvas->setTextColor(1);
    textCanvas->setCursor(hpos, calcurs);
    textCanvas->print(text);

    int index = 0;
    for (int y = 0; y < canvasHeight; y++)
    {
        for (int x = 0; x < canvasWidth / 8; x++)
        {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++)
            {
                int pixel = textCanvas->getPixel(x * 8 + bit, y);
                if (pixel > 0)
                    byte |= (1 << (7 - bit));
            }
            imageData[index++] = byte;
        }
    }

    // Send ESC/POS GS v 0 command
    uint8_t gs_v_cmd[] = {
        0x1D, 0x76, 0x30, 0,
        (uint8_t)(canvasWidth / 8), 0,
        (uint8_t)(canvasHeight & 0xFF),
        (uint8_t)((canvasHeight >> 8) & 0xFF)};

    TPChar->writeValue(gs_v_cmd, sizeof(gs_v_cmd));

    // Send image data in chunks
    for (int i = 0; i < (canvasWidth * canvasHeight / 8); i += 128)
    {
        TPChar->writeValue(&imageData[i], min((size_t)128, (size_t)((canvasWidth * canvasHeight / 8) - i)));
        delay(1);
    }
    // Cleanup
    free(imageData);
    delete textCanvas;
    getLineSpace();

    if (tplinespace != 8) // adds a line feed to match linespace
    {
        printText("\n"); // prints a line space after text if linespace is !default.
    }
}

void AbleTP::printQRCode(const char *qrData, uint8_t qrModel, uint8_t qrSize, uint8_t qrErrorLevel)
{
    if (qrModel < 1 || qrModel > 2)
    {
        Serial.println("qrModel must be either 1 (Model 1) or 2 (Model 2).");
        return;
    }
    // Validate qrSize
    if (qrSize < 1 || qrSize > 16)
    {
        Serial.println("qrSize must be in range: 1 to 16.");
        return;
    }
    // Validate qrErrorLevel
    if (qrErrorLevel > 3)
    {
        Serial.println("qrErrorLevel must be between (0 = Low, 1 = Medium, 2 = Quartile, 3 = High).");
        return;
    }
    // Check if qrData is not empty
    if (strlen(qrData) == 0)
    {
        Serial.println("qrData cannot be empty.");
        return;
    }
    // Calculate the data length for the QR Code command
    uint16_t dataLen = strlen(qrData) + 3; // +3 for the bytes 0x31, 0x50, 0x30 in the storeDataCmd

    // Set QR Code model
    uint8_t modelCmd[] = {0x1D, 0x28, 0x6B, 0x04, 0x00, 0x31, 0x41, qrModel, 0x00};
    sendCommand(modelCmd, sizeof(modelCmd));

    // Set QR Code size
    uint8_t sizeCmd[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x43, qrSize};
    sendCommand(sizeCmd, sizeof(sizeCmd));

    // Set QR Code error correction level
    uint8_t errorLevelCmd[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x45, qrErrorLevel};
    sendCommand(errorLevelCmd, sizeof(errorLevelCmd));

    // Store QR Code data
    uint8_t storeDataCmd[] = {0x1D, 0x28, 0x6B, (uint8_t)(dataLen & 0xFF), (uint8_t)((dataLen >> 8) & 0xFF), 0x31, 0x50, 0x30};
    sendCommand(storeDataCmd, sizeof(storeDataCmd));
    sendCommand((const uint8_t *)qrData, strlen(qrData));

    // Print QR Code with \n before and after to preserve whitespace.
    sendCommand((const uint8_t[]){0x0A}, 1); // \n (Newline)
    uint8_t printCmd[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x51, 0x30};
    sendCommand(printCmd, sizeof(printCmd));
    sendCommand((const uint8_t[]){0x0A}, 1); // \n (Newline)
}

void AbleTP::printUPCA(const char *data, uint8_t width, uint8_t height, uint8_t hriFont, uint8_t hriPosition)
{
    // Command to set the barcode width
    uint8_t widthCmd[] = {0x1D, 0x77, width};
    sendCommand(widthCmd, sizeof(widthCmd));

    // Command to set the barcode height
    uint8_t heightCmd[] = {0x1D, 0x68, height};
    sendCommand(heightCmd, sizeof(heightCmd));

    uint8_t hriFontCmd[] = {0x1D, 0x66, hriFont};
    sendCommand(hriFontCmd, sizeof(hriFontCmd));

    // Select font information for Human Readable Interpretation (HRI) characters
    uint8_t hriPositionCmd[] = {0x1D, 0x48, hriPosition};
    sendCommand(hriPositionCmd, sizeof(hriPositionCmd));

    // Command to print the barcode
    uint8_t printCmd[] = {0x1D, 0x6B, 0x00}; // UPC-A barcode command, adjusted length for command style
    sendCommand(printCmd, sizeof(printCmd));
    sendCommand((const uint8_t *)data, strlen(data)); // Send the UPC-A barcode data

    uint8_t terminator[] = {0x00};
    sendCommand(terminator, 1);
}

void AbleTP::printEAN13(const char *data, uint8_t width, uint8_t height, uint8_t hriFont, uint8_t hriPosition)
{
    // Set barcode width
    uint8_t widthCmd[] = {0x1D, 0x77, width};
    sendCommand(widthCmd, sizeof(widthCmd));

    // Set barcode height
    uint8_t heightCmd[] = {0x1D, 0x68, height};
    sendCommand(heightCmd, sizeof(heightCmd));

    uint8_t hriFontCmd[] = {0x1D, 0x66, hriFont}; // HRI font: 0 = font A, 1 = font B
    sendCommand(hriFontCmd, sizeof(hriFontCmd));

    // Set HRI character position
    uint8_t hriPosCmd[] = {0x1D, 0x48, hriPosition}; // HRI position: 0 = none, 1 = above, 2 = below
    sendCommand(hriPosCmd, sizeof(hriPosCmd));

    // Print EAN-13 barcode
    uint8_t printCmd[] = {0x1D, 0x6B, 2}; // 0x02 for EAN13
    sendCommand(printCmd, sizeof(printCmd));
    sendCommand((uint8_t *)data, strlen(data));

    // Terminate with a null byte to signify the end of data
    uint8_t terminator = 0x00;
    sendCommand(&terminator, 1);
}

void AbleTP::printCODE128(const char *data, uint8_t width, uint8_t height, uint8_t hriFont, uint8_t hriPosition)
{
    uint16_t dataLen = strlen(data);

    // Set barcode width
    uint8_t widthCmd[] = {0x1D, 0x77, width};
    sendCommand(widthCmd, sizeof(widthCmd));

    // Set barcode height
    uint8_t heightCmd[] = {0x1D, 0x68, height};
    sendCommand(heightCmd, sizeof(heightCmd));

    uint8_t hriFontCmd[] = {0x1D, 0x66, hriFont}; // HRI font: 0 = font A, 1 = font B
    sendCommand(hriFontCmd, sizeof(hriFontCmd));

    // Set HRI character position
    uint8_t hriPosCmd[] = {0x1D, 0x48, hriPosition}; // HRI position: 0 = none, 1 = above, 2 = below
    sendCommand(hriPosCmd, sizeof(hriPosCmd));

    // Print barcode
    uint8_t printCmd[] = {0x1D, 0x6B, 0x49, (uint8_t)strlen(data)};
    sendCommand(printCmd, sizeof(printCmd));
    sendCommand((uint8_t *)data, strlen(data));
}

void AbleTP::printCODE39(const char *data, uint8_t width, uint8_t height, uint8_t hriFont, uint8_t hriPosition)
{

    // Set barcode width
    uint8_t widthCmd[] = {0x1D, 0x77, width};
    sendCommand(widthCmd, sizeof(widthCmd));

    // Set barcode height
    uint8_t heightCmd[] = {0x1D, 0x68, height};
    sendCommand(heightCmd, sizeof(heightCmd));

    // Set HRI font
    uint8_t hriFontCmd[] = {0x1D, 0x66, hriFont};
    sendCommand(hriFontCmd, sizeof(hriFontCmd));

    // Set HRI position
    uint8_t hriPositionCmd[] = {0x1D, 0x48, hriPosition};
    sendCommand(hriPositionCmd, sizeof(hriPositionCmd));

    uint8_t barcodeDataLength = strlen(data);

    uint8_t barcodeTypeCmd[] = {0x1D, 0x6B, 69, barcodeDataLength}; // CODE39 command
    sendCommand(barcodeTypeCmd, sizeof(barcodeTypeCmd));

    // Send the formatted barcode data
    sendCommand((const uint8_t *)data, barcodeDataLength);
}

void AbleTP::canvasSetSize(int width, int height)
{
    if (width % 8 != 0)
    {
        Serial.println("ERROR: Width must be divisible by 8!");
        return;
    }
    if (gfx)
    {
        delete gfx;
    }
    gfx = new GFXcanvas1(width, height);
    if (!gfx)
    {
        Serial.println("ERROR: Memory allocation failed for GFX canvas!");
        return;
    }
    if (imageData)
    {
        free(imageData);
    }
    imageData = (uint8_t *)malloc((width * height) / 8);
    if (!imageData)
    {
        Serial.println("ERROR: Memory allocation failed for image data!");
        return;
    }
    memset(imageData, 0, (width * height) / 8);
    canvasWidth = width; // Ensure these are updated
    canvasHeight = height;
}

void AbleTP::PrepareImageData()
{
    if (!gfx)
    {
        Serial.println("Canvas not initialized!");
        return;
    }
    if (imageData)
    {
        free(imageData); // Free old image data if it exists
    }
    imageData = (uint8_t *)malloc((canvasWidth * canvasHeight) / 8);
    if (!imageData)
    {
        Serial.println("ERROR: Memory allocation failed for image data!");
        return;
    }
    memset(imageData, 0, (canvasWidth * canvasHeight) / 8);

    for (int y = 0; y < canvasHeight; y++)
    {
        for (int x = 0; x < canvasWidth / 8; x++)
        {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++)
            {
                int pixel = gfx->getPixel(x * 8 + bit, y);
                if (pixel > 0)
                {
                    byte |= (1 << (7 - bit));
                }
            }
            imageData[y * (canvasWidth / 8) + x] = byte;
        }
    }
}

void AbleTP::printCanvas()
{
    if (!gfx)
    {
        Serial.println("ERROR: Canvas not initialized!");
        return;
    }
    if (!imageData)
    {
        Serial.println("ERROR: No image data!");
        return;
    }
    uint8_t gs_v_cmd[] = {
        0x1D, 0x76, 0x30, 0x00,
        (uint8_t)(canvasWidth / 8), 0,
        (uint8_t)(canvasHeight & 0xFF),
        (uint8_t)((canvasHeight >> 8) & 0xFF)};
    TPChar->writeValue(gs_v_cmd, sizeof(gs_v_cmd));

    for (int i = 0; i < (canvasWidth * canvasHeight / 8); i += 128)
    {
        TPChar->writeValue(&imageData[i], min((size_t)128, (size_t)((canvasWidth * canvasHeight / 8) - i)));
        delay(1);
    }
}

void AbleTP::canvasDrawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (gfx)
    {
        gfx->drawPixel(x, y, color);
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    if (gfx)
    {
        gfx->writeLine(x0, y0, x1, y1, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasDrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    if (gfx)
    {
        gfx->writeFastHLine(x, y, w, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasDrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    if (gfx)
    {
        gfx->writeFastVLine(x, y, h, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (gfx)
    {
        gfx->drawRect(x, y, w, h, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (gfx)
    {
        gfx->fillRect(x, y, w, h, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasDrawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
    if (gfx)
    {
        gfx->drawCircle(x, y, r, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasFillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
    if (gfx)
    {
        gfx->fillCircle(x, y, r, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasDrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    if (gfx)
    {
        gfx->drawTriangle(x0, y0, x1, y1, x2, y2, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasFillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    if (gfx)
    {
        gfx->fillTriangle(x0, y0, x1, y1, x2, y2, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasDrawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
    if (gfx)
    {
        gfx->drawRoundRect(x0, y0, w, h, radius, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasFillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
    if (gfx)
    {
        gfx->fillRoundRect(x0, y0, w, h, radius, color);
        delay(1);           // Delay to prevent buffer overflow
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasPrintImage(const uint8_t *image, uint16_t width, uint16_t height, int16_t startX, int16_t startY, bool invert = false)
{
    if (!gfx)
    {
        Serial.println("Canvas not initialized!");
        return;
    }
    if (!image)
    {
        Serial.println("ERROR: Image is null!");
        return;
    }
    if (width % 8 != 0)
    {
        Serial.println("ERROR: Image width must be divisible by 8!");
        return;
    }

    uint16_t x, y;
    uint8_t byte, bit;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x += 8)
        {
            byte = image[y * (width / 8) + (x / 8)];
            for (bit = 0; bit < 8; bit++)
            {
                // Determine color based on the invert flag
                int color = (byte & (1 << (7 - bit))) ? (invert ? 0 : 1) : (invert ? 1 : 0);
                gfx->drawPixel(startX + x + bit, startY + y, color);
            }
        }
    }
    PrepareImageData();
}

void AbleTP::canvasSetTextScale(uint8_t sx, uint8_t sy)
{
    if (gfx)
    {
        gfx->setTextSize(sx, sy);
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasSetFont(const GFXfont *f = NULL)
{
    if (gfx)
    {
        gfx->setFont(f);
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasSetCursor(int16_t x, int16_t y)
{
    if (gfx)
    {
        gfx->setCursor(x, y);
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasSetTextColor(uint16_t c)
{
    if (gfx)
    {
        gfx->setTextColor(c);
        PrepareImageData(); // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasPrintText(const char *text)
{
    if (!gfx)
    {
        Serial.println("Canvas not initialized!");
        return;
    }
    if (text == nullptr || strlen(text) == 0)
    {
        Serial.println("ERROR: Empty text!");
        return;
    }
    int16_t x, y;
    x = gfx->getCursorX();
    y = gfx->getCursorY();

    int16_t x1, y1; // Adjusted base x, y coordinates
    uint16_t tw, th;

    gfx->getTextBounds(text, 0, 0, &x1, &y1, &tw, &th); // Get text bounds relative to (0,0)

    int16_t xold = x;             // Text width and height
    int16_t yold = y;             // Text width and height
    mytextdatum = getTextDatum(); // Get the current datum setting

    // Adjust starting position based on the current datum
    switch (mytextdatum)
    {
    case T_L: // Top Left
        y += th;
        break;
    case T_C: // Top Center
        x -= tw / 2;
        y += th;
        break;
    case T_R: // Top Right
        x -= tw;
        y += th;
        break;
    case M_L: // Middle Left
        y += th / 2;
        break;
    case M_C: // Middle Center
        x -= tw / 2;
        y += th / 2;
        break;
    case M_R: // Middle Right
        x -= tw;
        y += th / 2;
        break;
    case B_L: // Bottom Left
        break;
    case B_C: // Bottom Center
        x -= tw / 2;
        break;
    case B_R: // Bottom Right
        x -= tw;
        break;
    default:
        mytextdatum = T_L; // Reset to default if out of expected range
        break;
    }   
    canvasSetCursor(x, y);
    gfx->println(text);
    delay(1);                    // Delay to prevent buffer overflow
    PrepareImageData();          // Update imageData immediately after drawing
    canvasSetCursor(xold, yold); // reset to original cursor position
    x = gfx->getCursorX();
    y = gfx->getCursorY();    
}

void AbleTP::canvasSetTextWrap(bool w)
{
    if (gfx)
    {
        gfx->setTextWrap(w); // Correct method to set text wrapping
        PrepareImageData();  // Update imageData immediately after drawing
    }
    else
    {
        Serial.println("Canvas Size not set!.");
    }
}

void AbleTP::canvasTextDatum(uint8_t datum)
{
    mytextdatum = datum; // Store the current datum setting
}

uint8_t AbleTP::getTextDatum()
{
    return mytextdatum;
}
