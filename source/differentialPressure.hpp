#pragma once
#include <array>
#include "workerClass.hpp"
#include "blackBoard.hpp"
#include "i2cMaster.h"
#include "hardwareConf.hpp"

struct DiffPressureData {
  struct PressTemp {
    float pressure;
    float temp;

    bool operator == (const PressTemp &other) const {
      return (pressure == other.pressure) && (temp == other.temp);
    };
  };
  
  std::array<PressTemp, 3> pt;
  PressTemp& operator[](const size_t index) {
    return pt[index];
  }
  
  bool operator != (const DiffPressureData &other) const {
    return (pt != other.pt);
  };
 
};


class DifferentialPressure :
  public WorkerThread<TH_DIFFPRESS::threadStackSize, DifferentialPressure> {
public:
  DifferentialPressure(const tprio_t m_prio) :
    WorkerThread<TH_DIFFPRESS::threadStackSize, DifferentialPressure>("diff pressure", m_prio)
  {};
  BlackBoard<DiffPressureData, UpdateBehavior::OnContentChange> blackBoard;
private:
  friend  WorkerThread<TH_DIFFPRESS::threadStackSize, DifferentialPressure>;
  bool init(void) final;
  bool loop(void) final;
  bool initSdp(Sdp3xDriver &sdp, const uint8_t numSlave);
  DiffPressureData wdata {};
  std::array<Sdp3xDriver, 3> sensorsd;
};




