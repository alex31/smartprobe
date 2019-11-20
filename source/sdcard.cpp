#include <ch.h>
#include <hal.h>
#include "sdcard.hpp"
#include "stdutil.h"	
#include "sdio.h"

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
  return sdLogInit();
}


bool SdCard::loop()
{
  static int i = 0;
  SdioError se;
  se = logSensors("i = %d", i++);
    switch (se) {
    case SDLOG_FATFS_ERROR : DebugTrace("sdWrite sensors: Fatfs error");
      return false;
    case SDLOG_INTERNAL_ERROR : DebugTrace("sdWrite sensors: Internal error");
      return false;
    default: break;
  }

  se = logSyslog("i = %d", i++);
    switch (se) {
    case SDLOG_FATFS_ERROR : DebugTrace("sdWrite syslog: Fatfs error");
      return false;
    case SDLOG_INTERNAL_ERROR : DebugTrace("sdWrite syslog: Internal error");
      return false;
    default: break;
  }
  
  chThdSleepMilliseconds(500);
  return true;
}

  


bool  SdCard::sdLogInit(void)
{
  SdioError se;

  if (not sdioIsCardResponding()) {
    DebugTrace("sensors OK but µSD card not present, or not reponding\r\n");
    return false;
  }
  
  se = ::sdLogInit(&freeSpaceInKo);
  switch (se) {
  case SDLOG_OK : DebugTrace(" freeSpaceInKo = %lu Mo", freeSpaceInKo/1024); break;
  case SDLOG_FATFS_ERROR : DebugTrace("sdLogInit: Fatfs error"); return false;
  case SDLOG_INTERNAL_ERROR : DebugTrace("sdLogInit: Internal error"); return false;
  default: break;
  }

  se = sdLogOpenLog(&syslogFd, "SMARTPROBE", "syslog.txt", 1_seconde,
		    LOG_APPEND_TAG_AT_CLOSE_DISABLED, 1_megabyte,
		    LOG_PREALLOCATION_DISABLED);
  switch (se) {
  case SDLOG_OK : DebugTrace("sdOpenLog syslog Ok"); break;
  case SDLOG_FATFS_ERROR : DebugTrace("sdOpenLog syslog: Fatfs error");
    return false;
  case SDLOG_INTERNAL_ERROR : DebugTrace("sdOpenLog syslog: Internal error");
    return false;
  default: break;
  }

  se = sdLogOpenLog(&sensorsFd, "SMARTPROBE", "sensors.txt", 1_seconde,
		    LOG_APPEND_TAG_AT_CLOSE_DISABLED, 1_megabyte,
		    LOG_PREALLOCATION_ENABLED);
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

SdioError SdCard::logSensors (const char* fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  return sdLogvWriteLog(self->sensorsFd, fmt, ap);
  va_end(ap);
}

SdioError SdCard::logSyslog (const char* fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  return sdLogvWriteLog(self->syslogFd, fmt, ap);
  va_end(ap);
}



SdCard  *SdCard::self = nullptr;
