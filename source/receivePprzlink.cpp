#include <ch.h>
#include <hal.h>
#include "receivePprzlink.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "sdcard.hpp"
#include "pprzlink/pprzlink_smartprobe.h"


#define PERIOD(k) (CH_CFG_ST_FREQUENCY / CONF(k))


namespace {
  struct pprzlink_device_rx dev_rx;
  uint8_t rx_buffer[255];
  static ReceivePprzlink *rbb = nullptr; // hack waiting for user_data field in callback

  void new_message_cb(uint8_t sender_id, uint8_t receiver_id, uint8_t class_id,
		      uint8_t message_id, uint8_t *buf);
  static void rtcSetTime(uint16_t week, uint32_t itow);
};



bool ReceivePprzlink::initInThreadContext()
{


  return true;
}

bool ReceivePprzlink::init()
{
  rbb = this;
  dev_rx = pprzlink_device_rx_init(
				   [] (void) -> int { // char_available
				     return true; // always true, sdGet will block
				   },
				   [] (void) -> uint8_t  {return sdGet(&ExtSD);}, // get_char
				   rx_buffer
				   );

  while (ExtSD.state != SD_READY)
    chThdSleepMilliseconds(1); // wait for emitter to configure uart
  return true;
}

  

bool ReceivePprzlink::loop()
{
  pprzlink_check_and_parse(&dev_rx, &new_message_cb);
  return true;
}
  

namespace {
  
  void new_message_cb([[maybe_unused]] uint8_t sender_id,
		      [[maybe_unused]] uint8_t receiver_id,
		      [[maybe_unused]] uint8_t class_id,
		      uint8_t message_id, uint8_t *buf) {
    // check message/class IDs to before extracting data from the messages
    if (message_id == PPRZ_MSG_ID_GPS) {
      // get data from GPS
      GpsData wdata {
		     .mode = pprzlink_get_GPS_mode(buf),
		     .utm_east = pprzlink_get_GPS_utm_east(buf),
		     .utm_north = pprzlink_get_GPS_utm_north(buf),
		     .course = pprzlink_get_GPS_course(buf),	
		     .alt = pprzlink_get_GPS_alt(buf),		
		     .speed = pprzlink_get_GPS_speed(buf),	
		     .climb = pprzlink_get_GPS_climb(buf),	
		     .week = pprzlink_get_GPS_week(buf),	
		     .itow = pprzlink_get_GPS_itow(buf),	
		     .utm_zone = pprzlink_get_GPS_utm_zone(buf),	
		     .gps_nb_err = pprzlink_get_GPS_gps_nb_err(buf) 
      };
      rbb->pprzlink.write(wdata); // hack waiting for user_data field in callback
      SdCard::logSyslog(Severity::Info, "DEBUG> gps east = %ld north = %ld",
			wdata.utm_east, wdata.utm_north);
    } 
  }
  
  static void rtcSetTime(uint16_t week, uint32_t itow)
  {
    if (itow != 0) {
      // Unix timestamp of the GPS epoch 1980-01-06 00:00:00 UTC
      constexpr uint32_t unixToGpsEpoch = 315964800;
      struct tm time_tm;
      time_t univTime = ((week * 7 * 24 * 3600) + (itow / 1000)) + unixToGpsEpoch;
      gmtime_r(&univTime, &time_tm);
      // Chibios date struct
      RTCDateTime date;
      rtcConvertStructTmToDateTime(&time_tm, 0, &date);
      rtcSetTime(&RTCD1, &date);
    }
  }
  
}
