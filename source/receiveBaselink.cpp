#include <ch.h>
#include <hal.h>
#include "receiveBaselink.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "sdcard.hpp"
#include "util.hpp"


bool ReceiveBaselink::init()
{
  int count =0;
  while (ExtSD.state != SD_READY and ++count < 1000)
    chThdSleepMilliseconds(1); // wait for emitter to configure uart
  // after one second, nobody will do it for us; we start SD ourself
  if (ExtSD.state != SD_READY) {
    static const SerialConfig serialConfig =  {
					       static_cast<uint32_t>(CONF("uart.baud")),
					       0,
					       USART_CR2_STOP1_BITS | USART_CR2_LINEN,
					       0
    };
    sdStart(&ExtSD, &serialConfig);
  }
  return true;
}

  

uint8_t ReceiveBaselink::weekday(uint8_t month,  uint8_t day, uint16_t year)
{	
  uint16_t ix, tx, vx=0;
  
  switch (month) {
  case 2 :
  case 6 : vx = 0; break;
  case 8 : vx = 4; break;
  case 10 : vx = 8; break;
  case 9 :
  case 12 : vx = 12; break;
  case 3 :
  case 11 : vx = 16; break;
  case 1 :
  case 5 : vx = 20; break;
  case 4 :
  case 7 : vx = 24; break;
  }
  if (year > 1900) // 1900 was not a leap year
    year -= 1900;
  ix = ((year - 21) % 28) + vx + (month > 2); // take care of February
  tx = (ix + (ix / 4)) % 7 + day; // take care of leap year
  return ((tx+1) % 7);
}

RTCDateTime ReceiveBaselink::dmyToRTCD(const DayMonthYear& dmy)
{
  const uint32_t h = static_cast<uint32_t>(dmy.utc/10000);
  const uint32_t m = static_cast<uint32_t>(dmy.utc/100) % 100;
  const double s = fmod(dmy.utc, 100.0) + (h*3600) + (m*60);
  return RTCDateTime {
    .year = dmy.year - 1980U,
      .month = dmy.month,
      .dstflag = 0,
      .dayofweek = weekday(dmy.month, dmy.day,  dmy.year) + 1U,
      .day = dmy.day,
      .millisecond = static_cast<uint32_t>(s*1000.0)};
}


double ReceiveBaselink::gpsAngleToRad(const double gpsAngle)
{
  const double deg = floor(gpsAngle/100.0);
  const double min = gpsAngle-(deg*100.0);
  const double fracDeg = deg+(min/60.0);
  return deg2rad(fracDeg);
}
