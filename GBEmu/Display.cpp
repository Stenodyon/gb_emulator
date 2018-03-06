/*  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "stdafx.h"
#include "Display.h"
#include "CPU.h"

#include <cmath>
#include <algorithm>

static std::chrono::time_point<std::chrono::system_clock> last_refresh;
static bool last_lcd_display_enable = false;
static uint8_t last_control;

void Display::OnMachineCycle(uint64_t cycles)
{
    lastElapsedTime = elapsedTime;
    elapsedTime += cycles * 4 * 0.238418579;

    for (uint64_t cycle = 0; cycle < cycles * 4; cycle++)
        incrementFrameCycles();
}

void Display::incrementFrameCycles()
{
    //TODO: Implement LY being reset
    if (frameCycles >= 70224)
        frameCycles = 0;
    else
        frameCycles++;

    ly = (uint8_t)(frameCycles / 456);

    status.ly_coincidence = ly == lyc;
    if (status.coincidence_int && status.ly_coincidence)
        cpu->interrupt(0x48);

    if (frameCycles == 456 * 144) // VBLANK
    {
        SDL_UpdateTexture(win_texture, NULL, (void*)pixel_buffer, 160 * sizeof(pixel));
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, win_texture, NULL, NULL);
        SDL_RenderPresent(renderer);

#if 0
        static uint64_t ticks = SDL_GetPerformanceCounter();
        uint64_t cur_ticks = SDL_GetPerformanceCounter();
        uint32_t delay = (uint32_t)std::min((double)(cur_ticks - ticks) / SDL_GetPerformanceFrequency(), 1000. / 60);
        SDL_Delay((1000 / 60) - delay);
        ticks = cur_ticks;
#endif
        update();
        cpu->updateInput();

        cpu->interrupt(0x40);
        if (status.vblank_int)
            cpu->interrupt(0x48);
    }

    if (frameCycles >= 456 * 144)
    {
        status.stat_mode = 1;
    }
    else
    {
        uint64_t progress = frameCycles % 456;

#if 0
        if (progress >= 80 && progress < 252)
        {
            static uint8_t last_column = 0;
            uint8_t current_column = (uint8_t)(((progress - 80) * 160) / 172);
            drawLine(ly, last_column, current_column + 1);
            last_column = current_column + 1;
        }
#endif

        if (progress == 0) // new line
        {
            if (status.oam_int)
                cpu->interrupt(0x48);
        }

        else if (progress == 252) // HBLANK
        {
            drawLine(ly);
            if (status.hblank_int)
                cpu->interrupt(0x48);
        }

        if (progress < 80)
            status.stat_mode = 2;
        else if (progress < 80 + 172)
            status.stat_mode = 3;
        else
            status.stat_mode = 0;
    }
}

const Display::pixel color_palette[4] = {
    Display::pixel{ 255, 208, 248, 224 },
    Display::pixel{ 255, 112, 192, 136 },
    Display::pixel{ 255, 80, 104, 48 },
    Display::pixel{ 255, 32, 24, 8 },
};

void Display::drawPixel(uint8_t x, uint8_t y, uint8_t color, bool transparency)
{
    uint64_t index = (uint64_t)x + 160 * ((uint64_t)y);
    pixel * pix = pixel_buffer + index;
    std::memcpy(pix, &(color_palette[color]), sizeof(pixel));
#if 0
    uint8_t _color = (3 - color) << 6;
    pix->a = 0xFF;
    pix->r = pix->g = pix->b = _color;
#endif
}

uint8_t Display::getBGColor(uint8_t x, uint8_t y)
{
    const uint8_t tileX = x / 8;
    const uint8_t tileY = y / 8;
    const uint16_t tileIndex = tileX + 32 * tileY;
    const uint16_t tileData = bg_tilemap_select ? 0x1C00 : 0x1800;
    const uint8_t tileCode = *(vram + tileData + tileIndex);
    tile * tile = getTile(tileCode);
    const uint8_t pixelX = x & 0x07;
    const uint8_t pixelY = y & 0x07;
    const uint8_t pixelIndex = pixelX + 8 * pixelY;
    const uint8_t colorCode = (*tile)[pixelIndex];
    return colorCode;
}

uint8_t Display::getWindowColor(uint8_t x, uint8_t y)
{
    const uint8_t tileX = x / 8;
    const uint8_t tileY = y / 8;
    const uint16_t tileIndex = tileX + 32 * tileY;
    const uint16_t tileData = win_tilemap_select ? 0x1C00 : 0x1800;
    const uint8_t tileCode = *(vram + tileData + tileIndex);
    tile * tile = getTile(tileCode);
    const uint8_t pixelX = x & 0x07;
    const uint8_t pixelY = y & 0x07;
    const uint8_t pixelIndex = pixelX + 8 * pixelY;
    const uint8_t colorCode = (*tile)[pixelIndex];
    const uint8_t color = bg_palette[colorCode];
    return color;
}

tile * Display::getTile(uint8_t index)
{
    if (bg_win_tile_data_select)
        return (tile*)vram + index;
    else
        return (tile*)(vram + 0x1000) + (int8_t)index;
}

uint8_t Display::getBGColorUnderPixel(uint8_t x, uint8_t y)
{
    return getBGColor(x + scrollX, y + scrollY);
}

uint8_t Display::getWinColorUnderPixel(uint8_t x, uint8_t y)
{
    return getWindowColor(x - (winPosX - 7), y - winPosY);
}

bool sprite_ptr_comp8(sprite * a, sprite * b)
{
    if (a->x + 8 <= b->x || b->x + 8 <= a->x)
        return false;
    if (a->y + 8 <= b->y || b->y + 8 <= a->y)
        return false;
    return a->x < b->x;
}

bool sprite_ptr_comp16(sprite * a, sprite * b)
{
    if (a->x + 8 <= b->x || b->x + 8 <= a->x)
        return false;
    if (a->y + 16 <= b->y || b->y + 16 <= a->y)
        return false;
    return a->x < b->x;
}

void Display::getVisibleSprites(uint8_t line, sprite * buffer[], uint8_t & count)
{
    sprite * visible_sprites[40];
    count = 0;
    sprite * oam_base = (sprite*)oam_ram;
    for (uint8_t index = 0; index < 40; index++)
    {
        sprite * sprite = oam_base + index;
        int16_t spriteY = (int16_t)sprite->y - 0x10;
        if (line >= spriteY
            && ((!obj_size && line < spriteY + 8)
                || (obj_size && line < spriteY + 16)))
        {
            visible_sprites[count] = sprite;
            count++;
        }
    }
    if (obj_size)
        std::sort<sprite**>(visible_sprites, visible_sprites + count, sprite_ptr_comp16);
    else
        std::sort<sprite**>(visible_sprites, visible_sprites + count, sprite_ptr_comp8);
    count = std::min(count, (uint8_t)10);
    std::memcpy(buffer, visible_sprites, count * sizeof(sprite*));
}

tile * Display::getSpriteTile(uint8_t index)
{
    return (tile*)vram + index;
}

uint8_t Display::getSpriteColor(sprite * sprite, uint8_t x, uint8_t y)
{
    int16_t spriteX = sprite->x - 0x08;
    int16_t spriteY = sprite->y - 0x10;
    if (x < spriteX || y < spriteY || x >= spriteX + 8)
        return 0xFF;
    if (!obj_size)
    {
        if (y >= spriteY + 8)
            return 0xFF;
        tile * _tile = getSpriteTile(sprite->chr_code);
        uint8_t relX = x - spriteX;
        uint8_t relY = y - spriteY;
        if (sprite->hor_flip)
            relX = 7 - relX;
        if (sprite->vert_flip)
            relY = 7 - relY;
        uint8_t colorCode = (*_tile)[relX + 8 * relY];
        return colorCode;
    }
    else
    {
        if (y >= spriteY + 16)
            return 0xFF;
        uint8_t relX = x - spriteX;
        uint8_t relY = y - spriteY;
        if (sprite->hor_flip)
            relX = 7 - relX;
        if (sprite->vert_flip)
            relY = 15 - relY;
        uint8_t char_code = relY < 8 ? sprite->chr_code & ~0x01 : sprite->chr_code | 0x01;
        tile * _tile = getSpriteTile(char_code);
        uint8_t colorCode = (*_tile)[relX + 8 * (relY % 8)];
        return colorCode;
    }
}

void Display::drawLine(uint8_t line)
{
    sprite * visible_sprites[10];
    uint8_t visible_count = 0;
    if (obj_dispay_enable)
    {
        getVisibleSprites(line, visible_sprites, visible_count);
    }
    for (uint8_t x = 0; x < 160; x++)
    {
        uint8_t bgcolor;
        if (bg_display || obj_dispay_enable)
        {
            bgcolor = getBGColorUnderPixel(x, line);
        }

        if (win_display_enable && line >= winPosY && (x >= (winPosX - 7)))
        {
            uint8_t color = getWinColorUnderPixel(x, line);
            drawPixel(x, line, color);
        }
        else if (bg_display)
        {
            uint8_t color = bg_palette[bgcolor];
            drawPixel(x, line, color);
        }
        if (obj_dispay_enable)
        {
            for (uint8_t sprite_index = 0; sprite_index < visible_count; sprite_index++)
            {
                sprite * sprite = visible_sprites[sprite_index];
                uint8_t colorCode = getSpriteColor(sprite, x, line);
                if (colorCode != 0xFF)
                {
                    if (colorCode != 0 && (!sprite->priority || bgcolor == 0))
                    {
                        palette * pal = sprite->palette ? &obj_palette1 : &obj_palette0;
                        uint8_t color = (*pal)[colorCode];
                        drawPixel(x, line, color, true);
                    break;
                    }
                }
            }
        }
    }
}

void Display::update()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_KEYUP:
        case SDL_KEYDOWN:
        {
            bool set = e.type == SDL_KEYDOWN;
            switch (e.key.keysym.sym)
            {
            case SDLK_j: keyboard_status.button_a = set; break;
            case SDLK_k: keyboard_status.button_b = set; break;
            case SDLK_u: keyboard_status.button_start = set; break;
            case SDLK_i: keyboard_status.button_select = set; break;
            case SDLK_w: keyboard_status.up = set; break;
            case SDLK_a: keyboard_status.left = set; break;
            case SDLK_s: keyboard_status.down = set; break;
            case SDLK_d: keyboard_status.right = set; break;
            }
            break;
        }
        case SDL_QUIT:
            cpu->running = false;
            break;
        }
    }
    cpu->joypad.joypad.button_a = keyboard_status.button_a | controller_status.button_a;
    cpu->joypad.joypad.button_b = keyboard_status.button_b | controller_status.button_b;
    cpu->joypad.joypad.button_start = keyboard_status.button_start | controller_status.button_start;
    cpu->joypad.joypad.button_select = keyboard_status.button_select | controller_status.button_select;
    cpu->joypad.joypad.up = keyboard_status.up | controller_status.up;
    cpu->joypad.joypad.down = keyboard_status.down | controller_status.down;
    cpu->joypad.joypad.left = keyboard_status.left | controller_status.left;
    cpu->joypad.joypad.right = keyboard_status.right | controller_status.right;
}
