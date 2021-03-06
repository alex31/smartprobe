#pragma once
#include "workerClass.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "hardwareConf.hpp"


struct CommonGpsData {
  RTCDateTime rtcTime;
  int32_t     utm_east;  // cm
  int32_t     utm_north; // cm
  int32_t     alt;       // mm 	Altitude above geoid
  int16_t     course;    // decideg
  uint16_t    speed;     // cm/s norm of 2d ground speed in cm/s
  int16_t     climb;     // cm/s       
  uint8_t     utm_zone;
};

struct DayMonthYear {
  double   utc;
  uint16_t year;
  uint8_t  day;
  uint8_t  month;
};

class ReceiveBaselink : public WorkerThread<TH_RECEIVEBASELINK::threadStackSize,
					      ReceiveBaselink> {
public:
  ReceiveBaselink(const tprio_t m_prio) :
    WorkerThread<TH_RECEIVEBASELINK::threadStackSize,
		 ReceiveBaselink>("receiveBaselink", m_prio) {};
  BlackBoard<CommonGpsData, UpdateBehavior::Always> blackBoard;
  static  uint8_t weekday(uint8_t month,  uint8_t day, uint16_t year);
  static  double gpsAngleToRad(const double gpsAngle);
  static  RTCDateTime dmyToRTCD(const DayMonthYear& dmy);
protected:
  virtual bool init(void);
  virtual bool initInThreadContext(void) {return true;};
private:
  friend WorkerThread<TH_RECEIVEBASELINK::threadStackSize, ReceiveBaselink>;
  virtual bool loop(void)=0;
};




