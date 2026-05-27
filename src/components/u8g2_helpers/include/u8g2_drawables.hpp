/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#pragma once

#include "u8g2.h"
#include <esp_log.h>

/**
 * @brief C++ wrapper for u8g2 drawing functions with enhanced features
 */
class U8g2Drawables
{
private:
    u8g2_t* u8g2_;
    static constexpr char TAG[] = "drawables";
public:
    /**
     * @brief Constructor
     * @param u8g2 Pointer to u8g2_t structure
     */
    explicit U8g2Drawables(u8g2_t* u8g2) : u8g2_(u8g2)
    { }

    /**
     * @brief Draw a horizontal line with thickness using offset coordinate
     * @param x X-coordinate of the starting point
     * @param y Y-coordinate of the center of the line (negative means offset from bottom edge)
     * @param offset Offset from canvas edge if negative and length if positive (negative means offset from right edge)
     * @param thickness Line thickness in pixels (centered around y)
     */
    void DrawHLine(u8g2_uint_t x, u8g2_int_t y, u8g2_int_t offset, uint8_t thickness = 1);

    /**
     * @brief Draw a horizontal line with thickness using offset coordinate
     * @param x X-coordinate offset from edges
     * @param y Y-coordinate of the center of the line (negative means offset from bottom edge)
     * @param thickness Line thickness in pixels (centered around y)
     */
    void DrawHLineCentered(u8g2_uint_t x, u8g2_int_t y, uint8_t thickness = 1)
    {
        DrawHLine(x, y, -x, thickness);
    }

    /**
     * @brief Draw a vertical line with thickness using offset coordinate
     * @param x X-coordinate of the center of the line (negative means offset from right edge)
     * @param y Y-coordinate of the starting point
     * @param offset Offset from canvas edge if negative and length if positive (negative means offset from bottom edge)
     * @param thickness Line thickness in pixels (centered around x)
     */
    void DrawVLine(u8g2_int_t x, u8g2_uint_t y, u8g2_int_t offset, uint8_t thickness = 1);

    /**
     * @brief Draw a vertical line with thickness using offset coordinate
     * @param x X-coordinate of the center of the line (negative means offset from right edge)
     * @param y Y-coordinate offset from edges
     * @param thickness Line thickness in pixels (centered around x)
     */
    void DrawVLineCentered(u8g2_int_t x, u8g2_uint_t y, uint8_t thickness = 1)
    {
        DrawVLine(x, y, -y, thickness);
    }

    /**
     * @brief Draw a frame (rectangle outline)
     * @param x X-coordinate of the top-left corner
     * @param y Y-coordinate of the top-left corner
     * @param w Width of the frame
     * @param h Height of the frame
     */
    void DrawFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h);

    /**
     * @brief Draw a rounded frame (rectangle outline with rounded corners)
     * @param x X-coordinate of the top-left corner
     * @param y Y-coordinate of the top-left corner
     * @param w Width of the frame
     * @param h Height of the frame
     * @param r Radius of the rounded corners
     */
    void DrawRFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r);

    /**
     * @brief Get the width of the display
     * @return Width of the display in pixels
     */
    u8g2_uint_t GetDisplayWidth() const
    {
        return u8g2_GetDisplayWidth(u8g2_);
    }

    /**
     * @brief Get the height of the display
     * @return Height of the display in pixels
     */
    u8g2_uint_t GetDisplayHeight() const
    {
        return u8g2_GetDisplayHeight(u8g2_);
    }

    /**
     * @brief Draw a string at a specific position
     * @param x X-coordinate for the text (negative means offset from right edge)
     * @param y Y-coordinate for the text (negative means offset from bottom edge)
     * @param text String to draw
     */
    void DrawStr(u8g2_int_t x, u8g2_int_t y, const char* text)
    {
        u8g2_uint_t actual_x = x >= 0 ? x : GetDisplayWidth() + x;
        u8g2_uint_t actual_y = y >= 0 ? y : GetDisplayHeight() + y;
        // ESP_LOGI(TAG, "Drawing text at (%u, %u): %s", actual_x, actual_y, text);
        u8g2_DrawStr(u8g2_, actual_x, actual_y, text);
    }

    /**
     * @brief Draw a string at a specific position
     * @param x X-coordinate for the text (negative means offset from right edge)
     * @param y Y-coordinate for the text (negative means offset from bottom edge)
     * @param text String to draw
     */
    void DrawGlyph(u8g2_int_t x, u8g2_int_t y, uint16_t glyph)
    {
        u8g2_uint_t actual_x = x >= 0 ? x : GetDisplayWidth() + x;
        u8g2_uint_t actual_y = y >= 0 ? y : GetDisplayHeight() + y;
        // ESP_LOGI(TAG, "Drawing glyph at (%u, %u): %u", actual_x, actual_y, glyph);
        u8g2_DrawGlyph(u8g2_, actual_x, actual_y, glyph);
    }

    /**
     * @brief Draw a string centered horizontally on the display
     * @param x X-coordinate for the text (negative means offset from right edge)
     * @param y Y-coordinate for the text (negative means offset from bottom edge)
     * @param text String to draw
     */
    void DrawCenteredStr(u8g2_int_t x, u8g2_int_t y, const char* text);

    /**
     * @brief Draw a UTF-8 string centered horizontally on the display
     * @param x X-coordinate for the text (negative means offset from right edge)
     * @param y Y-coordinate for the text (negative means offset from bottom edge)
     * @param text String to draw
     */
    void DrawCenteredStrUtf8(u8g2_int_t x, u8g2_int_t y, const char* text);

    /**
     * @brief Set the font for drawing text
     * @param font Font to use
     */
    void SetFont(const uint8_t* font)
    {
        u8g2_SetFont(u8g2_, font);
    }

    void DrawUTF8(u8g2_int_t x, u8g2_int_t y, const char* text)
    {
        u8g2_uint_t actual_x = x >= 0 ? x : GetDisplayWidth() + x;
        u8g2_uint_t actual_y = y >= 0 ? y : GetDisplayHeight() + y;
        u8g2_DrawUTF8(u8g2_, actual_x, actual_y, text);
    }
};
