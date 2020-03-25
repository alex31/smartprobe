#include <iostream>
#include <fcntl.h> 
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <assert.h>
#include <cmath>
#include "ubxNavPvt.hpp"



//g++ -std=c++17 -Wall -Wextra ubxNavPvtTest.cpp
  

namespace {
  
  int init_file(const std::string& portName)
  {
   int fd = open(portName.c_str(), 0);
   if (fd < 0) {
     std::cerr << "Error opening " <<  portName << std::endl;
   }
   return fd;
  }
  

};

struct UbxNavPvt_valid {
  uint8_t validDate:1;
  uint8_t validTime:1;
  uint8_t fullyResolved:1;
  uint8_t validMag:1;
  uint8_t dummy:4;
};

struct UbxNavPvt_flag {
  uint8_t gnssFixOK:1;
  uint8_t diffSoln:1;
  uint8_t psmState:3;
  uint8_t headVehValid:1;
  uint8_t carrSoln:2;
};

struct UbxNavPvt_flag2 {
  uint8_t dummy:5;
  uint8_t confirmedAvai:1;
  uint8_t confirmedDate:1;
  uint8_t confirmedTime:1;
};


struct UbxNavPvt {
  uint32_t  itow;
  uint16_t  year;
  uint8_t   month;
  uint8_t   day;
  uint8_t   hour;
  uint8_t   min;
  uint8_t   sec;
  UbxNavPvt_valid valid;
  uint32_t  tAcc;
  int32_t   nano;
  uint8_t   fixType;
  UbxNavPvt_flag  flags;
  UbxNavPvt_flag2 flags2;
  uint8_t   numSv;
  int32_t   lon;
  int32_t   lat;
  int32_t   height;
  int32_t   hMSL;
  uint32_t  hAcc;
  uint32_t  vAcc;
  int32_t   velN;
  int32_t   velE;
  int32_t   velD;
  int32_t   gSpeed;
  int32_t   headMot;
  uint32_t  sAcc ;
  uint32_t  headAcc ;
  uint16_t  pDOP ;
  uint8_t   reserved1[6];
  int32_t   headVeh;
  int16_t   magDec;
  uint16_t  magAcc;
} __attribute__((packed));

static_assert(sizeof(UbxNavPvt) == 92);


int main(int argc, char* argv[])
{
  if (argc != 2) {
    printf ("usage : %s file.dat\n", argv[0]);
  }
  const std::string gpsDev = argv[1];
  
  int gpsFd = init_file(gpsDev);


  // uint8_t b;
  // size_t count =1;
  // while (count) {
  //   count = read(gpsFd, &b, sizeof(b));
  //   printf("%x\n",b);
  // }

  // exit(0);
  
  constexpr std::array<TopicLen_t, 2> topics = {{ {0x0104, 18}, {0x0107, sizeof(UbxNavPvt)} }};
  
  const auto readCB = [gpsFd] (std::byte *bpt, size_t len) -> int {
			const int res = read(gpsFd, bpt, len);
			//			printf("[%p] %x\n", bpt, static_cast<uint8_t>(*bpt));
			return res;
		      };
  
  const auto msgCb = [] (const TopicLen_t &toplen, const std::byte *buf) -> void {
		       (void) buf;
		       printf("receive message topic %x of len %u\n", 
			      toplen.t, toplen.l);
		       UbxNavPvt navPvt;
		       if (toplen.t != 0x0107)
			 return;

		       memcpy(&navPvt, buf, sizeof(navPvt));
		       printf ("%02d:%02d:%02d %02d/%02d/%04d lon=%d lat=%d height=%d\n",
			       navPvt.hour, navPvt.min, navPvt.sec, navPvt.day, navPvt.month, navPvt.year,
			       navPvt.lon, navPvt.lat, navPvt.height);
		       printf ("flags : validDate=%u validTime=%u fullyResolved=%u validMag=%u\n"
			       "        gnssFixOK=%u diffSoln=%u psmState=%u headVehValid=%u carrSoln=%u"
			       "        confirmedAvai=%u confirmedDate=%u confirmedTime=%u\n",
			       navPvt.valid.validDate, navPvt.valid.validTime,
			       navPvt.valid.fullyResolved, navPvt.valid.validMag, 
			       navPvt.flags.gnssFixOK, navPvt.flags.diffSoln, navPvt.flags.psmState,
			       navPvt.flags.headVehValid, navPvt.flags.carrSoln,
			       navPvt.flags2.confirmedAvai, navPvt.flags2.confirmedDate,
			       navPvt.flags2.confirmedTime);
		     };


  
  UbxNavPvtSerialDecoder<98, topics.size()> ubx(topics, msgCb, readCB);

  UbxError uerr = UbxError::OK;
  while (uerr != UbxError::READ) {
    uerr = ubx.step();
    if ((uerr != UbxError::OK) && (uerr != UbxError::TOPIC_NOT_BIND))  {
      printf ("error: %s\n", ubx.getLastError());
    }
  }

  return 0;
}


