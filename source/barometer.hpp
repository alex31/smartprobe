#pragma once
#include <array>
#include "workerClass.hpp"
#include "blackBoard.hpp"
#include "i2cMaster.h"
#include "hardwareConf.hpp"

struct BarometerData {
  float pressure;
  float temp;
  float rho;
  bool operator != (const BarometerData &other) {
    return (pressure != other.pressure) || (temp != other.temp) || (rho != other.rho);
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
  void estimateRho(void);
  float airSensorDelta{};
  BarometerData wdata = {};
  LPS33HWDriver lpsDriver;
};




