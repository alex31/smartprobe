#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"

class DemoLed : public WorkerThread<TH_DEMOLED::threadStackSize,
				    DemoLed> {
public:
  DemoLed(const tprio_t m_prio) :
    WorkerThread<TH_DEMOLED::threadStackSize,
		 DemoLed>("demoLed", m_prio) {};
private:
  friend WorkerThread<TH_DEMOLED::threadStackSize, DemoLed>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
};




