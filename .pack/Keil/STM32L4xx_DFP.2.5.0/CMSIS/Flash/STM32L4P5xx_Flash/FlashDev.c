/* -----------------------------------------------------------------------------
 * Copyright (c) 2014 - 2019 ARM Ltd.
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
 * $Date:        01. September 2020
 * $Revision:    V1.2.0
 *
 * Project:      Flash Device Description for ST STM32L4P5xx Flash
 * --------------------------------------------------------------------------- */

/* History:
 *  version 1.2.0
 *		add STM32L4P5xx_512 loader configuration 
 *  Version 1.1.0
 *    Changed to 4K Sector size (logical)
 *  Version 1.0.0
 *    Initial release
 */

/* Note:
   Flash category 3 devices DualBank configuration have 4K sector size.
   Flash category 3 devices SingleBank configuration have 8K sector size.
   We use 4K sector size for all. In case of 8K physical size two contiguous sectors are deleted. */

#include "..\FlashOS.h"        // FlashOS Structures


#ifdef FLASH_MEM

#ifdef STM32L4P5xx_1M
  struct FlashDevice const FlashDevice  =  {
     FLASH_DRV_VERS,             // Driver Version, do not modify!
     "STM32L4P5xx_1M Flash",   // Device Name
     ONCHIP,                     // Device Type
     0x08000000,                 // Device Start Address
     0x00100000,                 // Device Size in Bytes (1024kB)
     1024,                       // Programming Page Size
     0,                          // Reserved, must be 0
     0xFF,                       // Initial Content of Erased Memory
     400,                        // Program Page Timeout 400 mSec
     400,                        // Erase Sector Timeout 400 mSec
     // Specify Size and Address of Sectors
     0x1000, 0x000000,           // Sector Size  4kB
     SECTOR_END
  };
#endif
	
#ifdef STM32L4P5xx_512
  struct FlashDevice const FlashDevice  =  {
     FLASH_DRV_VERS,             // Driver Version, do not modify!
     "STM32L4P5xx_512 Flash",   // Device Name
     ONCHIP,                     // Device Type
     0x08000000,                 // Device Start Address
     0x00080000,                 // Device Size in Bytes (512kB)
     1024,                       // Programming Page Size
     0,                          // Reserved, must be 0
     0xFF,                       // Initial Content of Erased Memory
     400,                        // Program Page Timeout 400 mSec
     400,                        // Erase Sector Timeout 400 mSec
     // Specify Size and Address of Sectors
     0x1000, 0x000000,           // Sector Size  4kB
     SECTOR_END
  };
#endif	

#endif // FLASH_MEM
