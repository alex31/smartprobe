#pragma once
#include "receiveBaselink.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "hardwareConf.hpp"




class ReceivePprzlink : public ReceiveBaselink {
public:
  ReceivePprzlink(const tprio_t m_prio) :
    ReceiveBaselink(m_prio) {};
private:
  friend WorkerThread<TH_RECEIVEBASELINK::threadStackSize, ReceivePprzlink>;
  bool init(void) final;
  bool loop(void) final;
};




