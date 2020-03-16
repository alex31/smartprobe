#pragma once
#include "workerClass.hpp"
#include "imu.hpp"
#include "sdcard.hpp"
#include "stdutil.h"
#include "hardwareConf.hpp"


class RtcSync : public WorkerThread<TH_RTCSYNC::threadStackSize,
					   RtcSync> {
public:
  RtcSync(const tprio_t m_prio) :
    WorkerThread<TH_RTCSYNC::threadStackSize, RtcSync>("rtcSync", m_prio) {};
  BlackBoard<Vec3f, UpdateBehavior::OnContentChange> blackBoard;
private:
  friend WorkerThread<TH_RTCSYNC::threadStackSize, RtcSync>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  event_listener_t pprzGpsEvent, nmeaGpsEvent;
};
