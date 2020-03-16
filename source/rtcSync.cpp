#include <ch.h>
#include <hal.h>
#include "rtcSync.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "sdcard.hpp"
#include "threadAndEventDecl.hpp"
#include "receivePprzlink.hpp"
#include "rtcAccess.h"

namespace {
  CommonGpsData gpsData{};
}





bool RtcSync::init()
{
  return true;
}

bool RtcSync::initInThreadContext()
{
  // registerEvt must be done in the thread that will wait on event,
  // so cannot be done in init method which is call by the parent thread
  receivePPL.blackBoard.registerEvt(&pprzGpsEvent, PPRZ_GPS_EVT);
  receiveNMEA.blackBoard.registerEvt(&nmeaGpsEvent, NMEA_GPS_EVT);

  return true;
}



bool RtcSync::loop()
{
  const eventmask_t event = chEvtWaitAny(PPRZ_GPS_EVT | NMEA_GPS_EVT);
  switch (event) {
  case PPRZ_GPS_EVT:
    receivePPL.blackBoard.read(gpsData);
    break;
  case NMEA_GPS_EVT:
    receiveNMEA.blackBoard.read(gpsData);
    break;
  default:
    break;
  }
  rtcSetTime(&RTCD1, &gpsData.rtcTime);
  SdCard::logSyslog(Severity::Info, "RTC :"
		    "H=%02ld::%02ld::%02ld D=%s %02ld/%02ld/%04ld",
		    getUtcHour(), getMinute(), getSecond(),
		    getWeekDayAscii(), getMonthDay(), getMonth(), getYear());
		    

  return true;
}
  


