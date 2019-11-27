#pragma once
#include "workerClass.hpp"
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"



namespace TH_SHOWBLACKBOARD {
static constexpr size_t threadStackSize = 512U;
}

class ShowBlackboard : public WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
					   ShowBlackboard> {
public:
  ShowBlackboard(const tprio_t m_prio) :
    WorkerThread<TH_SHOWBLACKBOARD::threadStackSize,
		 ShowBlackboard>("showBlackboard", m_prio) {};
private:
  friend WorkerThread<TH_SHOWBLACKBOARD::threadStackSize, ShowBlackboard>;
  enum  EventMask {BARO_EVT=1<<0, PDIF_EVT=1<<1, ADC_EVT=1<<2, IMU_EVT=1<<3}; 
  bool init(void) final;
  bool initThread(void) final;
  bool loop(void) final;

  event_listener_t baroEvent, diffPressEvent, imuEvent;
  BarometerData baroData{};
  DiffPressureData diffPressData{};
  ImuData imuData{};
};




