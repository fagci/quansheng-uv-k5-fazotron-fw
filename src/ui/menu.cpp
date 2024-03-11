#include "menu.hpp"
#include "../driver/st7565.hpp"
#include "../helpers/measurements.hpp"
#include "../printf.hpp"
#include "../radio.hpp"
#include "globals.hpp"
#include "graphics.hpp"
#include <string.h>

const uint8_t MENU_Y = 8;
const uint8_t MENU_ITEM_H = 11;
const uint8_t MENU_LINES_TO_SHOW = 4;

void UI_DrawScrollBar(const uint16_t size, const uint16_t iCurrent,
                      const uint8_t nLines) {
  const uint8_t sbY =
      ConvertDomain(iCurrent, 0, size, 0, nLines * MENU_ITEM_H - 3);

  DrawVLine(LCD_WIDTH - 2, MENU_Y, LCD_HEIGHT - MENU_Y, C_FILL);

  FillRect(LCD_WIDTH - 3, MENU_Y + sbY, 3, 3, C_FILL);
}

void UI_ShowMenuItem(uint8_t line, const char *name, bool isCurrent) {
  uint8_t by = MENU_Y + line * MENU_ITEM_H + 8;
  PrintMedium(4, by, name);
  if (isCurrent) {
    FillRect(0, MENU_Y + line * MENU_ITEM_H, LCD_WIDTH - 3, MENU_ITEM_H,
             C_INVERT);
  }
}

void UI_ShowMenuSimple(const MenuItem *menu, uint16_t size,
                       uint16_t currentIndex) {
  const uint16_t maxItems =
      size < MENU_LINES_TO_SHOW ? size : MENU_LINES_TO_SHOW;
  const uint16_t offset = Clamp(currentIndex - 2, 0, size - maxItems);
  char name[32] = "";

  for (uint16_t i = 0; i < maxItems; ++i) {
    uint16_t itemIndex = i + offset;
    strncpy(name, menu[itemIndex].name, 31);
    PrintSmallEx(LCD_WIDTH - 4, MENU_Y + i * MENU_ITEM_H + 8, POS_R, C_FILL,
                 "%u", itemIndex + 1);
    UI_ShowMenuItem(i, name, currentIndex == itemIndex);
  }

  UI_DrawScrollBar(size, currentIndex, MENU_LINES_TO_SHOW);
}

void UI_ShowMenu(void (*getItemText)(uint16_t index, char *name), uint16_t size,
                 uint16_t currentIndex) {
  const uint16_t maxItems =
      size < MENU_LINES_TO_SHOW ? size : MENU_LINES_TO_SHOW;
  const uint16_t offset = Clamp(currentIndex - 2, 0, size - maxItems);

  for (uint16_t i = 0; i < maxItems; ++i) {
    char name[32] = "";
    uint16_t itemIndex = i + offset;
    getItemText(itemIndex, name);
    PrintSmallEx(LCD_WIDTH - 4, MENU_Y + i * MENU_ITEM_H + 8, POS_R, C_FILL,
                 "%u", itemIndex + 1);
    UI_ShowMenuItem(i, name, currentIndex == itemIndex);
  }

  UI_DrawScrollBar(size, currentIndex, MENU_LINES_TO_SHOW);
}

void UI_ShowMenuEx(void (*showItem)(uint16_t i, uint16_t index, bool isCurrent),
                   uint16_t size, uint16_t currentIndex, uint16_t linesMax) {
  const uint16_t maxItems = size < linesMax ? size : linesMax;
  const uint16_t offset = Clamp(currentIndex - 2, 0, size - maxItems);

  for (uint16_t i = 0; i < maxItems; ++i) {
    uint16_t itemIndex = i + offset;
    showItem(i, itemIndex, currentIndex == itemIndex);
  }

  UI_DrawScrollBar(size, currentIndex, linesMax);
}

void GetMenuItemValue(BandCfgMenu type, char *Output) {
  CH band;
  CHANNELS_Load(radio->vfo.channel, &band);
  uint32_t fs = band.f;
  uint32_t fe = band.offset;
  switch (type) {
  case M_START:
    sprintf(Output, "%lu.%03lu", fs / 100000, fs / 100 % 1000);
    break;
  case M_END:
    sprintf(Output, "%lu.%03lu", fe / 100000, fe / 100 % 1000);
    break;
  case M_NAME:
    strncpy(Output, band.name, 31);
    break;
  case M_BW:
    strncpy(Output, BW_NAMES[band.bw], 31);
    break;
  case M_SQ_TYPE:
    strncpy(Output, SQ_TYPE_NAMES[band.sq.type], 31);
    break;
  case M_SQ:
    sprintf(Output, "%u", band.sq.level);
    break;
  case M_GAIN:
    sprintf(Output, "%ddB", gainTable[band.gainIndex].gainDb);
    break;
  case M_MODULATION:
    strncpy(Output, modulationTypeOptions[band.modulation], 31);
    break;
  case M_STEP:
    sprintf(Output, "%u.%02uKHz", StepFrequencyTable[band.step] / 100,
            StepFrequencyTable[band.step] % 100);
    break;
  default:
    break;
  }
}

void AcceptRadioConfig(const MenuItem *item, uint8_t subMenuIndex) {
  switch (item->type) {
  case M_BW:
    radio->bw = subMenuIndex;
    BK4819_SetFilterBandwidth(subMenuIndex);
            RADIO_Save
    BANDS_SaveCurrent();
    break;
  case M_F_TXP:
    radio->power = subMenuIndex;
    BANDS_SaveCurrent();
    break;
  case M_TX_OFFSET_DIR:
    radio->offsetDir = subMenuIndex;
    BANDS_SaveCurrent();
    break;
  case M_MODULATION:
    radio->modulation = subMenuIndex;
    BK4819_SetModulation(subMenuIndex);
    BANDS_SaveCurrent();
    break;
  case M_STEP:
    radio->vfo.step = subMenuIndex;
    BANDS_SaveCurrent();
    break;
  case M_SQ_TYPE:
    radio->sq.type = subMenuIndex;
    BK4819_SquelchType(subMenuIndex);
    BANDS_SaveCurrent();
    break;
  case M_SQ:
    radio->sq.level = subMenuIndex;
    BK4819_Squelch(subMenuIndex, radio->f, radio->sq.openTime,
                   radio->sq.closeTime);
    BANDS_SaveCurrent();
    break;

  case M_GAIN:
    radio->gainIndex = subMenuIndex;
    BK4819_SetGain(subMenuIndex);
    BANDS_SaveCurrent();
    break;
  default:
    break;
  }
}
