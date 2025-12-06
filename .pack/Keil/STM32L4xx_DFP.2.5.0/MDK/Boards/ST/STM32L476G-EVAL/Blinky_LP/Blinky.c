/*------------------------------------------------------------------------------
 * Copyright (c) 2017 - 2019 Arm Limited (or its affiliates). All
 * rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   1.Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   2.Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   3.Neither the name of Arm nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *------------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------*/

#include "main.h"
#include "Board_LED.h"                  // Board Support:LED
#include "Board_Buttons.h"              // Board Support:Buttons

#include "RTX_Config.h"

static osThreadId_t tid_thrLED;         // Thread id of thread: LED
static osThreadId_t tid_thrBUT;         // Thread id of thread: BUT

/*------------------------------------------------------------------------------
  thrLED: blink LED
 *----------------------------------------------------------------------------*/
__NO_RETURN static void thrLED (void *arg) {
  uint32_t active_flag = 1U;

  (void)arg;

  for (;;) {
    if (osThreadFlagsWait (1U, osFlagsWaitAny, 0U) == 1U) {
      active_flag ^=1U; 
    }

    if (active_flag == 1U){
      LED_On (0U);                                // Switch LED on
      osDelay (500U);                             // Delay 500 ms
      LED_Off (0U);                               // Switch off
      osDelay (500U);                             // Delay 500 ms
    }
    else {
      osDelay (500U);                             // Delay 500 ms
    }
  }
}

/*------------------------------------------------------------------------------
  thrBUT: check button state
 *----------------------------------------------------------------------------*/
__NO_RETURN static void thrBUT(void *arg) {
  uint32_t last = 0U;
  uint32_t state;

  (void)arg;

  for (;;) {
    state = (Buttons_GetState () & 1U);           // Get pressed button state
    if (state != last){
      if (state == 1){
        osThreadFlagsSet (tid_thrLED, 1U);        // Set flag to thrLED
      }
    }
    last = state;
    osDelay (100U);
  }
}

/*------------------------------------------------------------------------------
 * Application main thread
 *----------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;

  LED_Initialize ();                              // Initialize LEDs
  Buttons_Initialize ();                          // Initialize Buttons

  tid_thrLED = osThreadNew (thrLED, NULL, NULL);  // Create LED thread
  if (tid_thrLED == NULL) { /* add error handling */ }

  tid_thrBUT = osThreadNew (thrBUT, NULL, NULL);  // Create BUT thread
  if (tid_thrBUT == NULL) { /* add error handling */ }

  osThreadExit();
}

/*----------------------------------------------------------------------------
 *      Low-Power Extension
 *---------------------------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;
static volatile unsigned int tc_wakeup;
  
void osRtxIdleThread (void) {
  
  for (;;) {
    tc_wakeup = osKernelSuspend();
    /* Is there some time to sleep? */
    if (tc_wakeup > 0) {
      /* Re-enable wakeup source */
      HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, tc_wakeup * (OS_TICK_FREQ/1000U) * 2, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
     
      /* Enter the low power state */
      HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI);
      
      /* Disable all used wakeup source */
      HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
      
      /* Reconfigure the system clock */
      SystemClock_Config();
    }
    /* Adjust the kernel ticks with the amount of ticks slept */
    osKernelResume (tc_wakeup);
  }
}

/*----------------------------------------------------------------------------
 *      Event Recorder - User Timer
 *---------------------------------------------------------------------------*/
/**
  Setup timer hardware
  \return       status (1=Success, 0=Failure)
*/
uint32_t EventRecorderTimerSetup (void) {
  return 1U;
}

 
/**
  Get timer frequency
  \return       timer frequency in Hz
*/
uint32_t EventRecorderTimerGetFreq (void) {
  return (LSE_VALUE);
}

/**
  Get timer count
  \return       timer count (32-bit)
*/
uint32_t EventRecorderTimerGetCount (void) {
  uint32_t cnt = 0;
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  if ((hrtc.Instance != NULL)&&(hrtc.State != HAL_RTC_STATE_RESET)) {
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    cnt = (sTime.Hours * 3600U * LSE_VALUE) +
          (sTime.Minutes * 60U * LSE_VALUE) +
          (sTime.Seconds       * LSE_VALUE) +
          (LSE_VALUE - sTime.SubSeconds);
  }
  return cnt;
}
