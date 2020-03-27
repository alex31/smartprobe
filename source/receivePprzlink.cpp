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
#include "util.hpp"

struct PprzGpsData {
  uint8_t  mode;
  int32_t  utm_east;
  int32_t  utm_north;
  int16_t  course;
  int32_t  alt;
  uint16_t speed;
  int16_t  climb;
  uint16_t week;
  uint32_t itow;
  uint8_t  utm_zone;
  uint8_t  gps_nb_err;   
};


namespace {
  struct pprzlink_device_rx dev_rx;
  uint8_t rx_buffer[255];

  void new_message_cb(uint8_t sender_id, uint8_t receiver_id, uint8_t class_id,
		      uint8_t message_id, uint8_t *buf, void *user_data);
  static RTCDateTime weekItowToRTC(uint16_t week, uint32_t itow);
};




bool ReceivePprzlink::init()
{
  ReceiveBaselink::init();
  dev_rx = pprzlink_device_rx_init(
				   [] (void) -> int { // char_available
				     return true; // always true, sdGet will block
				   },
				   [] (void) -> uint8_t  {return sdGet(&ExtSD);}, // get_char
				   rx_buffer,
				   this
				   );

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
		      uint8_t message_id, uint8_t *buf,
		      void *user_data) {
    ReceivePprzlink * const rpl = static_cast<ReceivePprzlink *>(user_data);
    // check message/class IDs to before extracting data from the messages
    if (message_id == PPRZ_MSG_ID_GPS) {
      // get data from GPS
      PprzGpsData pprzGps {
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

      CommonGpsData commonGps = {
				 .rtcTime = weekItowToRTC(pprzGps.week, pprzGps.itow),
				 .utm_east = pprzGps.mode,
				 .utm_north =  pprzGps.utm_north,
				 .alt = pprzGps.alt,	     
				 .course = pprzGps.course,	     
				 .speed = pprzGps.speed,	     
				 .climb =   pprzGps.climb,
				 .utm_zone =  pprzGps.utm_zone
      };

      if (pprzGps.itow and pprzGps.week) {
	rpl->blackBoard.write(commonGps); // hack waiting for user_data field in callback
	SdCard::logSyslog(Severity::Info, "DEBUG> gps east = %ld north = %ld",
			  commonGps.utm_east, commonGps.utm_north);
      } else {
	SdCard::logSyslog(Severity::Warning, "DEBUG> gps message with ITOW and WEEK = 0");
      }
    } 
  }

  static RTCDateTime weekItowToRTC(uint16_t week, uint32_t itow)
  {
      // Unix timestamp of the GPS epoch 1980-01-06 00:00:00 UTC
      constexpr uint32_t unixToGpsEpoch = 315964800;
      struct tm time_tm;
      time_t univTime = ((week * 7 * 24 * 3600) + (itow / 1000)) + unixToGpsEpoch;
      gmtime_r(&univTime, &time_tm);
      // Chibios date struct
      RTCDateTime date;
      rtcConvertStructTmToDateTime(&time_tm, 0, &date);
      return date;
  }

  
}
