#pragma once

#include "driver/abstractradio.hpp"

class ChannelService {
public:
  typedef struct CH : VFO {
    char *name[10];
  } __attribute__((packed)) VFO;

private:
};
