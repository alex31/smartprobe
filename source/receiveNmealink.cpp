#include <ch.h>
#include <hal.h>
#include "receiveNmealink.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "sdcard.hpp"
#include "pprz_geodetic_float.h"

typedef void (nmea_cb_t) (const void * const userData,
			  const uint32_t argc, const NmeaParam  * const argv);

namespace {
  struct DayMonthYear;
  uint8_t weekday(uint8_t month,  uint8_t day, uint16_t year);

  RTCDateTime zdaToRTCD(const DayMonthYear& dmy);

  double sideSign(const char side);

  double nmeaAngleToRad(const double nmeaAngle);
  
  nmea_cb_t gga_cb, zda_cb, vtg_cb, pubx00_cb;
  void error_cb (const NmeaError error, const void * const userData,
		 const char * const msg);

  
  const NmeaBinder nbs[] = {
  {.fieldClass = "$PUBX,00", .msgCb = &pubx00_cb,
   .field = {
	     {.fieldName = "utc time",     .fieldType = NMEA_DOUBLE,  .fieldIndex = 1}, // 0
	     {.fieldName = "latitude",     .fieldType = NMEA_DOUBLE,  .fieldIndex = 2}, // 1
	     {.fieldName = "nord/sud",     .fieldType = NMEA_CHAR,    .fieldIndex = 3}, // 2
	     {.fieldName = "longitude",    .fieldType = NMEA_DOUBLE,  .fieldIndex = 4}, // 3
	     {.fieldName = "est/ouest",    .fieldType = NMEA_CHAR,    .fieldIndex = 5}, // 4
	     {.fieldName = "Altitude",     .fieldType = NMEA_FLOAT,   .fieldIndex = 6}, // 5
	     {.fieldName = "status",       .fieldType = NMEA_INT,     .fieldIndex = 7}, // 6
	     {.fieldName = "SOG km/h",     .fieldType = NMEA_FLOAT,   .fieldIndex = 10}, // 7
	     {.fieldName = "COG",          .fieldType = NMEA_FLOAT,   .fieldIndex = 11}, // 8
	     {.fieldName = "Vel Down",     .fieldType = NMEA_FLOAT,   .fieldIndex = 13}, // 9 
	     {.fieldName = "hdop",         .fieldType = NMEA_FLOAT,   .fieldIndex = 14}, // 10
	     {.fieldName = "vdop",         .fieldType = NMEA_FLOAT,   .fieldIndex = 15}, // 11
	     {.fieldName = "nb sat",       .fieldType = NMEA_INT,     .fieldIndex = 17}, // 12
	     // *MANDATORY* marker of end of list
	     {.fieldIndex = 0}
    }
  },
  {.fieldClass = "$GNGGA", .msgCb = &gga_cb,
   .field = {
      {.fieldName = "utc time",     .fieldType = NMEA_DOUBLE,  .fieldIndex = 1},
      {.fieldName = "latitude",     .fieldType = NMEA_DOUBLE,  .fieldIndex = 2},
      {.fieldName = "nord/sud",     .fieldType = NMEA_CHAR,    .fieldIndex = 3},
      {.fieldName = "longitude",    .fieldType = NMEA_DOUBLE,  .fieldIndex = 4},
      {.fieldName = "est/ouest",    .fieldType = NMEA_CHAR,    .fieldIndex = 5},
      {.fieldName = "status",       .fieldType = NMEA_INT,     .fieldIndex = 6},
      {.fieldName = "nb sat",       .fieldType = NMEA_INT,     .fieldIndex = 7},
      {.fieldName = "hdop",         .fieldType = NMEA_FLOAT,   .fieldIndex = 8},
      {.fieldName = "Altitude",     .fieldType = NMEA_FLOAT,   .fieldIndex = 9},
      // *MANDATORY* marker of end of list
      {.fieldIndex = 0}
    }
  },
  {.fieldClass = "$GNZDA", .msgCb = &zda_cb,
   .field = {
      {.fieldName = "utc time",     .fieldType = NMEA_DOUBLE,  .fieldIndex = 1},
      {.fieldName = "day",   .fieldType = NMEA_INT,   .fieldIndex = 2},
      {.fieldName = "month", .fieldType = NMEA_INT,   .fieldIndex = 3},
      {.fieldName = "year",  .fieldType = NMEA_INT,   .fieldIndex = 4},
      // *MANDATORY* marker of end of list
      {.fieldIndex = 0}
    }
  },
  {.fieldClass = "$GNVTG", .msgCb = &vtg_cb,
   .field = {
      {.fieldName = "COG",          .fieldType = NMEA_FLOAT,   .fieldIndex = 1},
      {.fieldName = "COG mag",      .fieldType = NMEA_FLOAT,   .fieldIndex = 3},
      {.fieldName = "SOG km/h",     .fieldType = NMEA_FLOAT,   .fieldIndex = 7},
      {.fieldName = "mode indic",   .fieldType = NMEA_CHAR,    .fieldIndex = 9},
      // *MANDATORY* marker of end of list
      {.fieldIndex = 0}
    }
  },

  // *MANDATORY* marker of end of list
  {.fieldClass = nullptr, .msgCb = nullptr, .field={}}
};

  NmeaStateMachine sm{};
  CommonGpsData commonGps{};

  struct DayMonthYear {
    double   utc;
    uint16_t year;
    uint8_t  day;
    uint8_t  month;
  } dayMonthYear{};

  struct CogSog {
    float cog;
    float sog;
  } cogSog{};

  static ReceiveNmealink *rnl = nullptr; 
};




bool ReceiveNmealink::init()
{
  ReceiveBaselink::init();
  rnl = this;
  initStateMachine (&sm);
  
  return true;
}

  

bool ReceiveNmealink::loop()
{
  const int16_t  recb = sdGetTimeout(&ExtSD, TIME_MS2I(1000));
  if (recb < 0) {
    SdCard::logSyslog(Severity::Warning, "nmea link : 1 second without data from gps");
  } else {
    feedNmea(nbs, &sm, nullptr, (char) recb, error_cb);
  }
  return true;
}
  

namespace {
  void gga_cb ([[maybe_unused]] const void * const userData,
	       const uint32_t argc, const NmeaParam  * const argv)
  {
    // should verify that type of arg is what you use
    // tu be sure not to use bad field in the union
    chDbgAssert (argc == 9, "nmea callback num field error");
    chDbgAssert (argv[0].fieldDesc->fieldType == NMEA_DOUBLE, "nmea callback type of field error");
    chDbgAssert (argv[1].fieldDesc->fieldType == NMEA_DOUBLE, "nmea callback type of field error");
    chDbgAssert (argv[2].fieldDesc->fieldType == NMEA_CHAR  , "nmea callback type of field error");
    chDbgAssert (argv[3].fieldDesc->fieldType == NMEA_DOUBLE, "nmea callback type of field error");
    chDbgAssert (argv[4].fieldDesc->fieldType == NMEA_CHAR  , "nmea callback type of field error");
    chDbgAssert (argv[5].fieldDesc->fieldType == NMEA_INT   , "nmea callback type of field error");
    chDbgAssert (argv[6].fieldDesc->fieldType == NMEA_INT   , "nmea callback type of field error");
    chDbgAssert (argv[7].fieldDesc->fieldType == NMEA_FLOAT , "nmea callback type of field error");
    chDbgAssert (argv[8].fieldDesc->fieldType == NMEA_FLOAT , "nmea callback type of field error");

    UtmCoor_f utm{};
    LlaCoor_f latlong = {
			 .lat = float(nmeaAngleToRad(argv[1].f_d)*sideSign(argv[2].f_c)),
			 .lon = float(nmeaAngleToRad(argv[3].f_d)*sideSign(argv[4].f_c)),
			 .alt = argv[8].f_f};
    utm_of_lla_f(&utm, &latlong);
    commonGps = {
	       .rtcTime = zdaToRTCD(dayMonthYear),
	       .utm_east = static_cast<int32_t>(utm.east * 100),
	       .utm_north = static_cast<int32_t>(utm.north * 100),
	       .alt = static_cast<int32_t>(utm.alt * 1000),
	       .course =  static_cast<int16_t>(cogSog.cog * 10),
	       .speed =  static_cast<uint16_t>(cogSog.sog * 1000 / 36),
	       // TODO : recuperer climb dans PUBX,00
	       .climb = 0, 
	       .utm_zone = utm.zone
    };
    rnl->blackBoard.write(commonGps);
    SdCard::logSyslog(Severity::Info, "DEBUG> UTM: east = %ld north = %ld zone=%u "
		      "LATLONG(rad) lat=%.6f long=%.6f alt=%.1f",
		      commonGps.utm_east, commonGps.utm_north, commonGps.utm_zone,
		      static_cast<double>(latlong.lat),
		      static_cast<double>(latlong.lon),
		      static_cast<double>(latlong.alt)
		      );
    
  };
  

  

  void zda_cb ([[maybe_unused]] const void * const userData,
	       const uint32_t argc, const NmeaParam  * const argv)
  {
    // should verify that type of arg is what you use
    // tu be sure not to use bad field in the union
    chDbgAssert (argc == 4, "nmea callback num field error");
    chDbgAssert (argv[0].fieldDesc->fieldType == NMEA_DOUBLE, "nmea callback type of field error");
    chDbgAssert (argv[1].fieldDesc->fieldType == NMEA_INT, "nmea callback type of field error");
    chDbgAssert (argv[2].fieldDesc->fieldType == NMEA_INT, "nmea callback type of field error");
    chDbgAssert (argv[3].fieldDesc->fieldType == NMEA_INT, "nmea callback type of field error");
  
    dayMonthYear = {
		    .utc = argv[0].f_d,
		    .year = static_cast<uint16_t>(argv[3].f_i),
		    .day = static_cast<uint8_t>(argv[1].f_i),
		    .month =  static_cast<uint8_t>(argv[2].f_i)};


     SdCard::logSyslog(Severity::Info, "DEBUG> dayMonthYear: "
		       "utc = %.2f Y=%d M=%d D=%d",
		       dayMonthYear.utc,
		       dayMonthYear.year,
		       dayMonthYear.month,
		       dayMonthYear.day);

     }
  
  void vtg_cb ([[maybe_unused]] const void * const userData,
		 const uint32_t argc, const NmeaParam * const argv)
  {
    // should verify that type of arg is what you use
    // tu be sure not to use bad field in the union
    chDbgAssert (argc == 4, "nmea callback num field error");
    chDbgAssert (argv[0].fieldDesc->fieldType == NMEA_FLOAT, "nmea callback type of field error");
    chDbgAssert (argv[1].fieldDesc->fieldType == NMEA_FLOAT, "nmea callback type of field error");
    chDbgAssert (argv[2].fieldDesc->fieldType == NMEA_FLOAT, "nmea callback type of field error");
    chDbgAssert (argv[3].fieldDesc->fieldType == NMEA_CHAR, "nmea callback type of field error");

    cogSog = {
	      .cog = argv[0].f_f,
	      .sog = argv[2].f_f
    };



    SdCard::logSyslog(Severity::Info, "DEBUG> cog/sog: "
		      "cog = %.2f sog = %.2f",
		      static_cast<double>(cogSog.cog),
		      static_cast<double>(cogSog.sog));
  }
  
  void pubx00_cb ([[maybe_unused]] const void * const userData,
		  const uint32_t argc, const NmeaParam  * const argv)
  {
    chDbgAssert (argc == 13, "nmea callback num field error");
    chDbgAssert (argv[0].fieldDesc->fieldType == NMEA_DOUBLE, "nmea callback type of field error");
    chDbgAssert (argv[1].fieldDesc->fieldType == NMEA_DOUBLE, "nmea callback type of field error");
    chDbgAssert (argv[2].fieldDesc->fieldType == NMEA_CHAR  , "nmea callback type of field error");
    chDbgAssert (argv[3].fieldDesc->fieldType == NMEA_DOUBLE, "nmea callback type of field error");
    chDbgAssert (argv[4].fieldDesc->fieldType == NMEA_CHAR  , "nmea callback type of field error");
    chDbgAssert (argv[5].fieldDesc->fieldType == NMEA_FLOAT , "nmea callback type of field error");
    chDbgAssert (argv[6].fieldDesc->fieldType == NMEA_INT   , "nmea callback type of field error");
    chDbgAssert (argv[7].fieldDesc->fieldType == NMEA_FLOAT , "nmea callback type of field error");
    chDbgAssert (argv[8].fieldDesc->fieldType == NMEA_FLOAT , "nmea callback type of field error");
    chDbgAssert (argv[9].fieldDesc->fieldType == NMEA_FLOAT , "nmea callback type of field error");
    chDbgAssert (argv[10].fieldDesc->fieldType == NMEA_FLOAT, "nmea callback type of field error");
    chDbgAssert (argv[11].fieldDesc->fieldType == NMEA_FLOAT, "nmea callback type of field error");
    chDbgAssert (argv[12].fieldDesc->fieldType == NMEA_INT  , "nmea callback type of field error");
    
    cogSog = {
	      .cog = argv[8].f_f,
	      .sog = argv[7].f_f
    };

    UtmCoor_f utm{};
    LlaCoor_f latlong = {
			 .lat = float(nmeaAngleToRad(argv[1].f_d)*sideSign(argv[2].f_c)),
			 .lon = float(nmeaAngleToRad(argv[3].f_d)*sideSign(argv[4].f_c)),
			 .alt = argv[6].f_f};
    utm_of_lla_f(&utm, &latlong);
    commonGps = {
	       .rtcTime = zdaToRTCD(dayMonthYear),
	       .utm_east = static_cast<int32_t>(utm.east * 100),
	       .utm_north = static_cast<int32_t>(utm.north * 100),
	       .alt = static_cast<int32_t>(utm.alt * 1000),
	       .course =  static_cast<int16_t>(cogSog.cog * 10),
	       .speed =  static_cast<uint16_t>(cogSog.sog * 1000 / 36),
	       .climb =  static_cast<int16_t>(argv[3].f_f * -100), 
	       .utm_zone = utm.zone
    };
    rnl->blackBoard.write(commonGps);
    SdCard::logSyslog(Severity::Info, "DEBUG> PUBX UTM: east = %ld north = %ld zone=%u "
		      "LATLONG(rad) lat=%.6f long=%.6f alt=%.1f vd=%d",
		      commonGps.utm_east, commonGps.utm_north, commonGps.utm_zone,
		      static_cast<double>(latlong.lat),
		      static_cast<double>(latlong.lon),
		      static_cast<double>(latlong.alt),
		      commonGps.climb
		      );
 

  };

  void error_cb (const NmeaError error, const void * const userData,
		 const char * const msg) 
  {
    (void) userData;
    SdCard::logSyslog(Severity::Warning,
	      "NMEA error of type %s occurs on message %s",
	      error ==  NMEA_CHKSUM_ERR ? "Checksum" : 
	      error == NMEA_TIMOUT_ERR ? "Timeout" : 
	      error == NMEA_OVERLENGTH_ERR ? "Overlength" : 
	      error == NMEA_OVERFIELD_ERR ? "OverField" : "Unknow",
	      msg);
  }
  
  
  uint8_t weekday(uint8_t month,  uint8_t day, uint16_t year)
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


  double sideSign(const char side)
  {
    switch (side) {
    case 'E':
    case 'N':
      return 1.0;
    case 'S':
    case 'W':
      return -1.0;
    default:
      return 0.0;
    };
  }

  double nmeaAngleToRad(const double nmeaAngle)
  {
    const double deg = floor(nmeaAngle/100.0);
    const double min = nmeaAngle-(deg*100.0);
    const double fracDeg = deg+(min/60.0);
    return deg2rad(fracDeg);
  }

  

  // uint32_t      year: 8;            /**< @brief Years since 1980.           */
  // uint32_t      month: 4;           /**< @brief Months 1..12.               */
  // uint32_t      dstflag: 1;         /**< @brief DST correction flag.        */
  // uint32_t      dayofweek: 3;       /**< @brief Day of week 1..7.           */
  // uint32_t      day: 5;             /**< @brief Day of the month 1..31.     */
  // uint32_t      millisecond: 27;    /**< @brief Milliseconds since midnight.*/

  RTCDateTime zdaToRTCD(const DayMonthYear& dmy)
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
  
}
