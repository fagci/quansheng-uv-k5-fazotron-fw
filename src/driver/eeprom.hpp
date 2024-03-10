#ifndef DRIVER_EEPROM_H
#define DRIVER_EEPROM_H

#include <stdint.h>

extern bool gEepromRead;
extern bool gEepromWrite;

typedef enum {
  EEPROM_A,         // 000
  EEPROM_B,         // 001
  EEPROM_BL24C64,   // 010 checkbyte default
  EEPROM_BL24C128,  // 011
  EEPROM_BL24C256,  // 100
  EEPROM_BL24C512,  // 101
  EEPROM_BL24C1024, // 110
  EEPROM_M24M02,    // 111
} EEPROMType;

void EEPROM_ReadBuffer(uint32_t Address, void *pBuffer, uint8_t Size);
void EEPROM_WriteBuffer(uint32_t Address, void *pBuffer, uint8_t Size);

#endif
