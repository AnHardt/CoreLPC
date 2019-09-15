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

//SD:: Modified for use with RRF
#include "Core.h"
#include "chip.h"
#include <stddef.h>
#include "us_ticker_api.h"

/* PORTING STEP 4:
   Implement
       * "us_ticker_init", "us_ticker_read"
       * "us_ticker_set_interrupt", "us_ticker_disable_interrupt", "us_ticker_clear_interrupt"
 */
#define US_TICKER_TIMER      ((LPC_TIMER_T *)LPC_TIMER3_BASE)
#define US_TICKER_TIMER_IRQn TIMER3_IRQn

static int us_ticker_running = 0;

static inline void us_ticker_init(void) {
    us_ticker_running = 1;

    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_TIMER3); //enable power and clocking
    
    US_TICKER_TIMER->CTCR = 0x0; // timer mode
    uint32_t PCLK = SystemCoreClock / 4;

    US_TICKER_TIMER->TCR = 0x2;  // reset

    uint32_t prescale = PCLK / 1000000; // default to 1MHz (1 us ticks)
    US_TICKER_TIMER->PR = prescale - 1;
    US_TICKER_TIMER->TCR = 1; // enable = 1, reset = 0
}

uint32_t us_ticker_read() {
    if (!us_ticker_running)
        us_ticker_init();

    return US_TICKER_TIMER->TC;
}

static inline void us_ticker_set_interrupt(unsigned int timestamp) {
    // set match value
    US_TICKER_TIMER->MR[0] = timestamp;
    // enable match interrupt
    US_TICKER_TIMER->MCR |= 1;
}

static inline void us_ticker_disable_interrupt(void) {
    US_TICKER_TIMER->MCR &= ~1;
}

static inline void us_ticker_clear_interrupt(void) {
    US_TICKER_TIMER->IR = 1;
}

static ticker_event_handler event_handler;
static ticker_event_t *head = NULL;

//void irq_handler(void) {
void TIMER3_IRQHandler(void) {
    us_ticker_clear_interrupt();

    /* Go through all the pending TimerEvents */
    while (1) {
        if (head == NULL) {
            // There are no more TimerEvents left, so disable matches.
            us_ticker_disable_interrupt();
            return;
        }

        if ((int)(head->timestamp - us_ticker_read()) <= 0) {
            // This event was in the past:
            //      point to the following one and execute its handler
            ticker_event_t *p = head;
            head = head->next;

            event_handler(p->id); // NOTE: the handler can set new events

        } else {
            // This event and the following ones in the list are in the future:
            //      set it as next interrupt and return
            us_ticker_set_interrupt(head->timestamp);
            return;
        }
    }
}

void us_ticker_set_handler(ticker_event_handler handler) {
    event_handler = handler;

    //NVIC_SetVector(US_TICKER_TIMER_IRQn, (uint32_t)irq_handler);
    NVIC_EnableIRQ(US_TICKER_TIMER_IRQn);
}

void us_ticker_insert_event(ticker_event_t *obj, unsigned int timestamp, uint32_t id) {
    /* disable interrupts for the duration of the function */
    __disable_irq();

    // initialise our data
    obj->timestamp = timestamp;
    obj->id = id;

    /* Go through the list until we either reach the end, or find
       an element this should come before (which is possibly the
       head). */
    ticker_event_t *prev = NULL, *p = head;
    while (p != NULL) {
        /* check if we come before p */
        if ((int)(timestamp - p->timestamp) <= 0) {
            break;
        }
        /* go to the next element */
        prev = p;
        p = p->next;
    }
    /* if prev is NULL we're at the head */
    if (prev == NULL) {
        head = obj;
        us_ticker_set_interrupt(timestamp);
    } else {
        prev->next = obj;
    }
    /* if we're at the end p will be NULL, which is correct */
    obj->next = p;

    __enable_irq();
}

void us_ticker_remove_event(ticker_event_t *obj) {
    __disable_irq();

    // remove this object from the list
    if (head == obj) { // first in the list, so just drop me
        head = obj->next;
        if (obj->next != NULL) {
            us_ticker_set_interrupt(head->timestamp);
        }
    } else {            // find the object before me, then drop me
        ticker_event_t* p = head;
        while (p != NULL) {
            if (p->next == obj) {
                p->next = obj->next;
                break;
            }
            p = p->next;
        }
    }

    __enable_irq();
}
