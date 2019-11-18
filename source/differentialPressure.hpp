#pragma once
#include <array>
#include "workerClass.hpp"
#include "i2cMaster.h"

class DifferentialPressure : public WorkerThread<512U> {
public:
  DifferentialPressure(const tprio_t m_prio) : WorkerThread<512U>("diff pressure", m_prio) {};
private:
  bool init(void) final;
  bool loop(void) final;
  bool initSdp(Sdp3xDriver &sdp, const uint8_t numSlave);

  std::array<Sdp3xDriver, 3> sensorsd;
};




