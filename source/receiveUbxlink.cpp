#include <ch.h>
#include <hal.h>
#include "receiveUbxlink.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "sdcard.hpp"
#include "pprz_geodetic_float.h"
#include "ubxNavPvt.hpp"

namespace {
  CommonGpsData commonGps{};

  const auto readCB = [] (std::byte *bpt, size_t len) -> int {
		        const int res = sdRead(&ExtSD, reinterpret_cast<uint8_t *>(bpt), len);
			return res;
		      };

  
};




bool ReceiveUbxlink::init()
{
  const auto readCB = [] (std::byte *bpt, size_t len) -> int {
			int nb;
			do {nb = sdReadTimeout(&ExtSD,
					       reinterpret_cast<uint8_t *>(bpt),
					       len, TIME_S2I(2));
			  if (nb ==0) 
			    SdCard::logSyslog(Severity::Warning,
					      "ubx link : 2 seconds without data from gps");
			} while (nb != static_cast<int>(len));
			return nb;
		      };
  const auto msgCb = [this] ([[maybe_unused]] const TopicLen_t &toplen,
			     const std::byte *buf) -> void {
		       (void) buf;
		       UbxNavPvt navPvt;
		       memcpy(&navPvt, buf, sizeof(navPvt));
		       propagateNavPvt(navPvt);
		     };
  
  ubx = new UbxNavPvtSerialDecoder<98, topics.size()>(topics, msgCb, readCB);
  ReceiveBaselink::init();

  return true;
}

  

bool ReceiveUbxlink::loop()
{
  const UbxError uerr = ubx->step();

  if (uerr != UbxError::OK)
    SdCard::logSyslog(Severity::Warning, ubx->getLastError());
		      
  return (true);
}


void ReceiveUbxlink::propagateNavPvt(const UbxNavPvt& navPvt)
{
  const RTCDateTime rtcd {
    .year = navPvt.year - 1980U,
    .month = navPvt.month,
    .dstflag = 0,
    .dayofweek = weekday(navPvt.month, navPvt.day,  navPvt.year) + 1U,
    .day = navPvt.day,
    .millisecond = navPvt.iTOW % (24 * 60 * 60 * 1000)};

  UtmCoor_f utm{};
  LlaCoor_f latlong = {
		       .lat = deg2rad(navPvt.lat / 1e7f),
		       .lon = deg2rad(navPvt.lon / 1e7f),
		       .alt = navPvt.height / 1000.0f };
  utm_of_lla_f(&utm, &latlong);

  commonGps = {
	       .rtcTime = rtcd,
	       .utm_east = static_cast<int32_t>(utm.east * 100),
	       .utm_north = static_cast<int32_t>(utm.north * 100),
	       .alt = navPvt.height,
	       .course = static_cast<int16_t>(navPvt.headMot / 10000),
	       .speed =  static_cast<uint16_t>(navPvt.gSpeed / 10U),
	       .climb =  static_cast<int16_t>(navPvt.velD / -10),
	       .utm_zone = utm.zone
    };
  
  blackBoard.write(commonGps);
}
