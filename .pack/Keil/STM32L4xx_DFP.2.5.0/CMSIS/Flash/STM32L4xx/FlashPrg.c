/* -----------------------------------------------------------------------------
 * Copyright (c) 2014  - 2017 ARM Ltd.
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
 * Project:      Flash Programming Functions for ST STM32L4xx Flash
 * --------------------------------------------------------------------------- */

/* History:
 *  Version 1.1.0
 *    Added OPT Algorithms
 *  Version 1.0.0
 *    Initial release
 */

#include "..\FlashOS.h"        // FlashOS Structures

typedef volatile unsigned long    vu32;
typedef          unsigned long     u32;

#define M32(adr) (*((vu32 *) (adr)))

// Peripheral Memory Map
#define IWDG_BASE         0x40003000
#define FLASH_BASE        0x40022000
#define DBGMCU_BASE       0xE0042000

#define IWDG            ((IWDG_TypeDef   *) IWDG_BASE)
#define FLASH           ((FLASH_TypeDef  *) FLASH_BASE)
#define DBGMCU          ((DBGMCU_TypeDef *) DBGMCU_BASE)

// Debug MCU
typedef struct {
  vu32 IDCODE;
} DBGMCU_TypeDef;

// Independent WATCHDOG
typedef struct {
  vu32 KR;
  vu32 PR;
  vu32 RLR;
  vu32 SR;
} IWDG_TypeDef;

// Flash Registers
typedef struct {
  vu32 ACR;              /* Offset: 0x00  Access Control Register */
  vu32 PDKEYR;           /* Offset: 0x04  Power Down Key Register */
  vu32 KEYR;             /* Offset: 0x08  Key Register */
  vu32 OPTKEYR;          /* Offset: 0x0C  Option Key Register */
  vu32 SR;               /* Offset: 0x10  Status Register */
  vu32 CR;               /* Offset: 0x14  Control Register */
  vu32 ECCR;             /* Offset: 0x18  ECC Register */
  vu32 RESERVED0;
  vu32 OPTR;             /* Offset: 0x20  Option Register */
  vu32 PCROP1SR;         /* Offset: 0x24  Bank1 PCROP start address Register */
  vu32 PCROP1ER;         /* Offset: 0x28  Bank1 PCROP end address Register */
  vu32 WRP1AR;           /* Offset: 0x2C  Bank1 WRP area A address Register */
  vu32 WRP1BR;           /* Offset: 0x30  Bank1 WRP area B address Register */
  vu32 RESERVED1[4];
  vu32 PCROP2SR;         /* Offset: 0x44  Bank2 PCROP start address Register */
  vu32 PCROP2ER;         /* Offset: 0x48  Bank2 PCROP end address Register */
  vu32 WRP2AR;           /* Offset: 0x4C  Bank2 WRP area A address Register */
  vu32 WRP2BR;           /* Offset: 0x50  Bank2 WRP area B address Register */
} FLASH_TypeDef;


// Flash Keys
//#define RDPRT_KEY                0x00A5
#define FLASH_KEY1               0x45670123
#define FLASH_KEY2               0xCDEF89AB
#define FLASH_OPTKEY1            0x08192A3B
#define FLASH_OPTKEY2            0x4C5D6E7F

// Flash Control Register definitions
#define FLASH_PG                ((unsigned int)(   1U <<  0))
#define FLASH_PER               ((unsigned int)(   1U <<  1))
#define FLASH_MER1              ((unsigned int)(   1U <<  2))
#define FLASH_PNB_MSK           ((unsigned int)(0xFFU <<  3))
#define FLASH_BKER              ((unsigned int)(   1U << 11))
#define FLASH_MER2              ((unsigned int)(   1U << 15))
#define FLASH_STRT              ((unsigned int)(   1U << 16))
#define FLASH_OPTSTRT           ((unsigned int)(   1U << 17))
#define FLASH_OBL_LAUNCH        ((unsigned int)(   1U << 27))
#define FLASH_OPTLOCK           ((unsigned int)(   1U << 30))
#define FLASH_LOCK              ((unsigned int)(   1U << 31))


// Flash Status Register definitions
#define FLASH_EOP               ((unsigned int)(   1U <<  0))
#define FLASH_OPERR             ((unsigned int)(   1U <<  1))
#define FLASH_PROGERR           ((unsigned int)(   1U <<  3))
#define FLASH_WRPERR            ((unsigned int)(   1U <<  4))
#define FLASH_PGAERR            ((unsigned int)(   1U <<  5))
#define FLASH_SIZERR            ((unsigned int)(   1U <<  6))
#define FLASH_PGSERR            ((unsigned int)(   1U <<  7))
#define FLASH_MISSERR           ((unsigned int)(   1U <<  8))
#define FLASH_FASTERR           ((unsigned int)(   1U <<  9))
#define FLASH_RDERR             ((unsigned int)(   1U << 14))
#define FLASH_OPTVERR           ((unsigned int)(   1U << 15))
#define FLASH_BSY               ((unsigned int)(   1U << 16))

#define FLASH_PGERR             (FLASH_OPERR  | FLASH_PROGERR | FLASH_PROGERR | FLASH_WRPERR  | FLASH_PGAERR | \
                                 FLASH_SIZERR | FLASH_PGSERR  | FLASH_MISSERR | FLASH_FASTERR | FLASH_RDERR  | \
                                 FLASH_OPTVERR                                                                )

#if defined FLASH_MEM
  #define FLASH_START             (0x08000000U)
  #if   defined STM32L4x_1024      /* 2 x 512 KB  or 1 x 1024 KB */
    #define FLASH_BANK_SIZE       (0x00080000U)
  #elif defined STM32L4x_512       /* 2 x 256 KB  or 1 x  512 KB */
    #define FLASH_BANK_SIZE       (0x00040000U)
  #elif defined STM32L4x_256       /* 2 x 128 KB  or 1 x  256 KB */
    #define FLASH_BANK_SIZE       (0x00020000U)
  #elif defined STM32L4x_128       /*                1 x  128 KB */
    #define FLASH_BANK_SIZE       (0x00010000U)
   #elif defined STM32L4x_64       /*                1 x  64 KB */
    #define FLASH_BANK_SIZE       (0x00010000U)
  #else
    #error STM32L4xx Flash size not defined!
  #endif
#endif // FLASH_MEM

void DSB(void) {
    __asm("DSB");
}


/*
 * Get Flash Type
 *    Return Value:   0 = Single-Bank flash
 *                    1 = Dual-Bank Flash (may configurable)
 */

#if defined FLASH_MEM
u32 GetFlashType (void) {
  u32 flashType;

  switch ((DBGMCU->IDCODE & 0xFFFU)) {
    case 0x435 :
		case 0x462 :	
		case 0x464 :		
		/* these devices have only a singe bank flash */
      flashType = 0U;       // Single-Bank Flash type
    break;

    default :
      /* all other devices have a dual bank flash
          which is maybe configurable via FLASH_OPTR.DUALBANK */
      flashType = 1U;       // Dual-Bank Flash type
    break;
  }

  return (flashType);
}
#endif // FLASH_MEM


/*
 * Get DUALBANK Option
 *    Return Value:   0 = Single-Bank flash
 *                    1 = Dual-Bank Flash (may configurable)
 */

#if defined FLASH_MEM
u32 GetDualBankOption (void) {
  u32 dualBankOption;

  dualBankOption = ((FLASH->OPTR >> 21) & 1U);

  return (dualBankOption);
}
#endif // FLASH_MEM


/*
 * Get Bank Number
 *    Parameter:      adr:  Sector Address
 *    Return Value:   Bank Number
 */

#if defined FLASH_MEM
unsigned long GetBankNum(unsigned long adr) {
  unsigned long bankNum;

  if (GetFlashType() == 1U) {
    /* Dual-Bank Flash */
    if (GetDualBankOption() == 1U) {
      /* Dual-Bank Flash configured as Dual-Bank */
      if (adr >= (FLASH_START + FLASH_BANK_SIZE)) {
        bankNum = 1U;
      }
      else {
        bankNum = 0U;
      }
    }
    else {
      /* Dual-Bank Flash configured as Single-Bank */
      bankNum = 0U;
    }
  }
  else {
    /* Single-Bank Flash */
    bankNum = 0u;
  }

  return (bankNum);                                     // Bank Number
}
#endif // FLASH_MEM


/*
 * Get Sector Number
 *    Parameter:      adr:  Sector Address
 *    Return Value:   Sector Number
 */

#if defined FLASH_MEM
unsigned long GetSecNum (unsigned long adr) {
  unsigned long secNum;

  if (GetFlashType() == 1U) {
    /* Dual-Bank Flash */
    /* sector number 0..255 for both halfs */
    secNum = (((adr & 0x001FFFFF) & ~FLASH_BANK_SIZE) >> 11);
#if 0
    /* during test I found out that I can erase sectors in second half with number 0..255 or 256..511 */
    if (GetDualBankOption() == 1U) {
      /* Dual-Bank Flash configured as Dual-Bank */
      /* sector number  = 0..255, bank selection is done via bit FLASH_CR.BKER */
    }
    else {
      /* Dual-Bank Flash configured as Single-Bank */
      /* sector number  = 0..255 for 1st half, 256..511 2nd half */
      if (adr & FLASH_BANK_SIZE) {
        secNum += 256;                                  // Sector in second half (this is a guess, could not test)
      }
    }
#else
    /* this code is according pictures in the reference manuals */
    if (adr & FLASH_BANK_SIZE) {
      secNum += 256;                                    // Sector in second half (this is a guess, could not test)
    }
#endif
  }
  else {
    /* Single-Bank Flash */
    /* sector number 0..127 */
    secNum = (((adr & 0x001FFFFF)                    ) >> 11);
  }

  return (secNum);                                      // Sector Number
}
#endif // FLASH_MEM


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  FLASH->KEYR = FLASH_KEY1;                             // Unlock Flash
  FLASH->KEYR = FLASH_KEY2;

#ifdef FLASH_OPT
  FLASH->OPTKEYR  = FLASH_OPTKEY1;                      // Unlock Option Bytes
  FLASH->OPTKEYR  = FLASH_OPTKEY2;
#endif // FLASH_OPT

  FLASH->ACR  = 0x00000000;                             // Zero Wait State, no Cache, no Prefetch
  FLASH->SR   = FLASH_PGERR;                            // Reset Error Flags

  if ((FLASH->OPTR & 0x10000) == 0x00000) {             // Test if IWDG is running (IWDG in HW mode)
    // Set IWDG time out to ~32.768 second
    IWDG->KR  = 0x5555;                                 // Enable write access to IWDG_PR and IWDG_RLR
    IWDG->PR  = 0x06;                                   // Set prescaler to 256
    IWDG->RLR = 4095;                                   // Set reload value to 4095
  }

  return (0);                                           // Done
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {

  FLASH->CR  = FLASH_LOCK;                              // Lock Flash

#ifdef FLASH_OPT
//  FLASH->CR  = FLASH_OBL_LAUNCH;                        // Load option bytes
//  while ((FLASH->CR & FLASH_OBL_LAUNCH) != 0) {
//    IWDG->KR = 0xAAAA;                                  // Reload IWDG
//  }
  FLASH->CR = FLASH_OPTLOCK;                            // Lock option bytes
#endif // FLASH_OPT

  return (0);                                           // Done
}


/*
 *  Blank Check Checks if Memory is Blank
 *    Parameter:      adr:  Block Start Address
 *                    sz:   Block Size (in bytes)
 *                    pat:  Block Pattern
 *    Return Value:   0 - OK,  1 - Failed
 */

int BlankCheck (unsigned long adr, unsigned long sz, unsigned char pat) {
  /* force erase even if the content is 'Initial Content of Erased Memory'.
     Only a erased sector can be programmed. I think this is because of ECC */
  return (1);
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

#if defined FLASH_MEM
int EraseChip (void) {

  FLASH->SR  = FLASH_PGERR;                             // Reset Error Flags

  FLASH->CR  = (FLASH_MER1 | FLASH_MER2);               // Bank A/B Mass Erase Enabled
  FLASH->CR |=  FLASH_STRT;                             // Start Erase

  while (FLASH->SR & FLASH_BSY) {
    IWDG->KR = 0xAAAA;                                  // Reload IWDG
  }

  FLASH->CR  =  0;                                      // Reset CR

  return (0);                                           // Done
}
#endif // FLASH_MEM

#ifdef FLASH_OPT
int EraseChip (void) {

  FLASH->SR     = FLASH_PGERR;                          // Reset Error Flags

  FLASH->OPTR     = 0xFFEFF8AA;
  FLASH->PCROP1SR = 0xFFFFFFFF;
  FLASH->PCROP1ER = 0xFFFF0000;
  FLASH->WRP1AR   = 0xFF00FFFF;
  FLASH->WRP1BR   = 0xFF00FFFF;
#if defined FLASH_DB
  FLASH->PCROP2SR = 0xFFFFFFFF;
  FLASH->PCROP2ER = 0xFFFF0000;
  FLASH->WRP2AR   = 0xFF00FFFF;
  FLASH->WRP2BR   = 0xFF00FFFF;
#endif

  FLASH->CR       = FLASH_OPTSTRT;                      // Program values
  DSB();

  while (FLASH->SR & FLASH_BSY) {
    IWDG->KR = 0xAAAA;                                  // Reload IWDG
  }

  if (FLASH->SR & FLASH_PGERR) {                        // Check for Error
    FLASH->SR |= FLASH_PGERR;                           // Reset Error Flags
    return (1);                                         // Failed
  }

  return (0);                                           // Done
}
#endif // FLASH_OPT


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_MEM
int EraseSector (unsigned long adr) {
  unsigned long b, n;

  b = GetBankNum(adr);                                  // Get Bank Number   0..1
  n = GetSecNum(adr);                                   // Get Sector Number 0..255 or 0..511

  FLASH->SR  = FLASH_PGERR;                             // Reset Error Flags

  FLASH->CR  = (FLASH_PER |                             // Page Erase Enabled
                (n <<  3) |                             // Sector Number. 0 to 255 for each bank
                (b << 11)  );                           // Bank 0 start 0x00000000. Bank 1 start 0x00080000
  FLASH->CR |=  FLASH_STRT;                             // Start Erase
  DSB();

  while (FLASH->SR & FLASH_BSY) {
    IWDG->KR = 0xAAAA;                                  // Reload IWDG
  }

  FLASH->CR  =  0;                                      // Reset CR

  if (FLASH->SR & FLASH_PGERR) {                        // Check for Error
    FLASH->SR  = FLASH_PGERR;                           // Reset Error Flags
    return (1);                                         // Failed
  }

  return (0);                                           // Done
}
#endif // FLASH_MEM

#if defined FLASH_OPT || defined FLASH_OTP
int EraseSector (unsigned long adr) {
  /* erase sector is not needed for
     - Flash Option bytes
     - Flash One Time Programmable bytes
  */
  return (0);                                           // Done
}
#endif


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

#if defined FLASH_MEM || defined FLASH_OTP
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {

  sz = (sz + 7) & ~7;                                   // Adjust size for two words

  FLASH->SR  = FLASH_PGERR;                             // Reset Error Flags
  FLASH->CR  =  0;                                      // Reset CR

  while (sz) {
    FLASH->CR = FLASH_PG ;                              // Programming Enabled

    M32(adr    ) = *((u32 *)(buf + 0));                 // Program the first word of the Double Word
    M32(adr + 4) = *((u32 *)(buf + 4));                 // Program the second word of the Double Word
    DSB();

    while (FLASH->SR & FLASH_BSY) {
      IWDG->KR = 0xAAAA;                                // Reload IWDG
    }

    FLASH->CR  =  0;                                    // Reset CR

    if (FLASH->SR & FLASH_PGERR) {                      // Check for Error
      FLASH->SR  = FLASH_PGERR;                         // Reset Error Flags
      return (1);                                       // Failed
    }

    adr += 8;                                           // Go to next DoubleWord
    buf += 8;
    sz  -= 8;
  }

  return (0);                                           // Done
}
#endif // FLASH_MEM || FLASH_OTP

#ifdef FLASH_OPT
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  u32 optr;
  u32 pcrop1sr;
  u32 pcrop1er;
  u32 wrp1ar;
  u32 wrp1br;
#if defined FLASH_DB
  u32 pcrop2sr;
  u32 pcrop2er;
  u32 wrp2ar;
  u32 wrp2br;
#endif

  optr     = *((u32 *)(buf +  0));
  pcrop1sr = *((u32 *)(buf +  4));
  pcrop1er = *((u32 *)(buf +  8));
  wrp1ar   = *((u32 *)(buf + 12));
  wrp1br   = *((u32 *)(buf + 16));
#if defined FLASH_DB
  pcrop2sr = *((u32 *)(buf + 20));
  pcrop2er = *((u32 *)(buf + 24));
  wrp2ar   = *((u32 *)(buf + 28));
  wrp2br   = *((u32 *)(buf + 32));
#endif

  FLASH->SR  = FLASH_PGERR;                             // Reset Error Flags

  FLASH->OPTR     = (optr     & 0x0FFF77FF);            // Write OPTR values
  FLASH->PCROP1SR = (pcrop1sr & 0x0000FFFF);            // Write PCROP1SR values
  FLASH->PCROP1ER = (pcrop1er & 0x8000FFFF);            // Write PCROP1ER values
  FLASH->WRP1AR   = (wrp1ar   & 0x00FF00FF);            // Write WRP1AR values
  FLASH->WRP1BR   = (wrp1br   & 0x00FF00FF);            // Write WRP1BR values
#if defined FLASH_DB
  FLASH->PCROP2SR = (pcrop2sr & 0x0000FFFF);            // Write PCROP2SR values
  FLASH->PCROP2ER = (pcrop2er & 0x8000FFFF);            // Write PCROP2ER values
  FLASH->WRP2AR   = (wrp2ar   & 0x00FF00FF);            // Write WRP2AR values
  FLASH->WRP2BR   = (wrp2br   & 0x00FF00FF);            // Write WRP2BR values
#endif

  FLASH->CR  = FLASH_OPTSTRT;                           // Program values
  DSB();

  while (FLASH->SR & FLASH_BSY) {
    IWDG->KR = 0xAAAA;                                  // Reload IWDG
  }

  if (FLASH->SR & FLASH_PGERR) {                        // Check for Error
    FLASH->SR |= FLASH_PGERR;                           // Reset Error Flags
    return (1);                                         // Failed
  }

  return (0);                                           // Done
}
#endif //  FLASH_OPT


/*
 *  Verify Flash Contents
 *    Parameter:      adr:  Start Address
 *                    sz:   Size (in bytes)
 *                    buf:  Data
 *    Return Value:   (adr+sz) - OK, Failed Address
 */

#ifdef FLASH_OPT
unsigned long Verify (unsigned long adr, unsigned long sz, unsigned char *buf) {
  u32 optr;
  u32 pcrop1sr;
  u32 pcrop1er;
  u32 wrp1ar;
  u32 wrp1br;
#if defined FLASH_DB
  u32 pcrop2sr;
  u32 pcrop2er;
  u32 wrp2ar;
  u32 wrp2br;
#endif

  optr     = *((u32 *)(buf +  0));
  pcrop1sr = *((u32 *)(buf +  4));
  pcrop1er = *((u32 *)(buf +  8));
  wrp1ar   = *((u32 *)(buf + 12));
  wrp1br   = *((u32 *)(buf + 16));
#if defined FLASH_DB
  pcrop2sr = *((u32 *)(buf + 20));
  pcrop2er = *((u32 *)(buf + 24));
  wrp2ar   = *((u32 *)(buf + 28));
  wrp2br   = *((u32 *)(buf + 32));
#endif


  if ((FLASH->OPTR     & 0x0FFF77FF) != (optr     & 0x0FFF77FF)) {    // Check OPTR values
    return (adr + 0);
  }

  if ((FLASH->PCROP1SR & 0x0000FFFF) != (pcrop1sr & 0x0000FFFF)) {    // Check PCROP1SR values
    return (adr + 1);
  }

  if ((FLASH->PCROP1ER & 0x8000FFFF) != (pcrop1er & 0x8000FFFF)) {    // Check PCROP1ER values
    return (adr + 2);
  }

  if ((FLASH->WRP1AR   & 0x00FF00FF) != (wrp1ar   & 0x00FF00FF)) {    // Check WRP1AR values
    return (adr + 3);
  }

  if ((FLASH->WRP1BR   & 0x00FF00FF) != (wrp1br   & 0x00FF00FF)) {    // Check WRP1BR values
    return (adr + 4);
  }

#if defined FLASH_DB
  if ((FLASH->PCROP2SR & 0x0000FFFF) != (pcrop2sr & 0x0000FFFF)) {    // Check PCROP2SR values
    return (adr + 5);
  }

  if ((FLASH->PCROP2ER & 0x8000FFFF) != (pcrop2er & 0x8000FFFF)) {    // Check PCROP2ER values
    return (adr + 6);
  }

  if ((FLASH->WRP2AR   & 0x00FF00FF) != (wrp2ar   & 0x00FF00FF)) {    // Check WRP2AR values
    return (adr + 7);
  }

  if ((FLASH->WRP2BR   & 0x00FF00FF) != (wrp2br   & 0x00FF00FF)) {    // Check WRP2BR values
    return (adr + 8);
  }
#endif

  return (adr + sz);
}
#endif // FLASH_OPT
