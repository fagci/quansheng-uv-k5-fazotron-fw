#pragma once

#include "../inc/dp32g030/gpio.h"
#include "../inc/dp32g030/portcon.h"
#include "../misc.hpp"
#include "abstractradio.hpp"
#include "bk4819-regs.hpp"
#include "gpio.hpp"
#include "system.hpp"
#include <stdint.h>

#define F_MIN 0
#define F_MAX 130000000

typedef enum {
  F_SC_T_0_2s,
  F_SC_T_0_4s,
  F_SC_T_0_8s,
  F_SC_T_1_6s,
} FreqScanTime;

typedef struct {
  uint16_t regValue;
  int8_t gainDb;
} Gain;

static const uint16_t FSK_RogerTable[7] = {
    0xF1A2, 0x7446, 0x61A4, 0x6544, 0x4E8A, 0xE044, 0xEA84,
};

const uint8_t DTMF_COEFFS[] = {111, 107, 103, 98, 80,  71,  58,  44,
                               65,  55,  37,  23, 228, 203, 181, 159};

const uint8_t SQ[2][6][11] = {
    {
        {0, 10, 62, 66, 74, 75, 92, 95, 98, 170, 252},
        {0, 5, 60, 64, 72, 70, 89, 92, 95, 166, 250},
        {255, 240, 56, 54, 48, 45, 32, 29, 20, 25, 20},
        {255, 250, 61, 58, 52, 48, 35, 32, 23, 30, 30},
        {255, 240, 135, 135, 116, 17, 3, 3, 2, 50, 50},
        {255, 250, 150, 140, 120, 20, 5, 5, 4, 45, 45},
    },
    {
        {0, 50, 78, 88, 94, 110, 114, 117, 119, 200, 252},
        {0, 40, 76, 86, 92, 106, 110, 113, 115, 195, 250},
        {255, 65, 49, 44, 42, 40, 33, 30, 22, 23, 22},
        {255, 70, 59, 54, 46, 45, 37, 34, 25, 27, 25},
        {255, 90, 135, 135, 116, 10, 8, 7, 6, 32, 32},
        {255, 100, 150, 140, 120, 15, 12, 11, 10, 30, 30},
    },
};

const uint16_t BWRegValues[3] = {0x3028, 0x4048, 0x0018};

const Gain gainTable[19] = {
    {0x000, -43}, //
    {0x100, -40}, //
    {0x020, -38}, //
    {0x200, -35}, //
    {0x040, -33}, //
    {0x220, -30}, //
    {0x060, -28}, //
    {0x240, -25}, //
    {0x0A0, -23}, //
    {0x260, -20}, //
    {0x1C0, -18}, //
    {0x2A0, -15}, //
    {0x2C0, -13}, //
    {0x2E0, -11}, //
    {0x360, -9},  //
    {0x380, -6},  //
    {0x3A0, -4},  //
    {0x3C0, -2},  //
    {0x3E0, 0},   //
};

class BK4819 : AbstractRadio {

public:
  static constexpr uint32_t F_MIN = 1600000;
  static constexpr uint32_t F_MAX = 134000000;

  typedef enum {
    MOD_FM,
    MOD_AM,
    MOD_USB,
    MOD_BYP,
    MOD_RAW,
    MOD_WFM,
  } ModulationType;

  typedef enum {
    SQUELCH_RSSI_NOISE_GLITCH,
    SQUELCH_RSSI_GLITCH,
    SQUELCH_RSSI_NOISE,
    SQUELCH_RSSI,
  } SquelchType;

  typedef enum AF_Type_t {
    AF_MUTE,
    AF_FM,
    AF_ALAM, // tone
    AF_BEEP, // for tx
    AF_RAW,  // (ssb without if filter = raw in sdr sharp)
    AF_USB,  // (or ssb = lsb and usb at the same time)
    AF_CTCO, // ctcss/dcs (fm with narrow filters for ctcss/dcs)
    AF_AM,
    AF_FSKO,   // fsk out test with special fsk filters, reg58 fsk on to give
               // sound on speaker
    AF_BYPASS, // (fm without filter = discriminator output)
  } AF_Type_t;

  typedef enum FilterBandwidth_t {
    FILTER_BW_WIDE,
    FILTER_BW_NARROW,
    FILTER_BW_NARROWER,
  } FilterBandwidth_t;

  typedef enum CssScanResult_t {
    CSS_RESULT_NOT_FOUND,
    CSS_RESULT_CTCSS,
    CSS_RESULT_CDCSS,
  } CssScanResult_t;

  void init() {
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);

    writeRegister(BK4819_REG_00, 0x8000);
    writeRegister(BK4819_REG_00, 0x0000);
    writeRegister(BK4819_REG_37, 0x1D0F);
    writeRegister(BK4819_REG_36, 0x0022);
    setAGC(true);
    writeRegister(BK4819_REG_19, 0x1041);
    writeRegister(BK4819_REG_7D, 0xE94F);
    writeRegister(BK4819_REG_48, 0xB3A8);

    for (uint8_t i = 0; i < ARRAY_SIZE(DTMF_COEFFS); ++i) {
      writeRegister(BK4819_REG_09, (i << 12) | DTMF_COEFFS[i]);
    }

    writeRegister(BK4819_REG_1F, 0x5454);
    writeRegister(BK4819_REG_3E, 0xA037);
    gBK4819_GpioOutState = 0x9000;
    writeRegister(BK4819_REG_33, 0x9000);
    writeRegister(BK4819_REG_3F, 0);
  }

  void setF(uint32_t f) {
    writeRegister(BK4819_REG_38, f & 0xFFFF);
    writeRegister(BK4819_REG_39, (f >> 16) & 0xFFFF);
  }

  uint32_t getF() {
    return (readRegister(BK4819_REG_39) << 16) | readRegister(BK4819_REG_38);
  }

  uint16_t getRSSI() { return readRegister(BK4819_REG_67) & 0x1FF; }

  uint8_t getNoise() { return readRegister(BK4819_REG_65) & 0x7F; }

  uint8_t getGlitch() { return readRegister(BK4819_REG_63) & 0xFF; }

  uint8_t getSNR() { return (readRegister(BK4819_REG_61) >> 8) & 0xFF; }

  bool isSquelchOpen() { return (readRegister(BK4819_REG_0C) >> 1) & 1; }

  void getVoxAmp(uint16_t *pResult) {
    *pResult = readRegister(BK4819_REG_64) & 0x7FFF;
  }

  void setAGC(bool useDefault) {
    // QS
    writeRegister(BK4819_REG_13, 0x03BE);
    writeRegister(BK4819_REG_12, 0x037B);
    writeRegister(BK4819_REG_11, 0x027B);
    writeRegister(BK4819_REG_10, 0x007A);

    uint8_t Lo = 0;    // 0-1 - auto, 2 - low, 3 high
    uint8_t low = 48;  // 1dB / LSB 56
    uint8_t high = 80; // 1dB / LSB 84

    if (useDefault) {
      writeRegister(BK4819_REG_14, 0x0019);
    } else {
      writeRegister(BK4819_REG_14, 0x0000);
      // slow 25 45
      // fast 15 50
      low = 15;
      high = 50;
    }
    writeRegister(BK4819_REG_49, (Lo << 14) | (high << 7) | (low << 0));
    writeRegister(BK4819_REG_7B, 0x8420);
  }

  void toggleGpioOut(BK4819_GPIO_PIN_t Pin, bool bSet) {
    if (bSet) {
      gBK4819_GpioOutState |= (0x40U >> Pin);
    } else {
      gBK4819_GpioOutState &= ~(0x40U >> Pin);
    }

    writeRegister(BK4819_REG_33, gBK4819_GpioOutState);
  }

  void enableVox(uint16_t VoxEnableThreshold, uint16_t VoxDisableThreshold) {
    // VOX Algorithm
    // if(voxamp>VoxEnableThreshold)       VOX = 1;
    // else if(voxamp<VoxDisableThreshold) (After Delay) VOX = 0;
    uint16_t REG_31_Value;

    REG_31_Value = readRegister(BK4819_REG_31);
    // 0xA000 is undocumented?
    writeRegister(BK4819_REG_46, 0xA000 | (VoxEnableThreshold & 0x07FF));
    // 0x1800 is undocumented?
    writeRegister(BK4819_REG_79, 0x1800 | (VoxDisableThreshold & 0x07FF));
    // Bottom 12 bits are undocumented, 15:12 vox disable delay *128ms
    writeRegister(BK4819_REG_7A,
                  0x289A); // vox disable delay = 128*5 = 640ms
    // Enable VOX
    writeRegister(BK4819_REG_31, REG_31_Value | 4); // bit 2 - VOX Enable
  }

  void setFilterBandwidth(FilterBandwidth_t Bandwidth) {
    writeRegister(BK4819_REG_43, BWRegValues[Bandwidth]);
  }

  void setupPowerAmplifier(uint8_t bias, uint32_t f) {
    const uint8_t gain = f < 28000000 ? 0x08U : 0x22U;
    writeRegister(BK4819_REG_36, (bias << 8) | 0x80U | gain);
  }

  void setupSquelch(uint8_t SquelchOpenRSSIThresh,
                    uint8_t SquelchCloseRSSIThresh,
                    uint8_t SquelchOpenNoiseThresh,
                    uint8_t SquelchCloseNoiseThresh,
                    uint8_t SquelchCloseGlitchThresh,
                    uint8_t SquelchOpenGlitchThresh, uint8_t OpenDelay,
                    uint8_t CloseDelay) {
    writeRegister(BK4819_REG_70, 0);
    writeRegister(BK4819_REG_4D, 0xA000 | SquelchCloseGlitchThresh);
    writeRegister(
        BK4819_REG_4E,
        (1u << 14) |                      //  1 ???
            (uint16_t)(OpenDelay << 11) | // *5  squelch = open  delay .. 0 ~ 7
            (uint16_t)(CloseDelay << 9) | // *3  squelch = close delay .. 0 ~ 3
            SquelchOpenGlitchThresh);
    writeRegister(BK4819_REG_4F,
                  (SquelchCloseNoiseThresh << 8) | SquelchOpenNoiseThresh);
    writeRegister(BK4819_REG_78,
                  (SquelchOpenRSSIThresh << 8) | SquelchCloseRSSIThresh);
    setAF(AF_MUTE);
    rX_TurnOn();

    // NOTE: check if it works to prevent muting output
    // setAF(modTypeCurrent);
  }

  void squelch(uint8_t sql, uint32_t f, uint8_t OpenDelay, uint8_t CloseDelay) {
    uint8_t band = f > VHF_UHF_BOUND2 ? 1 : 0; // TODO: use user defined bound?
    setupSquelch(SQ[band][0][sql], SQ[band][1][sql], SQ[band][2][sql],
                 SQ[band][3][sql], SQ[band][4][sql], SQ[band][5][sql],
                 OpenDelay, CloseDelay);
  }

  void squelchType(SquelchType t) {
    const RegisterSpec sqType = {"SQ type", BK4819_REG_77, 8, 0xFF, 1};
    const uint8_t squelchTypeValues[4] = {0x88, 0xAA, 0xCC, 0xFF};
    setRegValue(sqType, squelchTypeValues[t]);
  }

  void setAF(AF_Type_t AF) { writeRegister(BK4819_REG_47, 0x6040 | (AF << 8)); }

  void setModulation(ModulationType type) {
    if (modTypeCurrent == type) {
      return;
    }
    modTypeCurrent = type;
    const AF_Type_t modTypeReg47Values[] = {
        AF_FM, AF_AM, AF_USB, AF_BYPASS, AF_RAW, AF_FM,
    };
    setAF(modTypeReg47Values[type]);
    setRegValue(afDacGainRegSpec, 0xF);
    setAGC(type != MOD_AM);
    writeRegister(BK4819_REG_3D, type == MOD_USB ? 0 : 0x2AAB);
    setRegValue(afcDisableRegSpec,
                type == MOD_AM || type == MOD_USB || type == MOD_BYP);
  }

  void rX_TurnOn() {
    writeRegister(BK4819_REG_37, 0x1F0F);
    writeRegister(BK4819_REG_30, 0);
    writeRegister(BK4819_REG_30, 0xBFF1);
  }

  void disableFilter() {
    toggleGpioOut(BK4819_GPIO4_PIN32_VHF_LNA, false);
    toggleGpioOut(BK4819_GPIO3_PIN31_UHF_LNA, false);
  }

  void disableVox() {
    uint16_t Value;

    Value = readRegister(BK4819_REG_31);
    writeRegister(BK4819_REG_31, Value & 0xFFFB);
  }

  void playTone(uint16_t Frequency, bool bTuningGainSwitch) {
    uint16_t ToneConfig;

    enterTxMute();
    setAF(AF_BEEP);

    if (bTuningGainSwitch == 0) {
      ToneConfig = 0 | BK4819_REG_70_ENABLE_TONE1 |
                   (96U << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN);
    } else {
      ToneConfig = 0 | BK4819_REG_70_ENABLE_TONE1 |
                   (28U << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN);
    }
    writeRegister(BK4819_REG_70, ToneConfig);

    writeRegister(BK4819_REG_30, 0);
    writeRegister(BK4819_REG_30, 0 | BK4819_REG_30_ENABLE_AF_DAC |
                                     BK4819_REG_30_ENABLE_DISC_MODE |
                                     BK4819_REG_30_ENABLE_TX_DSP);

    setToneFrequency(Frequency);
  }

  void enterTxMute() { writeRegister(BK4819_REG_50, 0xBB20); }

  void exitTxMute() { writeRegister(BK4819_REG_50, 0x3B20); }

  void sleep() {
    writeRegister(BK4819_REG_30, 0);
    writeRegister(BK4819_REG_37, 0x1D00);
  }

  void turnsOffTones_TurnsOnRX() {
    writeRegister(BK4819_REG_70, 0);
    setAF(AF_MUTE);
    exitTxMute();
    writeRegister(BK4819_REG_30, 0);
    writeRegister(
        BK4819_REG_30,
        0 | BK4819_REG_30_ENABLE_VCO_CALIB | BK4819_REG_30_ENABLE_RX_LINK |
            BK4819_REG_30_ENABLE_AF_DAC | BK4819_REG_30_ENABLE_DISC_MODE |
            BK4819_REG_30_ENABLE_PLL_VCO | BK4819_REG_30_ENABLE_RX_DSP);
  }

  void idle() { writeRegister(BK4819_REG_30, 0x0000); }

  void exitBypass() {
    setAF(AF_MUTE);
    writeRegister(BK4819_REG_7E, 0x302E);
  }

  void prepareTransmit() {
    exitBypass();
    exitTxMute();
    txOn_Beep();
  }

  void txOn_Beep() {
    writeRegister(BK4819_REG_37, 0x1D0F);
    writeRegister(BK4819_REG_52, 0x028F);
    writeRegister(BK4819_REG_30, 0x0000);
    writeRegister(BK4819_REG_30, 0xC1FE);
  }

  void exitSubAu() { writeRegister(BK4819_REG_51, 0x0000); }

  void enableRX() {
    toggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, true);
    rX_TurnOn();
  }

  void enableTXLink() {
    writeRegister(
        BK4819_REG_30,
        0 | BK4819_REG_30_ENABLE_VCO_CALIB | BK4819_REG_30_ENABLE_UNKNOWN |
            BK4819_REG_30_DISABLE_RX_LINK | BK4819_REG_30_ENABLE_AF_DAC |
            BK4819_REG_30_ENABLE_DISC_MODE | BK4819_REG_30_ENABLE_PLL_VCO |
            BK4819_REG_30_ENABLE_PA_GAIN | BK4819_REG_30_DISABLE_MIC_ADC |
            BK4819_REG_30_ENABLE_TX_DSP | BK4819_REG_30_DISABLE_RX_DSP);
  }

  void transmitTone(bool bLocalLoopback, uint32_t Frequency) {
    enterTxMute();
    writeRegister(BK4819_REG_70,
                  BK4819_REG_70_MASK_ENABLE_TONE1 |
                      (96U << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN));
    setToneFrequency(Frequency);

    setAF(bLocalLoopback ? AF_BEEP : AF_MUTE);

    enableTXLink();
    SYSTEM_DelayMs(50);
    exitTxMute();
  }

  void playRoger() {
    enterTxMute();
    setAF(AF_MUTE);
    writeRegister(BK4819_REG_70, 0xE000);
    enableTXLink();
    SYSTEM_DelayMs(50);
    writeRegister(BK4819_REG_71, 0x142A);
    exitTxMute();
    SYSTEM_DelayMs(80);
    enterTxMute();
    writeRegister(BK4819_REG_71, 0x1C3B);
    exitTxMute();
    SYSTEM_DelayMs(80);
    enterTxMute();
    writeRegister(BK4819_REG_70, 0x0000);
    writeRegister(BK4819_REG_30, 0xC1FE);
  }

  void playRogerMDC() {
    uint8_t i;

    setAF(AF_MUTE);
    writeRegister(
        BK4819_REG_58,
        0x37C3); // FSK Enable, RX Bandwidth FFSK1200/1800, 0xAA or 0x55
                 // Preamble, 11 RX Gain, 101 RX Mode, FFSK1200/1800 TX
    writeRegister(BK4819_REG_72, 0x3065); // Set Tone2 to 1200Hz
    writeRegister(BK4819_REG_70,
                  0x00E0); // Enable Tone2 and Set Tone2 Gain
    writeRegister(BK4819_REG_5D,
                  0x0D00); // Set FSK data length to 13 bytes
    writeRegister(BK4819_REG_59,
                  0x8068); // 4 byte sync length, 6 byte preamble, clear TX FIFO
    writeRegister(
        BK4819_REG_59,
        0x0068); // Same, but clear TX FIFO is now unset (clearing done)
    writeRegister(BK4819_REG_5A, 0x5555); // First two sync bytes
    writeRegister(BK4819_REG_5B,
                  0x55AA); // End of sync bytes. Total 4 bytes: 555555aa
    writeRegister(BK4819_REG_5C, 0xAA30); // Disable CRC
    for (i = 0; i < 7; i++) {
      writeRegister(BK4819_REG_5F,
                    FSK_RogerTable[i]); // Send the data from the roger table
    }
    SYSTEM_DelayMs(20);
    writeRegister(BK4819_REG_59,
                  0x0868); // 4 sync bytes, 6 byte preamble, Enable FSK TX
    SYSTEM_DelayMs(180);
    // Stop FSK TX, reset Tone2, disable FSK.
    writeRegister(BK4819_REG_59, 0x0068);
    writeRegister(BK4819_REG_70, 0x0000);
    writeRegister(BK4819_REG_58, 0x0000);
  }

  void enable_AfDac_DiscMode_TxDsp() {
    writeRegister(BK4819_REG_30, 0x0000);
    writeRegister(BK4819_REG_30, 0x0302);
  }

  void playDTMFEx(bool bLocalLoopback, char Code) {
    enableDTMF();
    enterTxMute();
    setAF(bLocalLoopback ? AF_BEEP : AF_MUTE);
    writeRegister(BK4819_REG_70, 0xD3D3);
    enableTXLink();
    SYSTEM_DelayMs(50);
    playDTMF(Code);
    exitTxMute();
  }

  void toggleAFBit(bool on) {
    uint16_t reg = readRegister(BK4819_REG_47);
    reg &= ~(1 << 8);
    if (on)
      reg |= 1 << 8;
    writeRegister(BK4819_REG_47, reg);
  }

  void toggleAFDAC(bool on) {
    uint16_t Reg = readRegister(BK4819_REG_30);
    Reg &= ~BK4819_REG_30_ENABLE_AF_DAC;
    if (on)
      Reg |= BK4819_REG_30_ENABLE_AF_DAC;
    writeRegister(BK4819_REG_30, Reg);
  }

  void tuneTo(uint32_t f, bool precise) {
    setF(f);
    uint16_t reg = readRegister(BK4819_REG_30);
    if (precise) {
      writeRegister(BK4819_REG_30, 0);
    } else {
      writeRegister(BK4819_REG_30, reg & ~BK4819_REG_30_ENABLE_VCO_CALIB);
    }
    writeRegister(BK4819_REG_30, reg);
  }

  void resetRSSI() {
    uint16_t Reg = readRegister(BK4819_REG_30);
    Reg &= ~1;
    writeRegister(BK4819_REG_30, Reg);
    Reg |= 1;
    writeRegister(BK4819_REG_30, Reg);
  }

  void setGain(uint8_t gainIndex) {
    writeRegister(BK4819_REG_13, gainTable[gainIndex].regValue | 6 | (3 << 3));
  }

  /// DTMF FUNCTIONS

  void disableDTMF() { writeRegister(BK4819_REG_24, 0); }

  void enableDTMF() {
    writeRegister(BK4819_REG_21, 0x06D8);
    writeRegister(BK4819_REG_24, 0 | (1U << BK4819_REG_24_SHIFT_UNKNOWN_15) |
                                     (24 << BK4819_REG_24_SHIFT_THRESHOLD) |
                                     (1U << BK4819_REG_24_SHIFT_UNKNOWN_6) |
                                     BK4819_REG_24_ENABLE |
                                     BK4819_REG_24_SELECT_DTMF |
                                     (14U << BK4819_REG_24_SHIFT_MAX_SYMBOLS));
  }

  uint8_t getDTMF_5TONE_Code() {
    return (readRegister(BK4819_REG_0B) >> 8) & 0x0F;
  }

  void enterDTMF_TX(bool bLocalLoopback) {
    enableDTMF();
    enterTxMute();
    if (bLocalLoopback) {
      setAF(AF_BEEP);
    } else {
      setAF(AF_MUTE);
    }
    writeRegister(BK4819_REG_70,
                  0 | BK4819_REG_70_MASK_ENABLE_TONE1 |
                      (83 << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN) |
                      BK4819_REG_70_MASK_ENABLE_TONE2 |
                      (83 << BK4819_REG_70_SHIFT_TONE2_TUNING_GAIN));

    enableTXLink();
  }

  void exitDTMF_TX(bool bKeep) {
    enterTxMute();
    setAF(AF_MUTE);
    writeRegister(BK4819_REG_70, 0x0000);
    disableDTMF();
    writeRegister(BK4819_REG_30, 0xC1FE);
    if (!bKeep) {
      exitTxMute();
    }
  }

  void playDTMF(char Code) {
    switch (Code) {
    case '0':
      writeRegister(BK4819_REG_71, 0x25F3);
      writeRegister(BK4819_REG_72, 0x35E1);
      break;
    case '1':
      writeRegister(BK4819_REG_71, 0x1C1C);
      writeRegister(BK4819_REG_72, 0x30C2);
      break;
    case '2':
      writeRegister(BK4819_REG_71, 0x1C1C);
      writeRegister(BK4819_REG_72, 0x35E1);
      break;
    case '3':
      writeRegister(BK4819_REG_71, 0x1C1C);
      writeRegister(BK4819_REG_72, 0x3B91);
      break;
    case '4':
      writeRegister(BK4819_REG_71, 0x1F0E);
      writeRegister(BK4819_REG_72, 0x30C2);
      break;
    case '5':
      writeRegister(BK4819_REG_71, 0x1F0E);
      writeRegister(BK4819_REG_72, 0x35E1);
      break;
    case '6':
      writeRegister(BK4819_REG_71, 0x1F0E);
      writeRegister(BK4819_REG_72, 0x3B91);
      break;
    case '7':
      writeRegister(BK4819_REG_71, 0x225C);
      writeRegister(BK4819_REG_72, 0x30C2);
      break;
    case '8':
      writeRegister(BK4819_REG_71, 0x225c);
      writeRegister(BK4819_REG_72, 0x35E1);
      break;
    case '9':
      writeRegister(BK4819_REG_71, 0x225C);
      writeRegister(BK4819_REG_72, 0x3B91);
      break;
    case 'A':
      writeRegister(BK4819_REG_71, 0x1C1C);
      writeRegister(BK4819_REG_72, 0x41DC);
      break;
    case 'B':
      writeRegister(BK4819_REG_71, 0x1F0E);
      writeRegister(BK4819_REG_72, 0x41DC);
      break;
    case 'C':
      writeRegister(BK4819_REG_71, 0x225C);
      writeRegister(BK4819_REG_72, 0x41DC);
      break;
    case 'D':
      writeRegister(BK4819_REG_71, 0x25F3);
      writeRegister(BK4819_REG_72, 0x41DC);
      break;
    case '*':
      writeRegister(BK4819_REG_71, 0x25F3);
      writeRegister(BK4819_REG_72, 0x30C2);
      break;
    case '#':
      writeRegister(BK4819_REG_71, 0x25F3);
      writeRegister(BK4819_REG_72, 0x3B91);
      break;
    }
  }

  void playDTMFString(const char *pString, bool bDelayFirst,
                      uint16_t FirstCodePersistTime,
                      uint16_t HashCodePersistTime, uint16_t CodePersistTime,
                      uint16_t CodeInternalTime) {
    uint8_t i;
    uint16_t Delay;

    for (i = 0; pString[i]; i++) {
      playDTMF(pString[i]);
      exitTxMute();
      if (bDelayFirst && i == 0) {
        Delay = FirstCodePersistTime;
      } else if (pString[i] == '*' || pString[i] == '#') {
        Delay = HashCodePersistTime;
      } else {
        Delay = CodePersistTime;
      }
      SYSTEM_DelayMs(Delay);
      enterTxMute();
      SYSTEM_DelayMs(CodeInternalTime);
    }
  }

  /// SCRAMBLER FUNCTIONS

  void disableScramble() {
    uint16_t Value;

    Value = readRegister(BK4819_REG_31);
    writeRegister(BK4819_REG_31, Value & 0xFFFD);
  }

  void enableScramble(uint8_t Type) {
    uint16_t Value;

    Value = readRegister(BK4819_REG_31);
    writeRegister(BK4819_REG_31, Value | 2);
    writeRegister(BK4819_REG_71, (Type * 0x0408) + 0x68DC);
  }

  /// TAIL TONES FUNCTIONS

  void setToneFrequency(uint16_t f) {
    writeRegister(BK4819_REG_71, (f * 103U) / 10U);
  }

  void setTailDetection(const uint32_t freq_10Hz) {
    writeRegister(BK4819_REG_07,
                  BK4819_REG_07_MODE_CTC2 | ((253910 + (freq_10Hz / 2)) /
                                             freq_10Hz)); // with rounding
  }

  void genTail(uint8_t Tail) {
    switch (Tail) {
    case 0: // 134.4Hz CTCSS Tail
      writeRegister(BK4819_REG_52, 0x828F);
      break;
    case 1: // 120° phase shift
      writeRegister(BK4819_REG_52, 0xA28F);
      break;
    case 2: // 180° phase shift
      writeRegister(BK4819_REG_52, 0xC28F);
      break;
    case 3: // 240° phase shift
      writeRegister(BK4819_REG_52, 0xE28F);
      break;
    case 4: // 55Hz tone freq NOTE: REG_07
      writeRegister(BK4819_REG_07, 0x046f);
      break;
    }
  }

  /// CTCSS / DCS FUNCTIONS

  void enableCDCSS() {
    genTail(0); // CTC134
    writeRegister(BK4819_REG_51, 0x804A);
  }

  void enableCTCSS() {
    // genTail(2); // CTC180
    genTail(4); // CTC55
    writeRegister(BK4819_REG_51, 0x904A);
  }

  CssScanResult_t getCxCSSScanResult(uint32_t *pCdcssFreq,
                                     uint16_t *pCtcssFreq) {
    uint16_t Low;
    uint16_t High = readRegister(BK4819_REG_69);

    if ((High & 0x8000) == 0) {
      Low = readRegister(BK4819_REG_6A);
      *pCdcssFreq = ((High & 0xFFF) << 12) | (Low & 0xFFF);
      return CSS_RESULT_CDCSS;
    }

    Low = readRegister(BK4819_REG_68);

    if ((Low & 0x8000) == 0) {
      *pCtcssFreq = ((Low & 0x1FFF) * 4843) / 10000;
      return CSS_RESULT_CTCSS;
    }

    return CSS_RESULT_NOT_FOUND;
  }

  uint8_t getCDCSSCodeType() { return (readRegister(BK4819_REG_0C) >> 14) & 3; }

  uint8_t getCTCType() { return (readRegister(BK4819_REG_0C) >> 10) & 3; }

  void setCDCSSCodeWord(uint32_t CodeWord) {
    // Enable CDCSS
    // Transmit positive CDCSS code
    // CDCSS Mode
    // CDCSS 23bit
    // Enable Auto CDCSS Bw Mode
    // Enable Auto CTCSS Bw Mode
    // CTCSS/CDCSS Tx Gain1 Tuning = 51
    writeRegister(
        BK4819_REG_51,
        0 | BK4819_REG_51_ENABLE_CxCSS | BK4819_REG_51_GPIO6_PIN2_NORMAL |
            BK4819_REG_51_TX_CDCSS_POSITIVE | BK4819_REG_51_MODE_CDCSS |
            BK4819_REG_51_CDCSS_23_BIT | BK4819_REG_51_1050HZ_NO_DETECTION |
            BK4819_REG_51_AUTO_CDCSS_BW_ENABLE |
            BK4819_REG_51_AUTO_CTCSS_BW_ENABLE |
            (51U << BK4819_REG_51_SHIFT_CxCSS_TX_GAIN1));

    // CTC1 Frequency Control Word = 2775
    writeRegister(BK4819_REG_07, 0 | BK4819_REG_07_MODE_CTC1 |
                                     (2775U << BK4819_REG_07_SHIFT_FREQUENCY));

    // Set the code word
    writeRegister(BK4819_REG_08, 0x0000 | ((CodeWord >> 0) & 0xFFF));
    writeRegister(BK4819_REG_08, 0x8000 | ((CodeWord >> 12) & 0xFFF));
  }

  void setCTCSSFrequency(uint32_t FreqControlWord) {
    uint16_t Config;

    if (FreqControlWord == 2625) { // Enables 1050Hz detection mode
      // Enable TxCTCSS
      // CTCSS Mode
      // 1050/4 Detect Enable
      // Enable Auto CDCSS Bw Mode
      // Enable Auto CTCSS Bw Mode
      // CTCSS/CDCSS Tx Gain1 Tuning = 74
      Config = 0x944A;
    } else {
      // Enable TxCTCSS
      // CTCSS Mode
      // Enable Auto CDCSS Bw Mode
      // Enable Auto CTCSS Bw Mode
      // CTCSS/CDCSS Tx Gain1 Tuning = 74
      Config = 0x904A;
    }
    writeRegister(BK4819_REG_51, Config);
    // CTC1 Frequency Control Word
    writeRegister(BK4819_REG_07, 0 | BK4819_REG_07_MODE_CTC1 |
                                     ((FreqControlWord * 2065) / 1000)
                                         << BK4819_REG_07_SHIFT_FREQUENCY);
  }

  /// FREQUENCY SCAN FUNCTIONS

  bool getFrequencyScanResult(uint32_t *pFrequency) {
    uint16_t High = readRegister(BK4819_REG_0D);
    bool Finished = (High & 0x8000) == 0;

    if (Finished) {
      uint16_t Low = readRegister(BK4819_REG_0E);
      *pFrequency = (uint32_t)((High & 0x7FF) << 16) | Low;
    }

    return Finished;
  }

  void disableFrequencyScan() { writeRegister(BK4819_REG_32, 0x0244); }

  void enableFrequencyScan() { writeRegister(BK4819_REG_32, 0x0245); }

  void enableFrequencyScanEx(FreqScanTime t) {
    writeRegister(BK4819_REG_32, 0x0245 | (t << 14));
  }

  void setScanFrequency(uint32_t Frequency) {
    setF(Frequency);
    writeRegister(
        BK4819_REG_51,
        0 | BK4819_REG_51_DISABLE_CxCSS | BK4819_REG_51_GPIO6_PIN2_NORMAL |
            BK4819_REG_51_TX_CDCSS_POSITIVE | BK4819_REG_51_MODE_CDCSS |
            BK4819_REG_51_CDCSS_23_BIT | BK4819_REG_51_1050HZ_NO_DETECTION |
            BK4819_REG_51_AUTO_CDCSS_BW_DISABLE |
            BK4819_REG_51_AUTO_CTCSS_BW_DISABLE);
    rX_TurnOn();
  }

  void stopScan() {
    disableFrequencyScan();
    idle();
  }

  /// FSK FUNCTIONS

  void resetFSK() {
    writeRegister(BK4819_REG_3F, 0x0000); // Disable interrupts
    writeRegister(BK4819_REG_59,
                  0x0068); // Sync length 4 bytes, 7 byte preamble
    SYSTEM_DelayMs(30);
    idle();
  }

  void fskClearFifo() {
    const uint16_t fsk_reg59 = readRegister(BK4819_REG_59);
    writeRegister(BK4819_REG_59, (1u << 15) | (1u << 14) | fsk_reg59);
  }

  void fskEnableRx() {
    const uint16_t fsk_reg59 = readRegister(BK4819_REG_59);
    writeRegister(BK4819_REG_59, (1u << 12) | fsk_reg59);
  }

  void fskEnableTx() {
    const uint16_t fsk_reg59 = readRegister(BK4819_REG_59);
    writeRegister(BK4819_REG_59, (1u << 11) | fsk_reg59);
  }

  /// REGISTERS SPECS FUNCTIONS

  uint16_t getRegValue(RegisterSpec s) {
    return (readRegister(s.num) >> s.offset) & s.mask;
  }

  void setRegValue(RegisterSpec s, uint16_t v) {
    uint16_t reg = readRegister(s.num);
    reg &= ~(s.mask << s.offset);
    writeRegister(s.num, reg | (v << s.offset));
  }

private:
  void writeU8(uint8_t Data) {
    uint8_t i;

    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
    for (i = 0; i < 8; i++) {
      if ((Data & 0x80U) == 0) {
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
      } else {
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
      }
      SYSTICK_DelayUs(1);
      GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
      SYSTICK_DelayUs(1);
      Data <<= 1;
      GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
      SYSTICK_DelayUs(1);
    }
  }

  static uint16_t readU16() {
    uint8_t i;
    uint16_t Value;

    PORTCON_PORTC_IE = (PORTCON_PORTC_IE & ~PORTCON_PORTC_IE_C2_MASK) |
                       PORTCON_PORTC_IE_C2_BITS_ENABLE;
    GPIOC->DIR = (GPIOC->DIR & ~GPIO_DIR_2_MASK) | GPIO_DIR_2_BITS_INPUT;
    SYSTICK_DelayUs(1);

    Value = 0;
    for (i = 0; i < 16; i++) {
      Value <<= 1;
      Value |= GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
      GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
      SYSTICK_DelayUs(1);
      GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
      SYSTICK_DelayUs(1);
    }
    PORTCON_PORTC_IE = (PORTCON_PORTC_IE & ~PORTCON_PORTC_IE_C2_MASK) |
                       PORTCON_PORTC_IE_C2_BITS_DISABLE;
    GPIOC->DIR = (GPIOC->DIR & ~GPIO_DIR_2_MASK) | GPIO_DIR_2_BITS_OUTPUT;

    return Value;
  }

  void writeU16(uint16_t Data) {
    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
    for (uint8_t i = 0; i < 16; i++) {
      if ((Data & 0x8000U) == 0U) {
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
      } else {
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
      }
      SYSTICK_DelayUs(1);
      GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
      Data <<= 1;
      SYSTICK_DelayUs(1);
      GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
      SYSTICK_DelayUs(1);
    }
  }

  uint16_t readRegister(BK4819_REGISTER_t Register) {
    uint16_t Value;

    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
    SYSTICK_DelayUs(1);
    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);

    writeU8(Register | 0x80);

    Value = readU16();

    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
    SYSTICK_DelayUs(1);
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);

    return Value;
  }

  void writeRegister(BK4819_REGISTER_t Register, uint16_t Data) {
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
    SYSTICK_DelayUs(1);
    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
    writeU8(Register);
    SYSTICK_DelayUs(1);
    writeU16(Data);
    SYSTICK_DelayUs(1);
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
    SYSTICK_DelayUs(1);
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
  }

  uint16_t gBK4819_GpioOutState;
  uint8_t modTypeCurrent = 255;
};
