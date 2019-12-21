#pragma once
#include "workerClass.hpp"
#include "imu.hpp"
#include "sdcard.hpp"
#include "stdutil.h"


namespace TH_RELWIND {
static constexpr size_t threadStackSize = 1536U;
}

struct AirSpeed {
  float velocity;
  float alpha;
  float beta;
  
  bool operator != (const AirSpeed &other) {
    return (velocity != other.velocity) || (alpha != other.alpha) || (beta != other.beta);
  };
};


class Relwind : public WorkerThread<TH_RELWIND::threadStackSize,
					   Relwind> {
public:
  Relwind(const tprio_t m_prio) :
    WorkerThread<TH_RELWIND::threadStackSize,
		 Relwind>("relwind", m_prio), timeStamp(chVTGetSystemTimeX()) {};
  BlackBoard<AirSpeed, UpdateBehavior::OnContentChange> blackBoard;
private:
  friend WorkerThread<TH_RELWIND::threadStackSize, Relwind>;
  systime_t timeStamp;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  event_listener_t pDiffEvent;
  AirSpeed  airSpeed;
};




