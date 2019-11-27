#pragma once
#include "workerClass.hpp"
#include "sdLog.h"

namespace TH_SDCARD {
static constexpr size_t threadStackSize = 1024U;
}

class SdCard : public WorkerThread<TH_SDCARD::threadStackSize, SdCard> {
public:
  SdCard(const tprio_t m_prio) :
    WorkerThread<TH_SDCARD::threadStackSize, SdCard>("sdcard", m_prio) {};
  static SdioError logSensors (const char* fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
  static SdioError logSyslog (const char* fmt, ...)
    __attribute__ ((format (printf, 1, 2)));

private:
  friend WorkerThread<TH_SDCARD::threadStackSize, SdCard>;
  bool init(void) final;
  bool loop(void) final;
  bool sdLogInit(void);

  static SdCard *self;
  uint32_t freeSpaceInKo = 0;
  FileDes syslogFd = -1;
  FileDes sensorsFd = -1;
};




