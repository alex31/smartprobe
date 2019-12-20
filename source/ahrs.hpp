#pragma once
#include "workerClass.hpp"
#include "imu.hpp"
#include "sdcard.hpp"
#include "stdutil.h"


namespace TH_AHRS {
static constexpr size_t threadStackSize = 1536U;
}


class Ahrs : public WorkerThread<TH_AHRS::threadStackSize,
					   Ahrs> {
public:
  Ahrs(const tprio_t m_prio) :
    WorkerThread<TH_AHRS::threadStackSize,
		 Ahrs>("ahrs", m_prio), timeStamp(chVTGetSystemTimeX()) {};
  BlackBoard<Vec3f, UpdateBehavior::OnContentChange> blackBoard;
private:
  friend WorkerThread<TH_AHRS::threadStackSize, Ahrs>;
  systime_t timeStamp;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  event_listener_t imuEvent;
  Vec3f attitude;
};




