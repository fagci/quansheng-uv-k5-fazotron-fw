#include "eeprom.hpp"
#include "../external/CMSIS_5/Device/ARM/ARMCM0/Include/ARMCM0.h"
#include "i2c.hpp"
#include "system.hpp"
#include <stddef.h>
#include <string.h>

bool gEepromWrite = false;
bool gEepromRead = false;
static EEPROMType eepromType = EEPROM_BL24C64;
static const uint32_t EEPROM_SIZES[8] = {
    8192,   // 000
    8192,   // 001
    8192,   // 010
    16384,  // 011
    32768,  // 100
    65536,  // 101
    131072, // 110
    262144, // 111
};

static const uint8_t EEPROM_PAGE_SIZES[8] = {
    32,  // 000
    32,  // 001
    32,  // 010
    32,  // 011
    32,  // 100
    32,  // 101
    32,  // 110
    128, // 111
};

void EEPROM_Init(EEPROMType t) { eepromType = t; }

void EEPROM_ReadBuffer(uint32_t address, void *pBuffer, uint8_t size) {
  uint8_t IIC_ADD = (uint8_t)(0xA0 | ((address / 0x10000) << 1));

  if (eepromType == EEPROM_M24M02) {
    if (address >= 0x40000) {
      IIC_ADD = (uint8_t)(0xA8 | (((address - 0x40000) / 0x10000) << 1));
      address -= 0x40000;
    }
  }

  __disable_irq();
  I2C_Start();

  I2C_Write(IIC_ADD);
  I2C_Write((address >> 8) & 0xFF);
  I2C_Write(address & 0xFF);

  I2C_Start();

  I2C_Write(IIC_ADD + 1);

  I2C_ReadBuffer(pBuffer, size);

  I2C_Stop();
  __enable_irq();

  gEepromRead = true;
}

// static uint8_t tmpBuffer[256];
void EEPROM_WriteBuffer(uint32_t address, void *pBuffer, uint8_t size) {
  if (pBuffer == NULL) {
    return;
  }
  const uint8_t PAGE_SIZE = EEPROM_PAGE_SIZES[eepromType];

  while (size) {
    uint32_t pageNum = address / PAGE_SIZE;
    uint32_t rest = (pageNum + 1) * PAGE_SIZE - address;

    // TODO: assume that size < PAGE_SIZE
    uint8_t n = rest > size ? size : (uint8_t)rest;

    /* EEPROM_ReadBuffer(address, tmpBuffer, n);
    if (memcmp(buf, tmpBuffer, n) != 0) { */
    uint8_t IIC_ADD = (uint8_t)(0xA0 | ((address / 0x10000) << 1));

    if (eepromType == EEPROM_M24M02) {
      if (address >= 0x40000) {
        IIC_ADD = (uint8_t)(0xA8 | (((address - 0x40000) / 0x10000) << 1));
      }
    }

    __disable_irq();
    I2C_Start();
    I2C_Write(IIC_ADD);
    I2C_Write((address >> 8) & 0xFF);
    I2C_Write(address & 0xFF);

    I2C_WriteBuffer(pBuffer, n);

    I2C_Stop();
    __enable_irq();
    SYSTEM_DelayMs(8);

    pBuffer += n;
    address += n;
    size -= n;
    // }
    gEepromWrite = true;
  }
}
