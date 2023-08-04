#pragma once
#include "sdio.h"
#include "workerClass.hpp"
#include "sdLiteLog.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "relativeWind.hpp"
#include "confParameters.hpp"
#include "hardwareConf.hpp"
#include "binaryLogFrame.hpp"

enum class Severity {Debug, Info, Warning, Fatal, Internal};

class SdCard : public WorkerThread<TH_SDCARD::threadStackSize, SdCard> {
public:
  SdCard(const tprio_t m_prio) :
    WorkerThread<TH_SDCARD::threadStackSize, SdCard>("sdcard", m_prio),
    sensors(SENSORS_SYNC_PERIOD), syslog(SYSLOG_SYNC_PERIOD)  {};

  bool initHardware(void);
  
  static SdLiteStatus logSensors (const char* fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
  template<typename T>
  static SdLiteStatus logSensors (const T& t);
  
  static SdLiteStatus logSyslog (const Severity severity, const char* fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
  static SdLiteStatus logSyslog (const char* fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
  

private:
  friend WorkerThread<TH_SDCARD::threadStackSize, SdCard>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  bool sdLogInit(void);
  void writeSyslogHeader(void);
  void writeTSVSensorlogHeader(void);
  void writeBinarySensorlogHeader(void);
  bool writeTSVSensorlog(void);
  bool writeBinarySensorlog(void);
  void lock(void) const {chMtxLock(&mutLog);}
  void unlock(void) const {chMtxUnlock(&mutLog);}
  SdLiteStatus writeTSVSensorlog_RAW_AND_GPS(void);
  SdLiteStatus writeTSVSensorlog_RAW_NO_GPS(void);
  SdLiteStatus writeTSVSensorlog_HEADLESS_AND_GPS(void);
  SdLiteStatus writeTSVSensorlog_HEADLESS_NO_GPS(void);


  static SdCard *self;
  uint32_t 	freeSpaceInKo = 0;
  SdLiteLog<SENSORS_BUFFER_SIZE> sensors;
  SdLiteLog<SYSLOG_BUFFER_SIZE> syslog;
  AhrsType 	ahrsType{};
  SerialMode 	serialMode{};
  bool     	logGps = false;
  bool		hardareInitialised{false};

  event_listener_t baroEvent, diffPressEvent, imuEvent;
  mutable mutex_t mutLog = __MUTEX_DATA(mutLog);
};



template<typename T>
SdLiteStatus SdCard::logSensors (const T& t)
{
  if (self != nullptr) {
    auto [s, record] = self->sensors.borrow<T>();
    record = t; // NOT zero copy : API must be changed
    return s;
  } else {
    return SdLiteStatus::NOT_READY;
  }
}

