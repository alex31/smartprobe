#pragma once
#include "workerClass.hpp"
#include "sdLog.h"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "relativeWind.hpp"
#include "confParameters.hpp"
#include "hardwareConf.hpp"


enum class Severity {Debug, Info, Warning, Fatal, Internal};

class SdCard : public WorkerThread<TH_SDCARD::threadStackSize, SdCard> {
public:
  SdCard(const tprio_t m_prio) :
    WorkerThread<TH_SDCARD::threadStackSize, SdCard>("sdcard", m_prio) {};
  static SdioError logSensors (const char* fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
  static SdioError logSyslog (const Severity severity, const char* fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
  static SdioError logSyslogRaw (const char* fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
  bool initHardware(void);
private:
  friend WorkerThread<TH_SDCARD::threadStackSize, SdCard>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  bool sdLogInit(void);
  void writeSyslogHeader(void);
  void writeSensorlogHeader(void);
  bool writeTSVSensorlog(void);
  bool writeBinarySensorlog(void);
  SdioError writeTSVSensorlog_RAW_AND_GPS(void);
  SdioError writeTSVSensorlog_RAW_NO_GPS(void);
  SdioError writeTSVSensorlog_HEADLESS_AND_GPS(void);
  SdioError writeTSVSensorlog_HEADLESS_NO_GPS(void);


  static SdCard *self;
  uint32_t 	freeSpaceInKo = 0;
  FileDes	syslogFd = -1;
  FileDes 	sensorsFd = -1;
  AhrsType 	ahrsType{};
  SerialMode 	serialMode{};
  bool     	logGps = false;
  bool		hardareInitialised{false};

  event_listener_t baroEvent, diffPressEvent, imuEvent;
};




