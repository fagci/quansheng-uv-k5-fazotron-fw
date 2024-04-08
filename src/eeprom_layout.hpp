#pragma once

struct CH {
  enum Type {
    CH_CHANNEL,
    CH_VFO,
    CH_BAND,
    CH_EMPTY = 255,
  };
  Type type;
  char *name[10];
  uint16_t groups;
  VFO vfo;
  static constexpr size_t size() { return sizeof(CH); };
} __attribute__((packed));

typedef struct MemoryCell {
  uint32_t number;
  CH ch;
} MemoryCell;

namespace eeprom_layout {
static int16_t channelsEndOffset() {
  return Settings::size() + Scanlist::size() * 16;
}

static uint32_t getChOffset(int16_t num) {
  return EEPROM::getSize() - (num + 1) * CH::size();
}

static uint32_t getChannelAddress(MemoryCell *data) {
  return getChOffset(data->number);
}

static int16_t getMaxChannels() {
  return (EEPROM::getSize() - channelsEndOffset()) / CH::size();
}
} // namespace eeprom_layout
