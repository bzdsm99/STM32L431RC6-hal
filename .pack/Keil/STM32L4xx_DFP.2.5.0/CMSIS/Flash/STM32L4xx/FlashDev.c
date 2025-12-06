/* -----------------------------------------------------------------------------
 * Copyright (c) 2014 - 2017 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        29. March 2017
 * $Revision:    V1.1.0
 *
 * Project:      Flash Device Description for ST STM32L4xx Flash
 * --------------------------------------------------------------------------- */

/* History:
 *  Version 1.1.0
 *    Added OPT Algorithms
 *  Version 1.0.0
 *    Initial release
 */

#include "..\FlashOS.h"        // FlashOS Structures

#ifdef FLASH_MEM

#ifdef STM32L4x_1024
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    "STM32L4xx 1MB Flash",      // Device Name (1024kB)
    ONCHIP,                     // Device Type
    0x08000000,                 // Device Start Address
    0x00100000,                 // Device Size in Bytes (1024kB)
    1024,                       // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    400,                        // Program Page Timeout 400 mSec
    400,                        // Erase Sector Timeout 400 mSec
    // Specify Size and Address of Sectors
    0x800, 0x000000,            // Sector Size  2kB
    SECTOR_END
  };
#endif

#ifdef STM32L4x_512
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    "STM32L4xx 512 KB Flash",   // Device Name (512kB)
    ONCHIP,                     // Device Type
    0x08000000,                 // Device Start Address
    0x00080000,                 // Device Size in Bytes (512kB)
    1024,                       // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    400,                        // Program Page Timeout 400 mSec
    400,                        // Erase Sector Timeout 400 mSec
    // Specify Size and Address of Sectors
    0x800, 0x000000,             // Sector Size  2kB
    SECTOR_END
  };
#endif

#ifdef STM32L4x_256
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    "STM32L4xx 256 KB Flash",   // Device Name (256kB)
    ONCHIP,                     // Device Type
    0x08000000,                 // Device Start Address
    0x00040000,                 // Device Size in Bytes (256kB)
    1024,                       // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    400,                        // Program Page Timeout 400 mSec
    400,                        // Erase Sector Timeout 400 mSec
    // Specify Size and Address of Sectors
    0x800, 0x000000,            // Sector Size  2kB
    SECTOR_END
  };
#endif

#ifdef STM32L4x_128
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    "STM32L4xx 128 KB Flash",   // Device Name (256kB)
    ONCHIP,                     // Device Type
    0x08000000,                 // Device Start Address
    0x00020000,                 // Device Size in Bytes (128kB)
    1024,                       // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    400,                        // Program Page Timeout 400 mSec
    400,                        // Erase Sector Timeout 400 mSec
    // Specify Size and Address of Sectors
    0x800, 0x000000,            // Sector Size  2kB
    SECTOR_END
  };
#endif

#ifdef STM32L4x_64
  struct FlashDevice const FlashDevice  =  {
    FLASH_DRV_VERS,             // Driver Version, do not modify!
    "STM32L4xx 64 KB Flash",   // Device Name (256kB)
    ONCHIP,                     // Device Type
    0x08000000,                 // Device Start Address
    0x00010000,                 // Device Size in Bytes (128kB)
    1024,                       // Programming Page Size
    0,                          // Reserved, must be 0
    0xFF,                       // Initial Content of Erased Memory
    400,                        // Program Page Timeout 400 mSec
    400,                        // Erase Sector Timeout 400 mSec
    // Specify Size and Address of Sectors
    0x800, 0x000000,            // Sector Size  2kB
    SECTOR_END
  };
#endif

#endif // FLASH_MEM


#if defined FLASH_OTP

  struct FlashDevice const FlashDevice  =  {
      FLASH_DRV_VERS,                     // Driver Version, do not modify!
      "STM32L4xx Flash OTP",              // Device Name
      ONCHIP,                             // Device Type
      0x1FFF7000,                         // Device Start Address
      0x00000400,                         // Device Size in Bytes (1024)
      1024,                               // Programming Page Size
      0,                                  // Reserved, must be 0
      0xFF,                               // Initial Content of Erased Memory
      3000,                               // Program Page Timeout 3 Sec
      3000,                               // Erase Sector Timeout 3 Sec
      // Specify Size and Address of Sectors
      0x0400, 0x000000,                   // Sector Size 1024B
      SECTOR_END
  };

#endif // FLASH_OTP


#if defined FLASH_OPT

#if defined FLASH_SB
  struct FlashDevice const FlashDevice  =  {
      FLASH_DRV_VERS,                     // Driver Version, do not modify!
      "STM32L4xx single bank Flash Options", // Device Name
      ONCHIP,                             // Device Type
      0x1FFF7800,                         // Device Start Address
      0x00000014,                         // Device Size in Bytes (20)
      20,                                 // Programming Page Size
      0,                                  // Reserved, must be 0
      0xFF,                               // Initial Content of Erased Memory
      3000,                               // Program Page Timeout 3 Sec
      3000,                               // Erase Sector Timeout 3 Sec
      // Specify Size and Address of Sectors
      0x0014, 0x000000,                   // Sector Size 20B
      SECTOR_END
  };
#endif // FLASH_SB

#if defined FLASH_DB
  struct FlashDevice const FlashDevice  =  {
      FLASH_DRV_VERS,                     // Driver Version, do not modify!
      "STM32L4xx dual bank Flash Options", // Device Name
      ONCHIP,                             // Device Type
      0x1FFF7800,                         // Device Start Address
      0x00000024,                         // Device Size in Bytes (36)
      36,                                 // Programming Page Size
      0,                                  // Reserved, must be 0
      0xFF,                               // Initial Content of Erased Memory
      3000,                               // Program Page Timeout 3 Sec
      3000,                               // Erase Sector Timeout 3 Sec
      // Specify Size and Address of Sectors
      0x0024, 0x000000,                   // Sector Size 36B
      SECTOR_END
  };
#endif // FLASH_DB

#endif // FLASH_OPT
