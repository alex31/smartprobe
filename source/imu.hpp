#pragma once
#include "workerClass.hpp"
#include "blackBoard.hpp"
#include "spiPeriphICM20600.h"

namespace TH_IMU {
static constexpr size_t threadStackSize = 1024U;
}

struct ImuData {
  Vec3f gyro, acc;
  float temp=0;

  bool operator != (const ImuData &other) {
    return (!vec3fIsEqual(&gyro, &other.gyro)) || (!vec3fIsEqual(&acc, &other.acc));
  };
};

class Imu : public WorkerThread<TH_IMU::threadStackSize, Imu> {
public:
  Imu(const tprio_t m_prio) :
    WorkerThread<TH_IMU::threadStackSize, Imu>("imu", m_prio) {};
  BlackBoard<ImuData, UpdateBehavior::OnContentChange> blackBoard;
private:
  friend WorkerThread<TH_IMU::threadStackSize, Imu>;
  bool init(void) final;
  bool loop(void) final;
  ImuData wdata = {};
  Icm20600Data icmd{};
};




