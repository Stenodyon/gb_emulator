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

#include <cmath>

#include "stdafx.h"
#include "CPU.h"

void CPU::run()
{
    std::cout << "--- Starting execution ---" << std::endl;
    while (running)
    {
        if (!halted)
            step();
        else
            cycleWait(4);
        checkInterrupts();
    }
    std::cout << "--- Execution stopped ---" << std::endl;

    regs.dump();
    stack_trace.dump();
    std::cout << "IE: " << hex<uint8_t>(int_enable) << "  IF: " << hex<uint8_t>(int_flag)
        << "  Interrupts " << (interruptsEnabled ? "enabled" : "disabled") << std::endl;
#ifdef _DEBUG
#endif
}

void CPU::DMATranfer(uint8_t source)
{
    dma_source = (uint16_t)source << 8;
    dma_left = 0xA0;
    dma_timer = 0;
}


void CPU::updateInput()
{
    SDL_GameControllerUpdate();
    if(display.controller != nullptr)
    {
        display.controller_status.button_a = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_B);
        display.controller_status.button_b = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_A);
        display.controller_status.button_start = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_START);
        display.controller_status.button_select = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_BACK);
        display.controller_status.up = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
        display.controller_status.down = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        display.controller_status.left = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        display.controller_status.right = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    }
}

void CPU::updateDMA()
{
    if (dma_left > 0)
    {
        dma_timer += 1.;
        while (dma_timer >= DMA_PER_CYCLE)
        {
            uint16_t source_index = dma_source + (0xA0 - dma_left);
            uint16_t dest_index = DMA_DEST + (0xA0 - dma_left);
            uint8_t value = ram.read(source_index);
            ram.writeB(dest_index, value);
            dma_left--;
            dma_timer = std::fmod(dma_timer, DMA_PER_CYCLE);
        }
    }
}
