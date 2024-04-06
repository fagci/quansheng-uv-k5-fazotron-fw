#pragma once

#include "../board.hpp"
#include "../dcs.hpp"
#include "../driver/uart.hpp"
#include "../svc/svc.hpp"
#include "driver/bk4819.hpp"

struct Loot {
  uint32_t f;
  uint32_t firstTime;
  uint32_t lastTimeCheck;
  uint32_t lastTimeOpen;
  uint32_t cd = 0xFF;
  uint16_t ct = 0xFF;
  uint16_t duration = 0;
  uint16_t rssi = 0;
  bool open;
  bool blacklist;
  bool goodKnown;
};

class LootService : public Svc {
public:
  static constexpr uint8_t LOOT_SIZE_MAX = 128;
  Loot loot[LOOT_SIZE_MAX] = {0};
  int16_t lootIndex = -1;

  Loot *gLastActiveLoot = NULL;
  int16_t gLastActiveLootIndex = -1;

  void blacklistLast() {
    if (gLastActiveLoot) {
      gLastActiveLoot->goodKnown = false;
      gLastActiveLoot->blacklist = true;
    }
  }

  void goodKnownLast() {
    if (gLastActiveLoot) {
      gLastActiveLoot->blacklist = false;
      gLastActiveLoot->goodKnown = true;
    }
  }

  Loot *get(uint32_t f) {
    for (uint8_t i = 0; i < size(); ++i) {
      if ((&loot[i])->f == f) {
        return &loot[i];
      }
    }
    return NULL;
  }

  int16_t indexOf(Loot *item) {
    for (int16_t i = 0; i < size(); ++i) {
      if (&item[i] == item) {
        return i;
      }
    }
    return -1;
  }

  Loot *addEx(uint32_t f, bool reuse) {
    if (reuse) {
      Loot *p = get(f);
      if (p) {
        return p;
      }
    }
    if (size() < LOOT_SIZE_MAX) {
      lootIndex++;
      loot[lootIndex] = (Loot){
          .f = f,
          .firstTime = elapsedMilliseconds,
          .lastTimeCheck = elapsedMilliseconds,
          .lastTimeOpen = elapsedMilliseconds,
          .open = true, // as we add it when open
      };
      return &loot[lootIndex];
    }
    return NULL;
  }

  Loot *add(uint32_t f) { return addEx(f, true); }

  void remove(uint8_t i) {
    if (size()) {
      for (uint8_t _i = i; _i < size() - 1; ++_i) {
        loot[_i] = loot[_i + 1];
      }
      lootIndex--;
    }
  }

  void clear() { lootIndex = -1; }

  uint8_t size() { return lootIndex + 1; }

  void standby() {
    for (uint8_t i = 0; i < size(); ++i) {
      Loot *p = &loot[i];
      p->open = false;
      p->lastTimeCheck = elapsedMilliseconds;
    }
  }

  static void swap(Loot *a, Loot *b) {
    Loot tmp = *a;
    *a = *b;
    *b = tmp;
  }

  bool sortByLastOpenTime(Loot *a, Loot *b) {
    return a->lastTimeOpen < b->lastTimeOpen;
  }

  bool sortByDuration(Loot *a, Loot *b) { return a->duration > b->duration; }

  bool sortByF(Loot *a, Loot *b) { return a->f > b->f; }

  static bool sortByBlacklist(Loot *a, Loot *b) {
    return a->blacklist > b->blacklist;
  }

  static void Sort(Loot *items, uint16_t n, bool (*compare)(Loot *a, Loot *b),
                   bool reverse) {
    for (uint16_t i = 0; i < n - 1; i++) {
      bool swapped = false;
      for (uint16_t j = 0; j < n - i - 1; j++) {
        if (compare(&items[j], &items[j + 1]) ^ reverse) {
          swap(&items[j], &items[j + 1]);
          swapped = true;
        }
      }
      if (!swapped) {
        break;
      }
    }
  }

  void sort(bool (*compare)(Loot *a, Loot *b), bool reverse) {
    Sort(loot, size(), compare, reverse);
  }

  Loot *item(uint8_t i) { return &loot[i]; }

  void replace(Loot *item, uint32_t f) {
    item->f = f;
    item->open = false;
    item->firstTime = elapsedMilliseconds;
    item->lastTimeCheck = elapsedMilliseconds;
    item->lastTimeOpen = 0;
    item->duration = 0;
    item->rssi = 0;
    item->ct = 0xFF;
    item->cd = 0xFF;
  }

  void replaceItem(uint8_t i, uint32_t f) { replace(item(i), f); }

  void updateEx(Loot *item, Loot *msm) {
    if (item == NULL) {
      return;
    }

    if (item->blacklist || item->goodKnown) {
      msm->open = false;
    }

    item->rssi = msm->rssi;

    if (item->open) {
      item->duration += elapsedMilliseconds - item->lastTimeCheck;
      gLastActiveLoot = item;
      gLastActiveLootIndex = indexOf(item);
    }
    if (msm->open) {
      uint32_t cd = 0;
      uint16_t ct = 0;
      uint8_t Code = 0;
      BK4819::CssScanResult_t res =
          Board::radio.bk4819.getCxCSSScanResult(&cd, &ct);
      switch (res) {
      case BK4819::CSS_RESULT_CDCSS:
        Code = DCS_GetCdcssCode(cd);
        if (Code != 0xFF) {
          item->cd = Code;
        }
        break;
      case BK4819::CSS_RESULT_CTCSS:
        Code = DCS_GetCtcssCode(ct);
        if (Code != 0xFF) {
          item->ct = Code;
        }
        break;
      default:
        break;
      }
      item->lastTimeOpen = elapsedMilliseconds;
    }
    item->lastTimeCheck = elapsedMilliseconds;
    item->open = msm->open;

    if (msm->blacklist) {
      item->blacklist = true;
    }
  }

  void updateLoot(Loot *msm) {
    Loot *item = get(msm->f);

    if (item == NULL && msm->open) {
      item = add(msm->f);
      UART_logf(1, "[LOOT] %u", msm->f);
    }

    updateEx(item, msm);
  }

  void removeBlacklisted() {
    sort(sortByBlacklist, true);
    for (uint8_t i = 0; i < size(); ++i) {
      if (loot[i].blacklist) {
        lootIndex = i;
        return;
      }
    }
  }
};
