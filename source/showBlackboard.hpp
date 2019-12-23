#pragma once
#include "workerClass.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"



namespace TH_SHOWBLACKBOARD {
static constexpr size_t threadStackSize = 1536U;
}

class ShowBlackboard : public WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
					   ShowBlackboard> {
public:
  ShowBlackboard(const tprio_t m_prio) :
    WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
		 ShowBlackboard>("showBlackboard", m_prio) {};
private:
  friend WorkerThread<TH_SHOWBLACKBOARD::threadStackSize, ShowBlackboard>;
  bool init(void) final;
  bool loop(void) final;
};




