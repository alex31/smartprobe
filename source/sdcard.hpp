#pragma once
#include "workerClass.hpp"
#include "sdLog.h"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"

namespace TH_SDCARD {
static constexpr size_t threadStackSize = 2048U;
}

enum class Severity {Debug, Info, Warning, Fatal, Internal};

class SdCard : public WorkerThread<TH_SDCARD::threadStackSize, SdCard> {
public:
  SdCard(const tprio_t m_prio) :
    WorkerThread<TH_SDCARD::threadStackSize, SdCard>("sdcard", m_prio) {};
  static SdioError logSensors (const char* fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
  static SdioError logSyslog (const Severity severity, const char* fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
  static SdioError logSyslogNoNl (const Severity severity, const char* fmt, ...)
    __attribute__ ((format (printf, 2, 3)));

private:
  friend WorkerThread<TH_SDCARD::threadStackSize, SdCard>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  bool sdLogInit(void);
  void writeSyslogHeader(void);

  static SdCard *self;
  uint32_t freeSpaceInKo = 0;
  FileDes syslogFd = -1;
  FileDes sensorsFd = -1;

  event_listener_t baroEvent, diffPressEvent, imuEvent;
  BarometerData baroData{};
  DiffPressureData diffPressData{};
  ImuData imuData{};
};




