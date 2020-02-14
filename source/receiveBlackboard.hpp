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

class ReceiveBlackboard : public WorkerThread<TH_RECEIVEBLACKBOARD::threadStackSize,
					      ReceiveBlackboard> {
public:
  ReceiveBlackboard(const tprio_t m_prio) :
    WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
		 ReceiveBlackboard>("receiveBlackboard", m_prio) {};
  BlackBoard<GpsData, UpdateBehavior::Always> blackBoard;
private:
  friend WorkerThread<TH_SHOWBLACKBOARD::threadStackSize, ReceiveBlackboard>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
};




