#pragma once
#include "workerClass.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "hardwareConf.hpp"

class TransmitBlackboard : public WorkerThread<TH_TRANSMITBLACKBOARD::threadStackSize,
					   TransmitBlackboard> {
public:
  TransmitBlackboard(const tprio_t m_prio) :
    WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
		 TransmitBlackboard>("transmitBlackboard", m_prio) {};
private:
  friend WorkerThread<TH_SHOWBLACKBOARD::threadStackSize, TransmitBlackboard>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
};




