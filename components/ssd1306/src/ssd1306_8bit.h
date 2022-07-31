/*
    MIT License

    Copyright (c) 2018, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
/**
 * @file ssd1306_8bit.h 8-bit specific draw functions
 */

#ifndef _SSD1306_8BIT_H_
#define _SSD1306_8BIT_H_

#include "nano_gfx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
//                 DIRECT GRAPH FUNCTIONS
///////////////////////////////////////////////////////////////////////

/**
 * @defgroup LCD_8BIT_GRAPHICS DIRECT DRAW: 8-bit API functions only for color displays
 * @{
 *
 * @brief LCD direct draw functions only for color display.
 *
 * @details LCD direct draw functions are applicable for color display types. These functions will NOT work
 *        in ssd1306 compatible mode. Use ssd1306_setMode() function to change display mode to NORMAL.
 *        You can combine combine NanoEngine capabilities with these functions.
 *        Direct draw functions draw directly in GDRAM and do not use any double-buffering.
 */

/**
 * @brief Sets default color, generated by RGB_COLOR8 or RGB_COLOR16 macros
 *
 * Sets color generated by RGB_COLOR8 or RGB_COLOR16 macros.
 *
 * @param color - new  color to use for monochrome-specific operations on color display.
 */
void ssd1306_setColor(uint16_t color);

/**
 * @brief Sets default color.
 *
 * Sets default color for monochrome operations.
 * For now this function supports only 8-bit RGB mode.
 * To work with RGB colors in 16-bit mode, please refer to ssd1306_setColor() function
 * and RGB_COLOR16 macros.
 * @param r - red in 0-255 range.
 * @param g - green in 0-255 range.
 * @param b - blue in 0-255 range.
 */
void ssd1306_setRgbColor(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Sets default color.
 *
 * Sets default color for monochrome operations.
 * For now this function supports only 8-bit RGB mode.
 * To work with RGB colors in 16-bit mode, please refer to ssd1306_setColor() function
 * and RGB_COLOR16 macros.
 * @param r - red in 0-255 range.
 * @param g - green in 0-255 range.
 * @param b - blue in 0-255 range.
 */
void ssd1306_setRgbColor8(uint8_t r, uint8_t g, uint8_t b);

/**
 * Draws 1-bit bitmap, located in SRAM, on the display
 * Each bit represents separate pixel: refer to ssd1306 datasheet for more information.
 *
 * @param xpos horizontal position in pixels
 * @param ypos vertical position in pixels
 * @param w width of bitmap in pixels
 * @param h height of bitmap in pixels
 * @param bitmap pointer to data, located in SRAM.
 */
void ssd1306_drawMonoBuffer8(lcdint_t xpos, lcdint_t ypos, lcduint_t w, lcduint_t h, const uint8_t *bitmap);

/**
 * Draws 8-bit bitmap, located in SRAM, on the display
 * Each byte represents separate pixel: refer to RGB_COLOR8 to understand RGB scheme, being used.
 *
 * @param x - horizontal position in pixels
 * @param y - vertical position in pixels
 * @param w - width of bitmap in pixels
 * @param h - height of bitmap in pixels
 * @param data - pointer to data, located in SRAM.
 */
void ssd1306_drawBufferFast8(lcdint_t x, lcdint_t y, lcduint_t w, lcduint_t h, const uint8_t *data);

/**
 * Draws 8-bit bitmap, located in SRAM, on the display, taking into account pitch parameter.
 * Each byte represents separate pixel: refer to RGB_COLOR8 to understand RGB scheme, being used.
 * pitch parameter specifies, length of single line in bytes.
 *
 * @param x - horizontal position in pixels
 * @param y - vertical position in pixels
 * @param w - width of bitmap in pixels
 * @param h - height of bitmap in pixels
 * @param pitch length of bitmap buffer line in bytes
 * @param data - pointer to data, located in SRAM.
 */
void ssd1306_drawBufferEx8(lcdint_t x, lcdint_t y, lcduint_t w, lcduint_t h, lcduint_t pitch, const uint8_t *data);

/**
 * Fills screen with pattern byte
 *
 * @param fill_Data pattern color to fill screen with
 */
void ssd1306_fillScreen8(uint8_t fill_Data);

/**
 * Fills screen with zero-byte
 */
void ssd1306_clearScreen8(void);

/**
 * Puts single color point directly in OLED display GDRAM.
 *
 * @param x - horizontal position in pixels
 * @param y - vertical position in pixels
 *
 * @note set color with ssd1306_setColor() function.
 */
void ssd1306_putPixel8(lcdint_t x, lcdint_t y);

/**
 * Puts single color point directly in OLED display GDRAM.
 *
 * @param x - horizontal position in pixels
 * @param y - vertical position in pixels
 * @param color color in RGB8 format (2-3-2)
 */
void ssd1306_putColorPixel8(lcdint_t x, lcdint_t y, uint8_t color);

/**
 * Draw vertical line directly in OLED display GDRAM.
 *
 * @param x1 - horizontal position in pixels
 * @param y1 - top vertical position in pixels
 * @param y2 - bottom vertical position in pixels
 *
 * @note set color with ssd1306_setColor() function.
 */
void ssd1306_drawVLine8(lcdint_t x1, lcdint_t y1, lcdint_t y2);

/**
 * Draw horizontal line directly in OLED display GDRAM.
 *
 * @param x1 - left position in pixels
 * @param y1 - vertical vertical position in pixels
 * @param x2 - right position in pixels
 *
 * @note set color with ssd1306_setColor() function.
 */
void ssd1306_drawHLine8(lcdint_t x1, lcdint_t y1, lcdint_t x2);

/**
 * Draw line directly in OLED display GDRAM.
 * This is software implementation. Some OLED controllers have hardware implementation.
 * Refer to datasheet.
 *
 * @param x1 - start horizontal position in pixels
 * @param y1 - start vertical position in pixels
 * @param x2 - end horizontal position in pixels
 * @param y2 - end vertical position in pixels
 *
 * @note set color with ssd1306_setColor() function.
 */
void ssd1306_drawLine8(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2);

/**
 * Draw rectangle directly in OLED display GDRAM.
 * This is software implementation. Some OLED controllers have hardware implementation.
 * Refer to datasheet.
 *
 * @param x1 - start horizontal position in pixels
 * @param y1 - start vertical position in pixels
 * @param x2 - end horizontal position in pixels
 * @param y2 - end vertical position in pixels
 *
 * @note set color with ssd1306_setColor() function.
 */
void ssd1306_drawRect8(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2);

/**
 * Fill rectangle directly in OLED display GDRAM.
 * This is software implementation. Some OLED controllers have hardware implementation.
 * Refer to datasheet.
 *
 * @param x1 - start horizontal position in pixels
 * @param y1 - start vertical position in pixels
 * @param x2 - end horizontal position in pixels
 * @param y2 - end vertical position in pixels
 *
 * @note set color with ssd1306_setColor() function.
 */
void ssd1306_fillRect8(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2);

/**
 * Draw monochrome bitmap, located in Flash, directly to OLED display GDRAM.
 * The bitmap should be in ssd1306 format (each byte represents 8 vertical pixels)
 *
 * @param xpos start horizontal position in pixels
 * @param ypos start vertical position in pixels
 * @param w bitmap width in pixels
 * @param h bitmap height in pixels
 * @param bitmap pointer to Flash data, containing monochrome bitmap.
 *
 * @note set color with ssd1306_setColor() function.
 */
void ssd1306_drawMonoBitmap8(lcdint_t xpos, lcdint_t ypos, lcduint_t w, lcduint_t h, const uint8_t *bitmap);

/**
 * Draw 8-bit color bitmap, located in Flash, directly to OLED display GDRAM.
 * Each pixel of the bitmap is expected in 3-3-2 format.
 *
 * @param xpos start horizontal position in pixels
 * @param ypos start vertical position in pixels
 * @param w bitmap width in pixels
 * @param h bitmap height in pixels
 * @param bitmap pointer to Flash data, containing 8-bit color bitmap.
 */
void ssd1306_drawBitmap8(lcdint_t xpos, lcdint_t ypos, lcduint_t w, lcduint_t h, const uint8_t *bitmap);

/**
 * Clears block, filling it with black pixels, directly in OLED display GDRAM.
 *
 * @param x start horizontal position in pixels
 * @param y start vertical position in pixels
 * @param w block width in pixels
 * @param h block height in pixels
 */
void ssd1306_clearBlock8(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

/**
 * Set cursor position for text functions
 *
 * @param x horizontal position in pixels.
 * @param y vertical position in pixels.
 */
void ssd1306_setCursor8(lcduint_t x, lcduint_t y);

/**
 * Draws single character to canvas. Cursor position is defined
 * by ssd1306_setCursor8(). Do not changes cursor position
 *
 * @param c - character code to print
 *
 * @note set color with ssd1306_setColor() function.
 */
void ssd1306_printChar8(uint8_t c);

/**
 * @brief Prints single character to display at current cursor position
 *
 * Prints single character to display at current cursor position.
 * Cursor position can be set by ssd1306_setCursor8().
 *
 * @param ch - character to print to the display. 'LF' and 'CR' are skipped
 * @return returns number of printed characters.
 */
size_t ssd1306_write8(uint8_t ch);

/**
 * @brief Prints null-terminated string to display at current cursor position
 *
 * Prints null-terminated string to display at current cursor position
 * Cursor position can be set by ssd1306_setCursor8().
 *
 * @param ch - string to print to the display. 'LF' and 'CR' are skipped
 * @return returns number of printed characters.
 */
size_t ssd1306_print8(const char ch[]);

/**
 * Prints text to screen using fixed font.
 * @param x horizontal position in pixels
 * @param y vertical position in pixels
 * @param ch NULL-terminated string to print
 * @param style font style (EFontStyle), normal by default (not implemented).
 * @returns number of chars in string
 *
 * @see ssd1306_setFixedFont
 * @note set color with ssd1306_setColor() function.
 */
uint8_t ssd1306_printFixed8(lcdint_t x, lcdint_t y, const char *ch, EFontStyle style);


#ifndef DOXYGEN_SHOULD_SKIP_THIS

/////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS BELOW ARE ONLY FOR COMPATIBILITY WITH PREVIOUSE VERSIONS OF LIBRARY

static inline void ssd1331_setColor(uint16_t color)
{
    ssd1306_setColor(color);
}

static inline void ssd1331_setRgbColor(uint8_t r, uint8_t g, uint8_t b)
{
    ssd1306_setRgbColor(r, g, b);
}

static inline void ssd1331_drawMonoBuffer8(lcdint_t xpos, lcdint_t ypos, lcduint_t w, lcduint_t h, const uint8_t *bitmap)
{
    ssd1306_drawMonoBuffer8(xpos, ypos, w, h, bitmap);
}

static inline void ssd1331_drawBufferFast8(lcdint_t x, lcdint_t y, lcduint_t w, lcduint_t h, const uint8_t *data)
{
    ssd1306_drawBufferFast8(x, y, w, h, data);
}

static inline void ssd1331_fillScreen8(uint8_t fill_Data)
{
    ssd1306_fillScreen8(fill_Data);
}

static inline void ssd1331_clearScreen8(void)
{
    ssd1306_clearScreen8();
}

static inline void ssd1331_putPixel8(lcdint_t x, lcdint_t y)
{
    ssd1306_putPixel8(x, y);
}

static inline void ssd1331_drawVLine8(lcdint_t x1, lcdint_t y1, lcdint_t y2)
{
    ssd1306_drawVLine8(x1, y1, y2);
}

static inline void ssd1331_drawHLine8(lcdint_t x1, lcdint_t y1, lcdint_t x2)
{
    ssd1306_drawHLine8(x1, y1, x2);
}

static inline void ssd1331_drawLine8(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2)
{
    ssd1306_drawLine8(x1, y1, x2, y2);
}

static inline void ssd1331_drawRect8(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2)
{
    ssd1306_drawRect8(x1, y1, x2, y2);
}

static inline void ssd1331_fillRect8(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2)
{
    ssd1306_fillRect8(x1, y1, x2, y2);
}

static inline void ssd1331_drawMonoBitmap8(lcdint_t xpos, lcdint_t ypos, lcduint_t w, lcduint_t h, const uint8_t *bitmap)
{
    ssd1306_drawMonoBitmap8(xpos, ypos, w, h, bitmap);
}

static inline void ssd1331_drawBitmap8(lcdint_t xpos, lcdint_t ypos, lcduint_t w, lcduint_t h, const uint8_t *bitmap)
{
    ssd1306_drawBitmap8(xpos, ypos, w, h, bitmap);
}

static inline void ssd1331_clearBlock8(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    ssd1306_clearBlock8(x, y, w, h);
}

static inline void ssd1331_setCursor8(lcduint_t x, lcduint_t y)
{
    ssd1306_setCursor8(x, y);
}

static inline void ssd1331_printChar8(uint8_t c)
{
    ssd1306_printChar8(c);
}

static inline size_t ssd1331_write8(uint8_t ch)
{
    return ssd1306_write8(ch);
}

static inline size_t ssd1331_print8(const char ch[])
{
    return ssd1306_print8(ch);
}

static inline uint8_t ssd1331_printFixed8(lcdint_t x, lcdint_t y, const char *ch, EFontStyle style)
{
    return ssd1306_printFixed8(x, y, ch, style);
}

#endif

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // _SSD1306_8BIT_H_
