#include <ch.h>
#include <hal.h>
#include "rtcSync.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "sdcard.hpp"
#include "threadAndEventDecl.hpp"
#include "receivePprzlink.hpp"

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
  receivePPL.blackBoard.registerEvt(&gpsEvent, PPRZ_GPS_EVT);

  return true;
}



bool RtcSync::loop()
{
  chEvtWaitOne(PPRZ_GPS_EVT);
  receivePPL.blackBoard.read(gpsData);
  rtcSetTime(&RTCD1, &gpsData.rtcTime);
  return true;
}
  


