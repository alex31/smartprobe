#pragma once
#include "workerClass.hpp"
#include "blackBoard.hpp"
#include "spiPeriphICM20600.h"
#include <array>

namespace TH_IMU {
static constexpr size_t threadStackSize = 1024U;
}

enum class ImuKindOfInit {BiasEstimation=0, Measure};
constexpr const char*  imuKindOfInitStr[] = {"Bias Estimation", "Measure"};

struct ImuData {
  Vec3f gyro, acc;
  float temp=0;

  ImuData operator+(const ImuData& other) {
    ImuData ret{};
    ret.gyro = vec3fAdd(&gyro, &other.gyro);
    ret.acc = vec3fAdd(&acc, &other.acc);
    return ret;
  }
  
  bool operator != (const ImuData &other) {
    return (!vec3fIsEqual(&gyro, &other.gyro)) || (!vec3fIsEqual(&acc, &other.acc));
  };
};

class Imu : public WorkerThread<TH_IMU::threadStackSize, Imu> {
public:
  Imu(const tprio_t m_prio) : 
    WorkerThread<TH_IMU::threadStackSize, Imu>("imu", m_prio) {};
  BlackBoard<ImuData, UpdateBehavior::OnContentChange> blackBoard;
  const Vec3f& getInitialAcceleration(void) {return bias.acc;};
private:
  friend WorkerThread<TH_IMU::threadStackSize, Imu>;
  bool init(void) final;
  bool loop(void) final;
  bool init20600(const ImuKindOfInit kind);
  bool estimateBiasAndPosition(void);
  ImuData wdata = {};
  ImuData bias = {};
  Icm20600Data icmd{};
};




