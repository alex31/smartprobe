#pragma once
#include "workerClass.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "hardwareConf.hpp"

class ReceiveBlackboard : public WorkerThread<TH_RECEIVEBLACKBOARD::threadStackSize,
					   ReceiveBlackboard> {
public:
  ReceiveBlackboard(const tprio_t m_prio) :
    WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
		 ReceiveBlackboard>("receiveBlackboard", m_prio) {};
private:
  friend WorkerThread<TH_SHOWBLACKBOARD::threadStackSize, ReceiveBlackboard>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
};




