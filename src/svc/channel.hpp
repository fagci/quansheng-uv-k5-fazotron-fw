#pragma once

#include "../radio.hpp"

class ChannelService {
public:
  typedef struct CH : Radio::VFO {
    char *name[10];
  } __attribute__((packed)) VFO;

private:
};
