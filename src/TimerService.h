#pragma once
#include <Arduino.h>

enum TimerMode {
    TIMER_MODE_STOPWATCH = 0, // Counts up (MM:SS.t)
    TIMER_MODE_COUNTDOWN = 1  // Counts down from duration (MM:SS)
};

enum TimerState {
    TIMER_STATE_STOPPED = 0,
    TIMER_STATE_RUNNING = 1,
    TIMER_STATE_PAUSED  = 2,
    TIMER_STATE_EXPIRED = 3
};

void Timer_Init();
void Timer_Update();

void Timer_Start();
void Timer_Pause();
void Timer_Toggle();
void Timer_Reset();

void Timer_SetMode(TimerMode mode);
TimerMode Timer_GetMode();

TimerState Timer_GetState();

void Timer_SetDuration(uint32_t seconds);
uint32_t Timer_GetDuration();
uint32_t Timer_GetRemainingSeconds();
uint32_t Timer_GetElapsedMillis();

void Timer_AdjustDuration(int32_t deltaSeconds);

void Timer_Render();
