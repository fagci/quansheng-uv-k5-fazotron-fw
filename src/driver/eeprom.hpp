#pragma once

#include "../external/CMSIS_5/Device/ARM/ARMCM0/Include/ARMCM0.h"
#include "i2c.hpp"
#include "system.hpp"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

class EEPROM {

public:
  typedef enum {
    EEPROM_BL24C64,   //
    EEPROM_BL24C128,  //
    EEPROM_BL24C256,  //
    EEPROM_BL24C512,  //
    EEPROM_BL24C1024, //
    EEPROM_M24M02,    //
  } Type;

  constexpr static const char *EEPROM_TYPE_NAMES[8] = {
      "BL24C64 (default)", //
      "BL24C128",          //
      "BL24C256",          //
      "BL24C512",          //
      "BL24C1024",         //
      "M24M02 (x1)",       //
  };

  constexpr static uint32_t SIZES[8] = {
      8192,   //
      16384,  //
      32768,  //
      65536,  //
      131072, //
      262144, //
  };

  constexpr static uint8_t PAGE_SIZES[8] = {
      32,  //
      32,  //
      32,  //
      32,  //
      32,  //
      128, //
  };

  static void setType(Type t) { eepromType = t; }

  static void readBuffer(uint32_t address, void *pBuffer, uint8_t size) {
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
  static void writeBuffer(uint32_t address, void *pBuffer, uint8_t size) {
    if (pBuffer == NULL) {
      return;
    }
    const uint8_t PAGE_SIZE = PAGE_SIZES[eepromType];

    while (size) {
      uint32_t pageNum = address / PAGE_SIZE;
      uint32_t rest = (pageNum + 1) * PAGE_SIZE - address;

      // TODO: assume that size < PAGE_SIZE
      uint8_t n = rest > size ? size : (uint8_t)rest;

      /* readBuffer(address, tmpBuffer, n);
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
      delayMs(8);

      pBuffer += n;
      address += n;
      size -= n;
      // }
      gEepromWrite = true;
    }
  }

  static uint32_t getSize() { return SIZES[eepromType]; }

  static uint8_t getPageSize() { return PAGE_SIZES[eepromType]; }

private:
  static bool gEepromWrite;
  static bool gEepromRead;
  static inline Type eepromType = EEPROM_BL24C64;
};
