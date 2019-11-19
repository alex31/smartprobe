#pragma once
#include "workerClass.hpp"
#include "spiPeriphICM20600.h"

namespace TH_IMU {
static constexpr size_t threadStackSize = 1024U;
}

class Imu : public WorkerThread<TH_IMU::threadStackSize, Imu> {
public:
  Imu(const tprio_t m_prio) :
    WorkerThread<TH_IMU::threadStackSize, Imu>("imu", m_prio) {};
private:
  bool init(void) final;
  bool loop(void) final;
  Icm20600Data icmd{};
};




