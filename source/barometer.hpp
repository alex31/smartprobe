#pragma once
#include <array>
#include "workerClass.hpp"
#include "blackBoard.hpp"
#include "i2cMaster.h"
#include "hardwareConf.hpp"
#include "windowsAverage.hpp"

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
  static constexpr uint16_t pressToInt(const float press) {return (press - 500.0f) * 100.0f;}
  static constexpr int16_t tempToInt(const float temp) {return temp * 100.0f;}
  static constexpr float pressToFloat(const uint16_t press) {return (press / 100.0f) + 500.0f;}
  static constexpr float tempToFloat(const int16_t temp) {return temp / 100.0f;}
  float airSensorDelta{};
  BarometerData wdata = {};
  LPS33HWDriver lpsDriver;
  WindowAverage<int16_t, BARO_TEMP_WINDOW_AVERAGE_SIZE> tempAverage;
  WindowAverage<uint16_t, BARO_TEMP_WINDOW_AVERAGE_SIZE> pressAverage;
};




