/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::USB:Device
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HID.c
 * Purpose: USB Device - Human Interface Device example
 *----------------------------------------------------------------------------*/

#include "main.h"
#include "rl_usb.h"                     // Keil.MDK-Pro::USB:CORE

#include "Board_LED.h"
#include "Board_Joystick.h"
#include "RTE_Components.h"

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};


/*------------------------------------------------------------------------------
 *        Application
 *----------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  uint8_t joy;
  uint8_t joy_last = 0U;

  (void)arg;

  LED_Initialize();                     // LED Initialization
  Joystick_Initialize();                // Joystick Initialization

  USBD_Initialize(0U);                  // USB Device 0 Initialization
  USBD_Connect   (0U);                  // USB Device 0 Connect

  joy = joy_last = 0U;
  for (;;) {                            // Loop forever
    joy = (uint8_t)(Joystick_GetState ());
    if ((joy ^ joy_last) & 0x1FU) {
      joy_last = joy & 0x1FU;
      USBD_HID_GetReportTrigger(0U, 0U, &joy, 1U);
    }
    osDelay(100U);                      // 100 ms delay for sampling buttons
  }
}
