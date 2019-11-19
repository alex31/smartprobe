#pragma once
#include <array>
#include "workerClass.hpp"
#include "i2cMaster.h"



class Barometer : public WorkerThread<512U> {
public:
  Barometer(const tprio_t m_prio) : WorkerThread<512U>("barometer", m_prio) {};
private:
  bool init(void) final;
  bool loop(void) final;
  
  LPS33HWDriver lpsDriver;
};




