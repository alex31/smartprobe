#pragma once
#include <array>
#include "workerClass.hpp"
#include "blackBoard.hpp"
#include "i2cMaster.h"

namespace TH_BARO {
  static constexpr size_t threadStackSize = 1512U;
}

struct BarometerData {
  float pressure;
  float temp;
  bool operator != (const BarometerData &other) {
    return (pressure != other.pressure) || (temp != other.temp);
  };
};

class Barometer : public WorkerThread<TH_BARO::threadStackSize, Barometer> {
public:
  Barometer(const tprio_t m_prio) :
    WorkerThread<TH_BARO::threadStackSize, Barometer>("barometer", m_prio) {};
  BlackBoard<BarometerData, UpdateBehavior::OnContentChange> blackBoard;
private:
  friend WorkerThread<TH_BARO::threadStackSize, Barometer>;
  bool init(void) final;
  bool loop(void) final;
  BarometerData wdata = {};
  LPS33HWDriver lpsDriver;
};




