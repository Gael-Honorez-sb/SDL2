/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#ifdef SDL_TIMER_NDS

#include <nds.h>
#include <nds/timers.h>

#include "SDL_timer.h"
#include "../SDL_timer_c.h"

static Uint32 timer_ticks;

void
SDL_StartTicks(void)
{
    if(!timer_alive) {
        SDL_SYS_TimerInit();
        SDL_SYS_StartTimer();
    }

    timer_ticks = 0;
}

Uint32
SDL_GetTicks(void)
{
    return timer_ticks;
}

void
SDL_Delay(Uint32 ms)
{
    Uint32 start = SDL_GetTicks();
    while(timer_alive) {
        if((SDL_GetTicks() - start) >= ms) break;
    }
}

/* Data to handle a single periodic alarm */
static int timer_alive = 0;
static int timer_ticks = 0;

static int
RunTimer(void *unused)
{
    while (timer_alive) {
        if (SDL_timer_running) {
        }
        SDL_Delay(1);
    }
    return (0);
}

void NDS_TimerInterrupt() {
    printf("timer irq\n");
}

/* This is only called if the event thread is not running */
int
SDL_SYS_TimerInit(void)
{
    timer_alive = 1;
    timer_ticks = 0;
    TIMER_CR(0) = TIMER_DIV_1024 | TIMER_IRQ_REQ;
    TIMER_DATA(0) = TIMER_FREQ_1024(1000);
    irqSet(IRQ_TIMER1, NDS_TimerInterrupt);
    return 0;
}

void
SDL_SYS_TimerQuit(void)
{
    if (timer_alive) {
        TIMER_CR(0) = 0;
    }
    timer_alive = 0;
}

int
SDL_SYS_StartTimer(void)
{
    TIMER_CR(0) |= TIMER_ENABLE;
    return 0;
}

void
SDL_SYS_StopTimer(void)
{
    TIMER_CR(0) &= ~TIMER_ENABLE;
    return;
}


#endif /* SDL_TIMER_NDS */
/* vi: set ts=4 sw=4 expandtab: */
