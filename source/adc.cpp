#include <ch.h>
#include <hal.h>
#include "stdutil.h"	
#include "hardwareConf.hpp"
#include "adc.hpp"
#include "sdLiteLog.hpp"	
#include "sdcard.hpp"	

namespace {
  void adcErrorCb(ADCDriver *adcp, adcerror_t err);
  
  constexpr adcsample_t psVoltToSample(const float voltage)  {
    return (voltage * SAMPLE_MAX * DIVIDER_R6) /
      ((VCC_33 * (DIVIDER_R6 + DIVIDER_R7)));
  }

  constexpr float sampleToPsVoltage(const adcsample_t sample ) {
    return (VCC_33 * sample / SAMPLE_MAX) *
      ((DIVIDER_R6+DIVIDER_R7) / DIVIDER_R7);
  }
  
  
  constexpr ADCConversionGroup adcgrpcfgThreshold = {
	.circular     = false,
	.num_channels = Adc::ADC_GRP1_NUM_CHANNELS,
	.end_cb       = NULL, // adc completed callback,
	.error_cb     = nullptr,
	.cr1		= 0,
	.cr2          = ADC_CR2_SWSTART,          
	.smpr1        = 0,
	.smpr2        = ADC_SMPR2_SMP_AN9(ADC_SAMPLE_480),
	.htr          = 0,
	.ltr          = 0,
	.sqr1         = 0,
	.sqr2         = 0,                                       
	.sqr3         = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN9) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN9)
  };

  ADCConversionGroup adcgrpcfg = {
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
	.ltr          = 0,
	.sqr1         = 0,
	.sqr2         = 0,                                       
	.sqr3         = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN9) |
	                ADC_SQR3_SQ2_N(ADC_CHANNEL_SENSOR)
  };
  
  BSEMAPHORE_DECL(adcWatchDogSem, true);
  EVENTSOURCE_DECL(eventSource);

  IN_DMA_SECTION_NOINIT(adcsample_t samples[Adc::ADC_GRP1_NUM_CHANNELS *
					    Adc::ADC_GRP1_BUF_DEPTH]);



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

  if (calculateThreshold() ==false)
    return false;
  
  adcgrpcfg.ltr = psThreshold;
  adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);

  return true;
}

bool Adc::calculateThreshold()
{
  constexpr size_t loop = 20'000U;

  chThdSleepMilliseconds(100); // wait for VSS to stabilize
  uint32_t accum = 0U;
  for (size_t i=0; i<loop; i++) {
    adcConvert(&ADCD1, &adcgrpcfgThreshold, samples, ADC_GRP1_BUF_DEPTH);
    accum += (samples[0] + samples[1]);
  }
  const float average = accum / (loop * 2.0f);
  const float averageVoltage = sampleToPsVoltage(average);
  psThreshold = average *((100.0f -  PS_VOLTAGE_THRESHOLD_PERCENT) /
			  100.0f);
  
  SdCard::logSyslog(Severity::Info, "Power supply Voltage = %.2f, "
		    "shutdown threshold voltage  = %.2f", 
		    double(averageVoltage), double(sampleToPsVoltage(psThreshold)));
  
  if (averageVoltage < PS_VOLTAGE_ABSOLUTE_MINIMUM) {
    SdCard::logSyslog(Severity::Fatal, "Power supply Voltage = %.2f is "
	      "less than absolute minimum %.2f", 
	      double(averageVoltage), double(PS_VOLTAGE_ABSOLUTE_MINIMUM));
    return false;
  }
  
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
  SdLiteLogBase::terminate(TerminateBehavior::DONT_WAIT);
  chThdSleepMilliseconds(PowerLossAwakeTimeBeforeDeepSleep);
  systemDeepSleep();
  while (true);
}

  
float Adc::getPowerSupplyVoltage(void) const
{
  return sampleToPsVoltage(samples[0]);
}

float Adc::getCoreTemp(void) const
{
  return scaleTemp(samples[1]);
}

void  Adc::registerEvt(event_listener_t *lst, const eventmask_t events) const
{
  chEvtRegisterMask(&eventSource, lst, events);
}

constexpr float Adc::scaleTemp (const int fromTmp) 
{
  const float sampleNorm = (float) fromTmp / SAMPLE_MAX;
  const float sampleVolt = sampleNorm * VCC_33;
  const float deltaVolt = sampleVolt - 0.76f;
  const float deltaCentigrade = deltaVolt / 2.5e-3f;
  const float temp = 25.0f + deltaCentigrade;

  return temp;
}

