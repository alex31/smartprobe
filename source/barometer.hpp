#pragma once
#include <array>
#include "workerClass.hpp"
#include "i2cMaster.h"

namespace TH_BARO {
  static constexpr size_t threadStackSize = 512U;
}

class Barometer : public WorkerThread<TH_BARO::threadStackSize, Barometer> {
public:
  Barometer(const tprio_t m_prio) :
    WorkerThread<TH_BARO::threadStackSize, Barometer>("barometer", m_prio) {};
private:
  bool init(void) final;
  bool loop(void) final;
  
  LPS33HWDriver lpsDriver;
};




