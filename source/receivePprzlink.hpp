#pragma once
#include "workerClass.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "hardwareConf.hpp"

struct GpsData {
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

class ReceivePprzlink : public WorkerThread<TH_RECEIVEPPRZLINK::threadStackSize,
					      ReceivePprzlink> {
public:
  ReceivePprzlink(const tprio_t m_prio) :
    WorkerThread<TH_RECEIVEPPRZLINK::threadStackSize,
		 ReceivePprzlink>("receivePprzlink", m_prio) {};
  BlackBoard<GpsData, UpdateBehavior::Always> pprzlink;
private:
  friend WorkerThread<TH_RECEIVEPPRZLINK::threadStackSize, ReceivePprzlink>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
};




