#pragma once

#include "../radio.hpp"
#include "channel.hpp"

class TuneService {
public:
  TuneService(Radio *r) : radio{r} {}

  void tuneTo(uint32_t f) {
    radio->setF(f);
    Radio::Filter filterNeeded = filterByF(f);
    radio->selectFilter(filterNeeded);
  }
  void tuneTo(ChannelService::CH ch) {}

private:
  Radio *radio;
  Radio::Filter filterByF(uint32_t f) {
    return f < settingsService.filterBound ? Radio::FILTER_VHF
                                           : Radio::FILTER_UHF;
  }
};
