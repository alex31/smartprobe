#pragma once
#include "workerClass.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "hardwareConf.hpp"


struct CommonGpsData {
  int32_t     utm_east;
  int32_t     utm_north;
  int16_t     course;
  int32_t     alt;
  uint16_t    speed;
  int16_t     climb;        
  RTCDateTime rtcTime;
};

class ReceivePprzlink : public WorkerThread<TH_RECEIVEPPRZLINK::threadStackSize,
					      ReceivePprzlink> {
public:
  ReceivePprzlink(const tprio_t m_prio) :
    WorkerThread<TH_RECEIVEPPRZLINK::threadStackSize,
		 ReceivePprzlink>("receivePprzlink", m_prio) {};
  BlackBoard<CommonGpsData, UpdateBehavior::Always> blackBoard;
private:
  friend WorkerThread<TH_RECEIVEPPRZLINK::threadStackSize, ReceivePprzlink>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  event_listener_t gpsEvent;
};




