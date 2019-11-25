#pragma once
#include "workerClass.hpp"

namespace TH_SHOWBLACKBOARD {
static constexpr size_t threadStackSize = 320U;
}

class ShowBlackboard : public WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
					   ShowBlackboard> {
public:
  ShowBlackboard(const tprio_t m_prio) :
    WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
		 ShowBlackboard>("showBlackboard", m_prio) {};
private:
  bool init(void) final;
  bool loop(void) final;

  event_listener_t baroEvent;
};




