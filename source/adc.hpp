#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include "util.hpp"



class Adc : public WorkerThread<TH_ADC::threadStackSize, Adc> {
public:
  Adc(const tprio_t m_prio) :
    WorkerThread<TH_ADC::threadStackSize, Adc>("adc", m_prio) {};
  float getPowerSupplyVoltage(void) const;
  float getCoreTemp(void) const;
  float getSensorsVoltage(void) const;
  float getCoreVdd(void) const;
  void  registerEvt(event_listener_t *lst, const eventmask_t events) const;
  
  static constexpr uint32_t  ADC_GRP1_NUM_CHANNELS = 4U;
  static constexpr uint32_t  ADC_GRP1_BUF_DEPTH = 1U;
  
  static  float vrefCalib(void) {return 3.3f * (*vrefCalib33Ptr) / SAMPLE_MAX;}

private:
  friend WorkerThread<TH_ADC::threadStackSize, Adc>;
  bool init(void) final;
  bool calculateThreshold(void);
  [[noreturn]] bool loop(void) final;
  static constexpr float scaleTemp (int fromTmp);
  adcsample_t psThreshold = 0U;
};




