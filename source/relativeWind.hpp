#pragma once
#include "workerClass.hpp"
#include "imu.hpp"
#include "sdcard.hpp"
#include "stdutil.h"
#include "hardwareConf.hpp"


struct AirSpeed {
  float tas;
  float eas;
  float alpha;
  float beta;
  
  bool operator != (const AirSpeed &other) {
    return (tas != other.tas) || (alpha != other.alpha) || (beta != other.beta);
  };

  const AirSpeed& clear(void) {
    tas = 0;
    eas = 0;
    alpha = 0;
    beta = 0;
    return *this;
  }
  const AirSpeed& operator /=(const float divisor) {
    tas /= divisor;
    eas /= divisor;
    alpha /= divisor;
    beta /= divisor;
    return *this;
  }
  const AirSpeed& operator +=(const AirSpeed& other) {
    tas += other.tas;
    eas += other.eas;
    alpha    += other.alpha;
    beta     += other.beta;
    return *this;
  }
  AirSpeed operator +(const AirSpeed& other) {
    AirSpeed res = *this;
    res.tas += other.tas;
    res.eas += other.eas;
    res.alpha    += other.alpha;
    res.beta     += other.beta;
    return res;
  }
  AirSpeed operator *(const float mult) {
    AirSpeed res = *this;
    res.tas *= mult;
    res.eas *= mult;
    res.alpha    *= mult;
    res.beta     *= mult;
    return res;
  }
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




