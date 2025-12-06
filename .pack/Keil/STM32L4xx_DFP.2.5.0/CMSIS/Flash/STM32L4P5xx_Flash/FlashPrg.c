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
 * Project:      Flash Programming Functions for ST STM32L4P5xx Flash
 * --------------------------------------------------------------------------- */

/* History:
 *  Version 1.2.0
 *     fix STM32L4P5 sector erase in Dual Bank mode
 *  Version 1.0.0
 *    Initial release
 */

/* Note:  
   Flash category 3 devices DualBank configuration have 4K sector size.
   Flash category 3 devices SingleBank configuration have 8K sector size.
   We use 4K sector size for all. In case of 8K physical size two contiguous sectors are deleted. */

#include "..\FlashOS.h"        // FlashOS Structures

typedef volatile unsigned long    vu32;
typedef          unsigned long     u32;

#define M32(adr) (*((vu32 *) (adr)))

// Peripheral Memory Map
#define FLASH_BASE        (0x40022000)
#define DBGMCU_BASE       (0xE0042000)     
#define FLASHSIZE_BASE    (0x1FFF75E0)      

#define FLASH           ((FLASH_TypeDef  *) FLASH_BASE)
#define DBGMCU          ((DBGMCU_TypeDef *) DBGMCU_BASE)

// Debug MCU
typedef struct {
  vu32 IDCODE;
} DBGMCU_TypeDef;

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
  vu32 PCROP1SR;         /* Offset: 0x24  Bank1 PCROP Start Address Register */
  vu32 PCROP1ER;         /* Offset: 0x28  Bank1 PCROP End Address Register */
  vu32 WRP1AR;           /* Offset: 0x2C  Bank1 WRP Area A Address Register */
  vu32 WRP1BR;           /* Offset: 0x30  Bank1 WRP Area B Address Register */
  vu32 RESERVED1[4];
  vu32 PCROP2SR;         /* Offset: 0x44  Bank2 PCROP Start Address Register */
  vu32 PCROP2ER;         /* Offset: 0x48  Bank2 PCROP End Address Register */
  vu32 WRP2AR;           /* Offset: 0x4C  Bank2 WRP Area A Address Register */
  vu32 WRP2BR;           /* Offset: 0x50  Bank2 WRP Area B Address Register */
  vu32 RESERVED2[7];
  vu32 SEC1R;            /* Offset: 0x70  Securable Memory Register Bank1 */
  vu32 SEC2R;            /* Offset: 0x74  Securable Memory Register Bank2 */
} FLASH_TypeDef;


// Flash Keys
#define FLASH_KEY1               0x45670123
#define FLASH_KEY2               0xCDEF89AB
#define FLASH_OPTKEY1            0x08192A3B
#define FLASH_OPTKEY2            0x4C5D6E7F

// Flash Control Register definitions
#define FLASH_CR_PG             ((u32)(  1U      ))
#define FLASH_CR_PER            ((u32)(  1U <<  1))
#define FLASH_CR_MER1           ((u32)(  1U <<  2))
#define FLASH_CR_PNB_MSK        ((u32)(0x7F <<  3))
#define FLASH_CR_BKER           ((u32)(  1U << 11))
#define FLASH_CR_MER2           ((u32)(  1U << 15))
#define FLASH_CR_STRT           ((u32)(  1U << 16))
#define FLASH_CR_OPTSTRT        ((u32)(  1U << 17))
#define FLASH_CR_FSTPG          ((u32)(  1U << 18))
#define FLASH_OBL_LAUNCH        ((u32)(  1U << 27))
#define FLASH_CR_OPTLOCK        ((u32)(  1U << 30))
#define FLASH_CR_LOCK           ((u32)(  1U << 31))


// Flash Status Register definitions
#define FLASH_SR_EOP            ((u32)(  1U      ))
#define FLASH_SR_OPERR          ((u32)(  1U <<  1))
#define FLASH_SR_PROGERR        ((u32)(  1U <<  3))
#define FLASH_SR_WRPERR         ((u32)(  1U <<  4))
#define FLASH_SR_PGAERR         ((u32)(  1U <<  5))
#define FLASH_SR_SIZERR         ((u32)(  1U <<  6))
#define FLASH_SR_PGSERR         ((u32)(  1U <<  7))
#define FLASH_SR_MISSERR        ((u32)(  1U <<  8))
#define FLASH_SR_FASTERR        ((u32)(  1U <<  9))
#define FLASH_SR_RDERR          ((u32)(  1U << 14))
#define FLASH_SR_OPTVERR        ((u32)(  1U << 15))
#define FLASH_SR_BSY            ((u32)(  1U << 16))

#define FLASH_PGERR             (FLASH_SR_OPERR   | FLASH_SR_PROGERR | FLASH_SR_WRPERR  | \
                                 FLASH_SR_PGAERR  | FLASH_SR_SIZERR  | FLASH_SR_PGSERR  | \
                                 FLASH_SR_MISSERR | FLASH_SR_FASTERR | FLASH_SR_RDERR   | FLASH_SR_OPTVERR )


// Flash option register definitions
#define FLASH_OPTR_RDP          ((u32)(0xFF      ))
#define FLASH_OPTR_RDP_NO       ((u32)(0xAA      ))
#define FLASH_OPTR_DBANK        ((u32)(  1U << 22))
#define FLASH_OPTR_DB1M        ((u32)(  1U << 21))              /* ADDED DB1M: Dual-Bank on 1MB/512kB Flash (Part Number)*/
                                                       /* 0: 1MB/512kB Single Flash: Continuous addresses of BankA */
                                                        /*1: 1MB/512kB Dual-Bank Flash */


u32 flashBase;                   /* Flash base address */
u32 flashSize;                   /* Flash size in bytes */
u32 flashBankSize;               /* Flash bank size in bytes */
u32 flashPageSize;               /* Flash page size in bytes */

static void DSB(void) {
    __asm("DSB");
}



/*
 * Get Flash Bank Mode
 *    Return Value:   0 = Single-Bank mode
 *                    1 = Dual-Bank mode
 */

#if defined FLASH_MEM
static u32 GetFlashBankMode (void) {
  u32 flashBankMode;

  flashBankMode = (FLASH->OPTR & FLASH_OPTR_DBANK) ? 1U : 0U;

  return (flashBankMode);
}
#endif /* FLASH_MEM */


/*
 * Get Flash Bank Number
 *    Parameter:      adr:  Sector Address
 *    Return Value:   Bank Number (0..1)
 */

#if defined FLASH_MEM
static u32 GetFlashBankNum(u32 adr) {
  u32 flashBankNum;

    /* Dual-Bank Flash */
    if (GetFlashBankMode() == 1U) {
      /* Dual-Bank Flash configured as Dual-Bank */
      if (adr >= (flashBase + flashBankSize)) {
        flashBankNum = 1U;
      }
      else {
        flashBankNum = 0U;
      }
    }
    else {
      /* Dual-Bank Flash configured as Single-Bank */
      flashBankNum = 0U;
    }


  return (flashBankNum);
}
#endif /* FLASH_MEM */


/*
 * Get Flash Page Number
 *    Parameter:      adr:  Page Address
 *    Return Value:   Page Number (0..255)
 */

#if defined FLASH_MEM
static u32 GetFlashPageNum (unsigned long adr) {
  u32 flashPageNum;

    /* Dual-Bank Flash */
    if (GetFlashBankMode() == 1U) {
      /* Dual-Bank Flash configured as Dual-Bank */
      flashPageNum = (((adr & (flashSize - 1U)) ) >> 12); /* 4K sector size */
    }
    else {
      /* Dual-Bank Flash configured as Single-Bank */
      flashPageNum = (((adr & (flashSize  - 1U)) ) >> 13); /* 8K sector size */
    }

  return (flashPageNum);
}
#endif /* FLASH_MEM */


/*
 * Get Flash Page Size
 *    Return Value:   flash page size (in Bytes)
 */

#if defined FLASH_MEM
static u32 GetFlashPageSize (void) {
  u32 flashPageSize;

    /* Dual-Bank Flash */
    if (GetFlashBankMode() == 1U) {
      /* Dual-Bank Flash configured as Dual-Bank */
      flashPageSize = 0x1000;                                /* 4K sector size */
    }
    else {
      /* Dual-Bank Flash configured as Single-Bank */
      flashPageSize = 0x2000;                                /* 8K sector size */
    }


  return (flashPageSize);
}
#endif /* FLASH_MEM */


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  FLASH->KEYR = FLASH_KEY1;                              /* Unlock Flash operation */
  FLASH->KEYR = FLASH_KEY2;

#ifdef FLASH_OPT
  FLASH->OPTKEYR  = FLASH_OPTKEY1;                       /* Unlock Option Bytes operation */
  FLASH->OPTKEYR  = FLASH_OPTKEY2;
#endif /* FLASH_OPT */

  if (((FLASH->OPTR & FLASH_OPTR_RDP) != FLASH_OPTR_RDP_NO))
  {
    FLASH->OPTKEYR = FLASH_OPTKEY1;
    FLASH->OPTKEYR = FLASH_OPTKEY2;

    /* clear  read protection */
    FLASH->OPTR &= ~(FLASH_OPTR_RDP);
    FLASH->OPTR |= FLASH_OPTR_RDP_NO;

    FLASH->CR |= FLASH_CR_OPTSTRT;
    FLASH->CR |= FLASH_OBL_LAUNCH;
    DSB();

    /* Wait until option bytes are updated */
    while (FLASH->CR & FLASH_OBL_LAUNCH);
  }

  /* Wait until the flash is ready */
  while (FLASH->SR & FLASH_SR_BSY);

  flashBase = adr;
  flashSize = ((*((u32 *)FLASHSIZE_BASE)) & 0x0000FFFF) << 10;
  flashBankSize = flashSize >> 1;
  flashPageSize = GetFlashPageSize();

  return (0);
}

/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {

  FLASH->CR |= FLASH_CR_LOCK;                            /* Lock Flash operation */
  DSB();

#ifdef FLASH_OPT
  FLASH->CR  = FLASH_OBL_LAUNCH;                         /* Load option bytes */
  DSB();
  while (FLASH->CR & FLASH_OBL_LAUNCH);

  FLASH->CR = FLASH_OPTLOCK;                             /* Lock option bytes operation */
  DSB();
#endif /* FLASH_OPT */

  return (0);
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

  FLASH->SR  = FLASH_PGERR;                              /* Reset Error Flags */

  FLASH->CR  = (FLASH_CR_MER1 | FLASH_CR_MER2);          /* Bank A/B mass rrase enabled */
  FLASH->CR |=  FLASH_CR_STRT;                           /* Start erase */
  DSB();

  while (FLASH->SR & FLASH_SR_BSY);

  return (0);                                            /* Done */
}
#endif /* FLASH_MEM */

#ifdef FLASH_OPT
int EraseChip (void) {

  FLASH->SR  = FLASH_PGERR;                              /* Reset Error Flags */

  FLASH->OPTR     = 0xFFEFF8AA;
  FLASH->PCROP1SR = 0x000000FF;
  FLASH->PCROP1ER = 0x00000000;
  FLASH->WRP1AR   = 0x000000FF;
  FLASH->WRP1BR   = 0x000000FF;
  FLASH->SEC1R    = 0x00000000;
  if (GetFlashType() == 1U) {
  FLASH->PCROP1SR = 0x000000FF;
  FLASH->PCROP1ER = 0x00000000;
  FLASH->WRP1AR   = 0x000000FF;
  FLASH->WRP1BR   = 0x000000FF;
  FLASH->SEC1R    = 0x00000000;
  }

  FLASH->CR       = FLASH_OPTSTRT;                       /* Program values */
  DSB();

  while (FLASH->SR & FLASH_SR_BSY);

  if (FLASH->SR & FLASH_PGERR) {                         /* Check for Error */
    FLASH->SR  = FLASH_PGERR;                            /* Reset Error Flags */
    return (1);                                          /* Failed */
  }

  return (0);                                            /* Done */
}
#endif /* FLASH_OPT */


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

#if defined FLASH_MEM
int EraseSector (unsigned long adr) {
  u32 b, p;

  b = GetFlashBankNum(adr);                              /* Get Bank Number 0..1  */
  p = GetFlashPageNum(adr);                              /* Get Page Number 0..127 */
while (FLASH->SR & FLASH_SR_BSY){};
	 FLASH->SR  = FLASH_PGERR;      
FLASH->CR  = (FLASH_CR_PER |                           /* Page Erase Enabled */
                (p <<  3) |                              /* page Number. 0 to 255 for each bank */
                (b << 11)  );	                        /* Reset Error Flags */
  
	FLASH->CR |=  FLASH_CR_STRT;                           /* Start Erase */
  DSB();

  while (FLASH->SR & FLASH_SR_BSY);
 FLASH->CR &= (~FLASH_CR_PER);
 FLASH->CR &= (~FLASH_CR_BKER);
	
  if (FLASH->SR & FLASH_PGERR) {                         /* Check for Error */
    FLASH->SR  = FLASH_PGERR;                            /* Reset Error Flags */
    return (1);                                          /* Failed */
  }


  return (0);                                            /* Done */
}
#endif /* FLASH_MEM */

#if defined FLASH_OPT || defined FLASH_OTP
int EraseSector (unsigned long adr) {
  /* erase sector is not needed for
     - Flash Option bytes
     - Flash One Time Programmable bytes
  */
  return (0);                                            /* Done */
}
#endif /* FLASH_OPT || defined FLASH_OTP */


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

#if defined FLASH_MEM
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {

  sz = (sz + 7) & ~7;                                    /* Adjust size for two words */

  FLASH->SR  = FLASH_PGERR;                              /* Reset Error Flags */

  FLASH->CR = FLASH_CR_PG ;	                             /* Programming Enabled */

  while (sz) {
    M32(adr    ) = *((u32 *)(buf + 0));                  /* Program the first word of the Double Word */
    M32(adr + 4) = *((u32 *)(buf + 4));                  /* Program the second word of the Double Word */
    DSB();

    while (FLASH->SR & FLASH_SR_BSY);

    if (FLASH->SR & FLASH_PGERR) {                       /* Check for Error */
      FLASH->SR  = FLASH_PGERR;                          /* Reset Error Flags */
      return (1);                                        /* Failed */
    }

    adr += 8;                                            /* Go to next DoubleWord */
    buf += 8;
    sz  -= 8;
  }

  FLASH->CR = 0U;                                        /* Reset CR */

  return (0);
}
#endif /* FLASH_MEM */
