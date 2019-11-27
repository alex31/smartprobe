#include <ch.h>
#include <hal.h>
#include "hardwareConf.hpp"
#include "adc.hpp"
#include "stdutil.h"	
#include "sdLog.h"	

namespace {
  void adcErrorCb(ADCDriver *adcp, adcerror_t err);
  
  constexpr adcsample_t psVoltToSample(const float voltage)  {
    return (voltage * SAMPLE_MAX * DIVIDER_R6) /
      ((VCC_33 * (DIVIDER_R6 + DIVIDER_R7)));
  }

  constexpr ADCConversionGroup adcgrpcfg = {
  .circular     = true,   // continuous conversion
  .num_channels = Adc::ADC_GRP1_NUM_CHANNELS,
  .end_cb       = NULL, // adc completed callback,
  .error_cb     = &adcErrorCb, // adc error callback,
  .cr1		= ADC_CR1_AWDSGL | ADC_CR1_AWDEN | ADC_CR1_AWDIE |
		 (9 << ADC_CR1_AWDCH_Pos), 
  .cr2          = ADC_CR2_SWSTART,          
  .smpr1        = ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_480),
  .smpr2        = ADC_SMPR2_SMP_AN9(ADC_SAMPLE_480),
  .htr          = SAMPLE_MAX,
  .ltr          = psVoltToSample(PS_VOLTAGE_THRESHOLD),
  .sqr1         = 0,
  .sqr2         = 0,                                       
  .sqr3         = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN9) |
		  ADC_SQR3_SQ2_N(ADC_CHANNEL_SENSOR)
};

  BSEMAPHORE_DECL(adcWatchDogSem, true);
  EVENTSOURCE_DECL(eventSource);

  IN_DMA_SECTION_NOINIT(adcsample_t samples[Adc::ADC_GRP1_NUM_CHANNELS * Adc::ADC_GRP1_BUF_DEPTH]);



  void adcErrorCb(ADCDriver *adcp, adcerror_t err)
{
  (void) adcp;

  if (err == ADC_ERR_WATCHDOG) {
    chSysLockFromISR();
    chBSemSignalI(&adcWatchDogSem);
    chSysUnlockFromISR();
  }
}
  
} // end of anonymous namespace 

bool Adc::init()
{
  adcStart(&ADCD1, NULL);
  adcSTM32EnableTSVREFE();
  adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);

  return true;
}

[[noreturn]]
bool Adc::loop()
{
  /*
    Do no try to flush file, with 4.5 volts as condensator voltage, there is
    only 20 ms before hitting 3V, not enough time to flush buffer, but enough to
    cleanly umount to avoid dirty bit
  */

  chBSemWait(&adcWatchDogSem); // wait for powerLoss event
  
  stopAllPeripherals();
  sdLogFinish();
  chThdSleepMilliseconds(PowerLossAwakeTimeBeforeDeepSleep);
  systemDeepSleep();
  while (true);
}

  
float Adc::getPowerSupplyVoltage(void) const
{
  return (VCC_33 * samples[0] / SAMPLE_MAX) *
    ((DIVIDER_R6+DIVIDER_R7) / DIVIDER_R7);
}

float Adc::getCoreTemp(void) const
{
  return scaleTemp(samples[1]);
}

void  Adc::registerEvt(event_listener_t *lst, const eventmask_t events) const
{
  chEvtRegisterMask(&eventSource, lst, events);
}

float Adc::scaleTemp (int fromTmp) 
{
  const float sampleNorm = (float) fromTmp / SAMPLE_MAX;
  const float sampleVolt = sampleNorm * VCC_33;
  const float deltaVolt = sampleVolt - 0.76f;
  const float deltaCentigrade = deltaVolt / 2.5e-3f;
  const float temp = 25.0f + deltaCentigrade;

  return temp;
}

