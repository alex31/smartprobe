#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"


constexpr float rad2deg(const float rad) {return rad * 180.0f / static_cast<float>(M_PI);}

class Adc : public WorkerThread<TH_ADC::threadStackSize, Adc> {
public:
  Adc(const tprio_t m_prio) :
    WorkerThread<TH_ADC::threadStackSize, Adc>("adc", m_prio) {};
  float getPowerSupplyVoltage(void) const;
  float getCoreTemp(void) const;
  void  registerEvt(event_listener_t *lst, const eventmask_t events) const;
  
  static constexpr uint32_t  ADC_GRP1_NUM_CHANNELS = 2U;
  static constexpr uint32_t  ADC_GRP1_BUF_DEPTH = 1U;

private:
  friend WorkerThread<TH_ADC::threadStackSize, Adc>;
  bool init(void) final;
  bool calculateThreshold(void);
  [[noreturn]] bool loop(void) final;
  static constexpr float scaleTemp (int fromTmp);
  adcsample_t psThreshold = 0U;
};




