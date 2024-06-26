#pragma once

#include "../board.hpp"
#include "../scheduler.hpp"
#include "channel.hpp"
#include "settings.hpp"

namespace svc::tune {
constexpr static uint32_t upConverterValues[3] = {0, 5000000, 12500000};

Radio::Filter filterByF(uint32_t f) {
  return f < Board::settings.getFilterBound() ? Radio::FILTER_VHF
                                              : Radio::FILTER_UHF;
}

uint32_t GetScreenF(uint32_t f) {
  return f - upConverterValues[Board::settings.upconverter];
}

uint32_t GetTuneF(uint32_t f) {
  return f + upConverterValues[Board::settings.upconverter];
}


// USE CASE: set vfo temporary for current app
void tuneTo(uint32_t f) {
  Board::radio.setF(f);
  Radio::Filter filterNeeded = filterByF(f);
  Board::radio.selectFilter(filterNeeded);
}

// USE CASE: set vfo and use in another app
void tuneToSave(uint32_t f) {
  tuneTo(f);
  saveCurrentCH();
}

void nextVFO() {
  settings->activeCH = !gSettings.activeCH;
  radio = &gCH[settings->activeCH];
  gCurrentLoot = &gLoot[settings->activeCH];
  setupByCurrentCH();
  toggleRX(false);
  SETTINGS_Save();
}

void toggleVfoMR() {
  if (vfo.vfo.channel >= 0) {
    vfo.vfo.channel = -1;
  } else {
    nextCH(true);
  }
  saveCurrentCH();
}

bool tuneToCH(int16_t num) {
  if (CHANNELS_Existing(num)) {
    CHANNELS_Load(num, radio);
    vfo.vfo.channel = num;
    onVfoUpdate();
    setupByCurrentCH();
    return true;
  }
  return false;
}

void nextCH(bool next) {
  int16_t i;
  if (vfo.vfo.channel >= 0) {
    i = CHANNELS_Next(vfo.vfo.channel, next);
    if (i > -1) {
      vfo.vfo.channel = i;
      vfoLoadCH(settings->activeCH);
    }
    onVfoUpdate();
    setupByCurrentCH();
    return;
  }

  i = vfo.vfo.channel;

  if (!CHANNELS_Existing(vfo.vfo.channel)) {
    i = CHANNELS_Next(vfo.vfo.channel, true);
    if (i == -1) {
      return;
    }
  }

  tuneToCH(i);
}

// TODO: бесшовное
void nextFreq(bool next) {
  int8_t dir = next ? 1 : -1;

  if (vfo.vfo.channel >= 0) {
    nextCH(next);
    return;
  }

  Band *nextBand = BAND_ByFrequency(vfo.f + dir);
  if (nextBand != gCurrentBand && nextBand != &defaultBand) {
    if (next) {
      tuneTo(nextBand->band.bounds.start);
    } else {
      tuneTo(nextBand->band.bounds.end -
             nextBand->band.bounds.end %
                 StepFrequencyTable[nextBand->band.step]);
    }
  } else {
    tuneTo(vfo.f + StepFrequencyTable[nextBand->band.step] * dir);
  }
  onVfoUpdate();
}

void nextBandFreq(bool next) {
  uint32_t steps = BANDS_GetSteps(gCurrentBand);
  uint32_t step = BANDS_GetChannel(gCurrentBand, vfo.f);
  IncDec32(&step, 0, steps, next ? 1 : -1);
  vfo.f = BANDS_GetF(gCurrentBand, step);
  tuneToPure(vfo.f, true);
}

void nextBandFreqEx(bool next, bool precise) {
  uint32_t steps = BANDS_GetSteps(gCurrentBand);
  uint32_t step = BANDS_GetChannel(gCurrentBand, vfo.f);
  IncDec32(&step, 0, steps, next ? 1 : -1);
  vfo.f = BANDS_GetF(gCurrentBand, step);
  tuneToPure(vfo.f, precise);
}

void onVfoUpdate() {
  Scheduler::taskRemove(saveCurrentCH);
  Scheduler::taskAdd("CH save", saveCurrentCH, 2000, false, 0);
}
}; // namespace svc::tune
