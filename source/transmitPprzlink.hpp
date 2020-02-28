#pragma once
#include "workerClass.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "hardwareConf.hpp"

class TransmitPprzlink : public WorkerThread<TH_TRANSMITPPRZLINK::threadStackSize,
					   TransmitPprzlink> {
public:
  TransmitPprzlink(const tprio_t m_prio) :
    WorkerThread<TH_TRANSMITPPRZLINK::threadStackSize,
		 TransmitPprzlink>("transmitPprzlink", m_prio) {};
private:
  friend WorkerThread<TH_TRANSMITPPRZLINK::threadStackSize, TransmitPprzlink>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
};




