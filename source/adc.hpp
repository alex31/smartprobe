#pragma once
#include "workerClass.hpp"


namespace TH_ADC {
static constexpr size_t threadStackSize = 320U;
}

class Adc : public WorkerThread<TH_ADC::threadStackSize, Adc> {
public:
  Adc(const tprio_t m_prio) :
    WorkerThread<TH_ADC::threadStackSize, Adc>("adc", m_prio) {};
  float getPowerSupplyVoltage(void) const;
  float getCoreTemp(void) const;

  static constexpr uint32_t  ADC_GRP1_NUM_CHANNELS = 2U;
  static constexpr uint32_t  ADC_GRP1_BUF_DEPTH = 1U;
  static constexpr adcsample_t psVoltToSample(const float voltage);


private:
  bool init(void) final;
  bool loop(void) final;
  static float scaleTemp (int fromTmp);

  static adcsample_t samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
};




