#pragma once
#include <array>
#include "workerClass.hpp"
#include "i2cMaster.h"

namespace TH_DIFFPRESS {
static constexpr size_t threadStackSize = 512U;
}


class DifferentialPressure :
  public WorkerThread<TH_DIFFPRESS::threadStackSize, DifferentialPressure> {
public:
  DifferentialPressure(const tprio_t m_prio) :
    WorkerThread<TH_DIFFPRESS::threadStackSize, DifferentialPressure>("diff pressure", m_prio) {};
private:
  bool init(void) final;
  bool loop(void) final;
  bool initSdp(Sdp3xDriver &sdp, const uint8_t numSlave);

  std::array<Sdp3xDriver, 3> sensorsd;
};




