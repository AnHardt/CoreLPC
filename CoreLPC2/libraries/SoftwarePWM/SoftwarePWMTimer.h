/* mbed Microcontroller Library
* Copyright (c) 2006-2013 ARM Limited
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

//SD:: Code from mbed us_ticker_api merged into class for Software PWM Timer

#ifndef SOFTWAREPWMTIMER_H
#define SOFTWAREPWMTIMER_H

#include "Core.h"
#include "chip.h"
#ifdef LPC_DEBUG
#include "MessageType.h"
#endif

class SoftwarePWM; //Fwd decl

typedef struct ticker_event_s
{
    uint32_t timestamp;
    SoftwarePWM *PWM;
    struct ticker_event_s *next;
} ticker_event_t;



class SoftwarePWMTimer
{
    
    
public:
    SoftwarePWMTimer();
    
    void ScheduleEventInMicroseconds(ticker_event_t *obj, uint32_t microseconds, SoftwarePWM *softPWMObject);
    void RemoveEvent(ticker_event_t *obj);
    void Interrupt();

    inline uint32_t TickerRead(){ return LPC_RITIMER->COUNTER; };
#ifdef NEWCODE
    inline uint32_t TicksPerMicrosecond(){ return ticksPerMicrosecond; };
#else
    inline uint32_t TicksPerMicrosecond(){ return Chip_Clock_GetPeripheralClockRate(SYSCTL_PCLK_RIT)/1000000; };
#endif

#ifdef LPC_DEBUG
    void Diagnostics(MessageType mtype) noexcept;
#endif

private:
    void ticker_set_interrupt(ticker_event_t *obj, bool inInterrupt=false);
    void ticker_disable_interrupt(void);
    void ticker_clear_interrupt(void);
    void ticker_insert_event(ticker_event_t *obj, uint32_t timestamp, SoftwarePWM *softPWMObject);
    ticker_event_t *head;
#ifdef NEWCODE
    const uint32_t ticksPerMicrosecond;
#endif
};


extern SoftwarePWMTimer softwarePWMTimer;

#endif
