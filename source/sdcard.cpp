#include <ch.h>
#include <hal.h>
#include "sdcard.hpp"
#include "stdutil.h"	
#include "sdio.h"
#include <frozen/map.h>
#include <frozen/string.h>

#define xstr(s) str(s)
#define str(s) #s
  

namespace {
  
  constexpr auto severityName = frozen::make_map<Severity, frozen::string> ({
	{Severity::Debug, "DEBUG"},
	{Severity::Info, "INFO"},
	{Severity::Warning, "WARNING"},
	{Severity::Fatal, "FATAL"}
    });
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
    writeSyslogHeader();
  }
  else
    self = nullptr;

  return retVal;
}


bool SdCard::loop()
{
  static int i = 0, j= 0;
  SdioError se = logSensors("i = %d\n", i++);
  
  switch (se) {
  case SDLOG_FATFS_ERROR : DebugTrace("sdWrite sensors: Fatfs error");
    return false;
  case SDLOG_INTERNAL_ERROR : DebugTrace("sdWrite sensors: Internal error");
    return false;
  default: break;
  }
  
  if ((i % 1000) == 0) {
    se = logSyslog(Severity::Info, "j = %d", j++);
    switch (se) {
    case SDLOG_FATFS_ERROR : DebugTrace("sdWrite syslog: Fatfs error");
      return false;
    case SDLOG_INTERNAL_ERROR : DebugTrace("sdWrite syslog: Internal error");
      return false;
    default: break;
    }
  }
  
  chThdSleepMilliseconds(1); // in final will be event waiting
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

  se = sdLogOpenLog(&syslogFd, "SMARTPROBE", "syslog", 1_seconde,
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

  se = sdLogOpenLog(&sensorsFd, "SMARTPROBE", "sensors", 10_seconde,
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


SdioError SdCard::logSensors (const char* fmt, ...)
{
  va_list ap;

  if (self != nullptr) {
    va_start(ap, fmt);
    return sdLogvWriteLog(self->sensorsFd, fmt, ap);
    va_end(ap);
  } else {
    return SDLOG_NOT_READY;
  }
}

SdioError SdCard::logSyslog (const Severity severity, const char* fmt, ...)
{
  va_list ap;
  
#ifdef TRACE 
#include "printf.h"
  {
    va_start(ap, fmt);
    chvprintf(chp, fmt, ap);
    chprintf(chp, "\r\n");
    va_end(ap);
  }
#endif
  
  if (self != nullptr) {
    va_start(ap, fmt);
    sdLogWriteLog(self->syslogFd, "[%.3f] %s : ",
		   TIME_I2MS(chVTGetSystemTimeX())/1000.0f,
		   severityName.at(severity).data());
    auto retVal = sdLogvWriteLog(self->syslogFd, fmt, ap);
    sdLogWriteLog(self->syslogFd, "\r\n");
    va_end(ap);
    return retVal;
  } else {
    return SDLOG_NOT_READY;
  }
}


SdCard  *SdCard::self = nullptr;
