#include <iostream>
#include "nmeaFrame.h"
#include <fcntl.h> 
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <assert.h>
#include <cmath>
#include "paparazzi/math/pprz_geodetic_float.h"
//#include "sw/airborne/math/pprz_geodetic_float.h"

//g++ -std=c++17 -Wall -Wextra -I.  -I/home/alex/DEV/STM32/CHIBIOS/COMMON/various nmeaFrame.c /home/alex/DEV/STM32/CHIBIOS/COMMON/various/paparazzi/math/pprz_geodetic_float.c nmeaUnitTest.cpp

//g++ -std=c++17 -Wall -Wextra -I.  -I/home/alex/DEV/PAPARAZZI/paparazzi nmeaFrame.c -I/home/alex/DEV/PAPARAZZI/paparazzi/sw/include -I/home/alex/DEV/PAPARAZZI/paparazzi/sw/airborne /home/alex/DEV/PAPARAZZI/paparazzi/sw/airborne/math/pprz_geodetic_float.c nmeaUnitTest.cpp


typedef void (nmea_cb_t) (const void * const userData,
			  const uint32_t argc, const NmeaParam  * const argv);

namespace {
  struct DayMonthYear;

  template <typename T>
  constexpr T deg2rad(const T deg) {return deg * static_cast<T>(M_PI) / static_cast<T>(180.0);}

  double sideSign(const char side);
  nmea_cb_t zda_cb, pubx00_cb;
  double nmeaAngleToRad(const double nmeaAngle);
  void error_cb (const NmeaError error, const void * const userData,
		 const char * const msg);

  
  const NmeaBinder nbs[] = {
   /*
    Parameter Value	    Unit		Description
    2 UTC	                    hhmmss.sss		Universal time coordinated
    3 Lat	                    ddmm.mmmm		Latitude
    4 Northing Indicator	    N=North, S=South
    5 Lon	                    dddmm.mmmm		Longitude
    6 Easting Indicator	    E=East, W=West
    7 Alt (HAE)	            m			Altitude (above ellipsoid)
    8 Status		    NF=No Fix,...
    9 Horizontal Accuracy	    m	                Horizontal accuracy
    10 Vertical Accuracy	    m	                Vertical accuracy
    11 SOG	                    km/h	        Speed Over Ground
    12 COG (true)	            *	                Course Over Ground (true)
    13 VD	                    m/s	                Velocity Down
    14 Age of DGPS Corr	    s	                Age of Differentiel Corrections
    15 HDOP		                        Horizontal Dillution of Precision
    16 VDOP		                        Vertical Dillution of Precision
    17 TDOP		                        Time Dillution of Precision
    18 SVs Used		                        Number of SVs used for Navigation
    19 DR Status		                        Dead Reckon Status Flags
   */

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
  {.fieldClass = "$G?ZDA", .msgCb = &zda_cb,
   .field = {
      {.fieldName = "utc time",     .fieldType = NMEA_DOUBLE,  .fieldIndex = 1},
      {.fieldName = "day",   .fieldType = NMEA_INT,   .fieldIndex = 2},
      {.fieldName = "month", .fieldType = NMEA_INT,   .fieldIndex = 3},
      {.fieldName = "year",  .fieldType = NMEA_INT,   .fieldIndex = 4},
      // *MANDATORY* marker of end of list
      {.fieldIndex = 0}
    }
  },

  // *MANDATORY* marker of end of list
  {.fieldClass = nullptr, .msgCb = nullptr, .field={}}
  };



static inline constexpr int getLinuxBaudRate(const uint32_t bauds)
{
  switch (bauds) {
  case 50       : return B50     ;    
  case 75       : return B75     ;     
  case 110      : return B110    ;     
  case 134      : return B134    ;    
  case 150      : return B150    ;    
  case 200      : return B200    ;    
  case 300      : return B300    ;     
  case 600      : return B600    ;   
  case 1200     : return B1200   ;   
  case 1800     : return B1800   ;  
  case 2400     : return B2400   ;  
  case 4800     : return B4800   ;  
  case 9600     : return B9600   ;  
  case 19200    : return B19200  ;  
  case 38400    : return B38400  ; 
  case 57600    : return B57600  ; 
  case 115200   : return B115200 ;
  case 230400   : return B230400 ; 
  case 460800   : return B460800 ;
  case 500000   : return B500000 ;
  case 576000   : return B576000 ;
  case 921600   : return B921600 ;
  case 1000000  : return B1000000;
  case 1152000  : return B1152000;
  case 1500000  : return B1500000;
  case 2000000  : return B2000000;
  case 2500000  : return B2500000;
  case 3000000  : return B3000000;
  case 3500000  : return B3500000;
  case 4000000  : return B4000000;
  default :
    std::cerr << "baud rate " << bauds << " not standard, aborting\n";
    abort();
    return 0;
  }
}

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 255;
    tty.c_cc[VTIME] = 255;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

  
  int init_file(const std::string& portName)
  {
   int fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
   if (fd < 0) {
     std::cerr << "Error opening " <<  portName << std::endl;
   }
   return fd;
  }
  
  [[maybe_unused]]
  int init_uart(const std::string& portName, const uint32_t baudRate)
  {
   int fd = init_file(portName);
   
   if (fd >= 0) {
     /*baudrate 115200, 8 bits, no parity, 1 stop bit */
     if (set_interface_attribs(fd, getLinuxBaudRate(baudRate)) < 0)
       return fd;
     
     fcntl(fd, F_SETFL, 0);
   }
   
   return fd;
  }

};




int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  NmeaStateMachine sm{};
  const std::string gpsDev = argv[1];
  
  int gpsFd = init_file(gpsDev);
  initStateMachine (&sm);

  bool eof = false;
  while(not eof) {
    std::byte  buf;
    int retv = read(gpsFd, &buf, sizeof(buf));
    if (retv == static_cast<int>(sizeof(buf))) {
       feedNmea(nbs, &sm, nullptr, (char) buf, &error_cb);
    } else {
      eof = true;
    }
  }
  
}





namespace {
  void zda_cb ([[maybe_unused]] const void * const userData,
	       const uint32_t argc, const NmeaParam  * const argv)
  {
    assert (argc == 4);
    assert (argv[0].fieldDesc->fieldType == NMEA_DOUBLE);
    assert (argv[1].fieldDesc->fieldType == NMEA_INT);
    assert (argv[2].fieldDesc->fieldType == NMEA_INT);
    assert (argv[3].fieldDesc->fieldType == NMEA_INT);

    std::cout << "ZDA utc = " << argv[0].f_d << std::endl;
  }

  /*
    Parameter Value	    Unit		Description
    UTC	                    hhmmss.sss		Universal time coordinated
    Lat	                    ddmm.mmmm		Latitude
    Northing Indicator	    N=North, S=South
    Lon	                    dddmm.mmmm		Longitude
    Easting Indicator	    E=East, W=West
    Alt (HAE)	            m			Altitude (above ellipsoid)
    Status		    NF=No Fix,...
    Horizontal Accuracy	    m	                Horizontal accuracy
    Vertical Accuracy	    m	                Vertical accuracy
    SOG	                    km/h	        Speed Over Ground
    COG (true)	            *	                Course Over Ground (true)
    VD	                    m/s	                Velocity Down
    Age of DGPS Corr	    s	                Age of Differentiel Corrections
    HDOP		                        Horizontal Dillution of Precision
    VDOP		                        Vertical Dillution of Precision
    TDOP		                        Time Dillution of Precision
    SVs Used		                        Number of SVs used for Navigation
    DR Status		                        Dead Reckon Status Flags
   */


  
  void pubx00_cb ([[maybe_unused]] const void * const userData,
		  const uint32_t argc, const NmeaParam  * const argv)
  {
    assert (argc == 13);
    assert (argv[0].fieldDesc->fieldType == NMEA_DOUBLE);
    assert (argv[1].fieldDesc->fieldType == NMEA_DOUBLE);
    assert (argv[2].fieldDesc->fieldType == NMEA_CHAR  );
    assert (argv[3].fieldDesc->fieldType == NMEA_DOUBLE);
    assert (argv[4].fieldDesc->fieldType == NMEA_CHAR  );
    assert (argv[5].fieldDesc->fieldType == NMEA_FLOAT );
    assert (argv[6].fieldDesc->fieldType == NMEA_INT   );
    assert (argv[7].fieldDesc->fieldType == NMEA_FLOAT );
    assert (argv[8].fieldDesc->fieldType == NMEA_FLOAT );
    assert (argv[9].fieldDesc->fieldType == NMEA_FLOAT );
    assert (argv[10].fieldDesc->fieldType == NMEA_FLOAT);
    assert (argv[11].fieldDesc->fieldType == NMEA_FLOAT);
    assert (argv[12].fieldDesc->fieldType == NMEA_INT  );
    
    std::cout << "PUBX utc = " << argv[0].f_d
	      << " lat= " << argv[1].f_d * sideSign(argv[2].f_c) 
	      << " lon = " << argv[3].f_d * sideSign(argv[4].f_c)
	      << " alt = " << argv[5].f_f
	      << " velocity down = " << argv[9].f_f
	      << " nb sat = " << argv[12].f_i
	      <<  std::endl;



    UtmCoor_f utm{};
    LlaCoor_f latlong = {
			 .lat = float(nmeaAngleToRad(argv[1].f_d)*sideSign(argv[2].f_c)),
			 .lon = float(nmeaAngleToRad(argv[3].f_d)*sideSign(argv[4].f_c)),
			 .alt = argv[5].f_f};
 
    utm_of_lla_f(&utm, &latlong);
    std::cout << "UTM east = " << utm.east * 100
	      << " north = "   << utm.north * 100
	      << " zone = "    << utm.zone+0
	      << " alt = "     << utm.alt
	      <<  std::endl;;
  };

  void error_cb ([[maybe_unused]] const NmeaError error, [[maybe_unused]] const void * const userData,
		 const char * const msg) 
  {
    std::cout <<  "NMEA error occurs on message " << msg << std::endl;
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

}

