/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#include "u8g2_drawables.hpp"

void U8g2Drawables::DrawHLine(u8g2_uint_t x, u8g2_int_t y, u8g2_int_t x_offset, uint8_t thickness)
{
    if(thickness == 0) {
        return;
    }

    // Handle negative y coordinate (offset from bottom edge)
    u8g2_uint_t actual_y = y;
    if(y < 0) {
        u8g2_uint_t display_height = u8g2_GetDisplayHeight(u8g2_);
        actual_y = display_height + y;
    }

    // Handle x_offset
    u8g2_uint_t len;
    if(x_offset >= 0) {
        // Positive offset: draw from x to (x + offset)
        len = x_offset;
    }
    else {
        // Negative offset: draw from x to (width + offset), so length = width - x + offset
        u8g2_uint_t display_width = u8g2_GetDisplayWidth(u8g2_);
        len = display_width - x + x_offset;
    }

    // Calculate start offset to center the line around y
    int8_t start_offset = -(thickness / 2);

    for(uint8_t i = 0; i < thickness; i++) {
        u8g2_DrawHLine(u8g2_, x, actual_y + start_offset + i, len);
    }
}

void U8g2Drawables::DrawVLine(u8g2_int_t x, u8g2_uint_t y, u8g2_int_t y_offset, uint8_t thickness)
{
    if(thickness == 0) {
        return;
    }

    // Handle negative x coordinate (offset from right edge)
    u8g2_uint_t actual_x = x;
    if(x < 0) {
        u8g2_uint_t display_width = u8g2_GetDisplayWidth(u8g2_);
        actual_x = display_width + x;
    }

    // Handle y_offset
    u8g2_uint_t len;
    if(y_offset >= 0) {
        // Positive offset: draw from y to (y + offset)
        len = y_offset;
    }
    else {
        // Negative offset: draw from y to (height + offset), so length = height - y + offset
        u8g2_uint_t display_height = u8g2_GetDisplayHeight(u8g2_);
        len = display_height - y + y_offset;
    }

    // Calculate start offset to center the line around x
    int8_t start_offset = -(thickness / 2);

    for(uint8_t i = 0; i < thickness; i++) {
        u8g2_DrawVLine(u8g2_, actual_x + start_offset + i, y, len);
    }
}

void U8g2Drawables::DrawFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
{
    u8g2_DrawFrame(u8g2_, x, y, w, h);
}

void U8g2Drawables::DrawRFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r)
{
    u8g2_DrawRFrame(u8g2_, x, y, w, h, r);
}
