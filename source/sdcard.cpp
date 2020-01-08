#include <ch.h>
#include <hal.h>
#include "sdcard.hpp"
#include "stdutil.h"	
#include "sdio.h"
#include <frozen/map.h>
#include <frozen/string.h>
#include "hardwareConf.hpp"
#include "confFile.hpp"
#include "threadAndEventDecl.hpp"

#define xstr(s) str(s)
#define str(s) #s
  

namespace {
  BarometerData baroData{};
  DiffPressureData diffPressData{};
  ImuData imuData{};
  Vec3f   attitude{};
  AirSpeed relAirSpeed{};
  
  constexpr auto severityName = frozen::make_map<Severity, frozen::string> ({
	{Severity::Debug, "DEBUG"},
	{Severity::Info, "INFO"},
	{Severity::Warning, "WARNING"},
	{Severity::Fatal, "FATAL"},
	{Severity::Internal, "INTERNAL (please report bug)"}
    });

  bool highTimeStampPrecision;
}

static constexpr uint32_t operator"" _seconde (unsigned long long int duration)
{
  return duration;
}

static constexpr uint32_t operator"" _megabyte (unsigned long long int size)
{
  return size;
}


bool SdCard::init()
{
  bool retVal = sdLogInit();
  if (retVal) {
    self = this;
  }
  else
    self = nullptr;

  return retVal;
}

bool SdCard::initInThreadContext()
{
  // registerEvt must be done in the thread that will wait on event,
  // so cannot be done in init method which is call by the parent thread

  writeSyslogHeader();
  baro.blackBoard.registerEvt(&baroEvent, BARO_EVT);
  dp.blackBoard.registerEvt(&diffPressEvent, PDIF_EVT);
  imu.blackBoard.registerEvt(&imuEvent, IMU_EVT);

  // wait for the conf file to be read and dictionary initialised.
  // any event are sent after this inititialisation, so we wait for
  // the first event to write the header and launch the worker logger thread
  chEvtWaitAny(ALL_EVENTS);
  ahrsType = static_cast<AhrsType>(CONF("ahrs.type"));
  highTimeStampPrecision = CONF("thread.frequency.d_press") >= 100;
  writeSensorlogHeader();
  return true;
}


bool SdCard::loop()
{
  //  chEvtWaitAll(IMU_EVT | BARO_EVT | PDIF_EVT);
  SdioError se;
  const eventmask_t event = chEvtWaitOneTimeout(PDIF_EVT, TIME_MS2I(1000)); // log each new differential sample

  if (event) {
    baro.blackBoard.read(baroData);
    dp.blackBoard.read(diffPressData);
    relwind.blackBoard.read(relAirSpeed);
    
    if (ahrsType == RAW_IMU) {
      imu.blackBoard.read(imuData);
      se = logSensors("%4.2f\t%3.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.2f\t%.1f\t",
		      baroData.pressure,
		      baroData.temp,
		      diffPressData[0].pressure,
		      diffPressData[1].pressure,
		      diffPressData[2].pressure,
		      diffPressData[0].temp,
		      diffPressData[1].temp,
		      diffPressData[2].temp,
		      relAirSpeed.velocity,
		      relAirSpeed.alpha,
		      relAirSpeed.beta,
		      imuData.acc.v[0],
		      imuData.acc.v[1],
		      imuData.acc.v[2],
		      imuData.gyro.v[0],
		      imuData.gyro.v[1],
		      imuData.gyro.v[2],
		      adc.getPowerSupplyVoltage(),
		      adc.getCoreTemp());
    } else  {
          ahrs.blackBoard.read(attitude);
	  se = logSensors("%4.2f\t%3.2f\t"
			  "%.4f\t%.4f\t%.4f\t"
			  "%.2f\t%.2f\t%.2f\t"
			  "%.2f\t%.2f\t%.2f\t"
			  "%.4f\t%.4f\t%.4f\t"
			  "%.2f\t%.1f\t",
			  baroData.pressure, baroData.temp,
			  diffPressData[0].pressure,
			  diffPressData[1].pressure,
			  diffPressData[2].pressure,
			  diffPressData[0].temp,
			  diffPressData[1].temp,
			  diffPressData[2].temp,
			  relAirSpeed.velocity,
			  relAirSpeed.alpha,
			  relAirSpeed.beta,
			  rad2deg(attitude.v[0]), rad2deg(attitude.v[1]), rad2deg(attitude.v[2]),
			  adc.getPowerSupplyVoltage(), adc.getCoreTemp());
    }
    
    switch (se) {
    case SDLOG_FATFS_ERROR : DebugTrace("sdWrite sensors: Fatfs error");
      return false;
    case SDLOG_INTERNAL_ERROR : DebugTrace("sdWrite sensors: Internal error");
      return false;
    default: break;
    }
  }

  // timout is not considered as an error
  return true;
}

  


bool  SdCard::sdLogInit(void)
{
  SdioError se;

  if (not sdioIsCardResponding()) {
    DebugTrace("µSD card not present, or not reponding\r\n");
    return false;
  }
  
  se = ::sdLogInit(&freeSpaceInKo);
  switch (se) {
  case SDLOG_OK : DebugTrace(" freeSpaceInKo = %lu Mo", freeSpaceInKo/1024); break;
  case SDLOG_FATFS_ERROR : DebugTrace("sdLogInit: Fatfs error"); return false;
  case SDLOG_INTERNAL_ERROR : DebugTrace("sdLogInit: Internal error"); return false;
  default: break;
  }

  se = sdLogOpenLog(&syslogFd, ROOTDIR, SYSLOG_FILENAME, 1_seconde,
		    LOG_APPEND_TAG_AT_CLOSE_ENABLED, 0,
		    LOG_PREALLOCATION_DISABLED);
  switch (se) {
  case SDLOG_OK : DebugTrace("sdOpenLog syslog Ok"); break;
  case SDLOG_FATFS_ERROR : DebugTrace("sdOpenLog syslog: Fatfs error");
    return false;
  case SDLOG_INTERNAL_ERROR : DebugTrace("sdOpenLog syslog: Internal error");
    return false;
  default: break;
  }

  se = sdLogOpenLog(&sensorsFd, ROOTDIR,
		    "sensors",
		    10_seconde,
		    LOG_APPEND_TAG_AT_CLOSE_DISABLED, 0,
		    LOG_PREALLOCATION_DISABLED);
  switch (se) {
  case SDLOG_OK : DebugTrace("sdOpenLog sensors Ok"); break;
  case SDLOG_FATFS_ERROR : DebugTrace("sdOpenLog sensors: Fatfs error");
    return false;
  case SDLOG_INTERNAL_ERROR : DebugTrace("sdOpenLog sensors: Internal error");
    return false;
  default: break;
  }

  
  return true;
}


void  SdCard::writeSyslogHeader(void)
{
  logSyslog(Severity::Info, "syslog start");
  logSyslog(Severity::Info, xstr(GIT_VERSION));
  
    logSyslog(Severity::Info, "Kernel:       %s", CH_KERNEL_VERSION);
#ifdef HAL_VERSION
  logSyslog(Severity::Info, "Hal:          %s", HAL_VERSION);
#endif

#ifdef CH_COMPILER_NAME
  logSyslog(Severity::Info, "Compiler:     %s", CH_COMPILER_NAME);
#endif
#ifdef PORT_COMPILER_NAME
  logSyslog(Severity::Info, "Compiler:     %s", PORT_COMPILER_NAME);
#endif

#ifdef CH_ARCHITECTURE_NAME
  logSyslog(Severity::Info, "Architecture: %s", CH_ARCHITECTURE_NAME);
#endif
#ifdef PORT_ARCHITECTURE_NAME
  logSyslog(Severity::Info, "Architecture: %s", PORT_ARCHITECTURE_NAME);
#endif
  

#ifdef CH_CORE_VARIANT_NAME
  logSyslog(Severity::Info, "Core Variant: %s", CH_CORE_VARIANT_NAME);
#endif
#ifdef PORT_CORE_VARIANT_NAME
  logSyslog(Severity::Info, "Core Variant: %s", PORT_CORE_VARIANT_NAME);
#endif
#ifdef STM32_SYSCLK
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
  logSyslog(Severity::Info, "Main STM32_SYSCLK frequency %.2f Mhz", STM32_SYSCLK/1e6f);
#pragma GCC diagnostic pop
#endif

#ifdef CH_PORT_INFO
  logSyslog(Severity::Info, "Port Info:    %s", CH_PORT_INFO);
#endif
#ifdef PORT_INFO
  logSyslog(Severity::Info, "Port Info:    %s", PORT_INFO);
#endif

#ifdef PLATFORM_NAME
  logSyslog(Severity::Info, "Platform:     %s", PLATFORM_NAME);
#endif

#ifdef BOARD_NAME
  logSyslog(Severity::Info, "Board:        %s", BOARD_NAME);
#endif

  
#ifdef __DATE__
#ifdef __TIME__
  logSyslog(Severity::Info, "Build time:   %s%s%s", __DATE__, " - ", __TIME__);
#endif
#endif
}

void  SdCard::writeSensorlogHeader(void)
{
  if (ahrsType == RAW_IMU) {
  logSensors("baro.p baro.t "
	     "dp[0].p "
	     "dp[1].p "
	     "dp[2].p "
	     "dp[0].t "
	     "dp[1].t "
	     "dp[2].t "
	     "velocity "
	     "alpha "
	     "beta "
	     "acc.x "
	     "acc.y "
	     "acc.z "
	     "gyro.x "
	     "gyro.y "
	     "gyro.z "
	     "vcc "
	     "CoreTemp "
	     );
  } else  if (ahrsType == HEADLESS_AHRS) {
  logSensors("baro.p baro.t "
	     "dp[0].p "
	     "dp[1].p "
	     "dp[2].p "
	     "dp[0].t "
	     "dp[1].t "
	     "dp[2].t "
	     "velocity "
	     "alpha "
	     "beta "
	     "pitch   "
	     "roll   "
	     "yaw   "
	     "vcc "
	     "CoreTemp "
	     );
  } else {
    SdCard::logSyslog(Severity::Fatal, "ahrsType HEADLESS_COMPLETE not yet implemented");
  }
}


SdioError SdCard::logSensors (const char* fmt, ...)
{
  va_list ap;

  if (self != nullptr) {
    sdLogWriteLog(self->sensorsFd, highTimeStampPrecision ? "[%.4f] : " :  "[%.3f] : ",
		  TIME_I2US(chVTGetSystemTimeX())/1e6f);
    va_start(ap, fmt);
    auto retVal = sdLogvWriteLog(self->sensorsFd, fmt, &ap);
    va_end(ap);
    sdLogWriteLog(self->sensorsFd, "\r\n");
    return retVal;
  } else {
    return SDLOG_NOT_READY;
  }
}

SdioError SdCard::logSyslog (const Severity severity, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  
#ifdef TRACE 
#include "printf.h"
  {
    va_list apc;
    va_copy(apc, ap);
    chvprintf(chp, fmt, apc);
    va_end(apc);
    chprintf(chp, "\r\n");
  }
#endif
  
  if (self != nullptr) {
    sdLogWriteLog(self->syslogFd, "[%.3f] %s : ",
		  TIME_I2MS(chVTGetSystemTimeX())/1000.0f,
		  severityName.at(severity).data());
    auto retVal = sdLogvWriteLog(self->syslogFd, fmt, &ap);
    sdLogWriteLog(self->syslogFd, "\r\n");
    va_end(ap);
    sdLogFlushLog(self->syslogFd);
    return retVal;
  } else {
    return SDLOG_NOT_READY;
  }
}

SdioError SdCard::logSyslogRaw (const char* fmt, ...)
{
  va_list ap;
  
  va_start(ap, fmt);
#ifdef TRACE 
#include "printf.h"
  {
    va_list apc;
    va_copy(apc, ap);
    chvprintf(chp, fmt, apc);
    va_end(apc);
  }
#endif
  
  if (self != nullptr) {
    auto retVal = sdLogvWriteLog(self->syslogFd, fmt, &ap);
    va_end(ap);
    return retVal;
  } else {
    return SDLOG_NOT_READY;
  }
}


SdCard  *SdCard::self = nullptr;
