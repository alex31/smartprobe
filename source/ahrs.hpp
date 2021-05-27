#pragma once
#include "workerClass.hpp"
#include "imu.hpp"
#include "sdcard.hpp"
#include "stdutil.h"
#include "hardwareConf.hpp"


struct AttitudeEQ {
  Vec3f euler;
  Vec4f quat;
  bool operator!=(const AttitudeEQ &o)
  {
    return !vec3fIsEqual(&euler, &o.euler);
  };

};

class Ahrs : public WorkerThread<TH_AHRS::threadStackSize,
					   Ahrs> {
public:
  Ahrs(const tprio_t m_prio) :
    WorkerThread<TH_AHRS::threadStackSize,
		 Ahrs>("ahrs", m_prio), timeStamp(chVTGetSystemTimeX()) {};
  BlackBoard<AttitudeEQ, UpdateBehavior::OnContentChange> blackBoard;
private:
  friend WorkerThread<TH_AHRS::threadStackSize, Ahrs>;
  systime_t timeStamp;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  event_listener_t imuEvent;
  AttitudeEQ  attitude;
};




