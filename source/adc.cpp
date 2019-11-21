#include <ch.h>
#include <hal.h>
#include "hardwareConf.hpp"
#include "adc.hpp"
#include "stdutil.h"	
#include "sdLog.h"	

namespace {
  void adcErrorCb(ADCDriver *adcp, adcerror_t err);
  
  constexpr adcsample_t psVoltToSample(const float voltage)  {
    return (voltage * SAMPLE_MAX * DIVIDER_R4) /
      ((VCC_33 * (DIVIDER_R4 + DIVIDER_R14)));
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
  .sqr3         = ADC_SQR3_SQ1_N(ADC_CHANNEL_SENSOR) |
		  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN9)
};

  BSEMAPHORE_DECL(adcWatchDogSem, true);
  static THD_WORKING_AREA(waAdcWatchdog, 512);

  void adcErrorCb(ADCDriver *adcp, adcerror_t err)
{
  (void) adcp;

  if (err == ADC_ERR_WATCHDOG) {
    chSysLockFromISR();
    chBSemSignalI(&adcWatchDogSem);
    chSysUnlockFromISR();
  }
}
  
[[ noreturn ]]
  static void adcWatchdog (void *arg)
  {
    (void)arg;
    chRegSetThreadName("surveyAdc");

    chBSemWait(&adcWatchDogSem);
    for (const ioline_t line : LineToStopInCaseOfPowerFailure) {
      palSetLineMode(line, PAL_MODE_INPUT);
    }

    sdLogCloseAllLogs(LOG_FLUSH_BUFFER);
    sdLogFinish();
    systemDeepSleep();
    while (true);
  }
  
  
}

bool Adc::init()
{
  adcStart(&ADCD1, NULL);
  adcSTM32EnableTSVREFE();
  adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
  chThdCreateStatic(waAdcWatchdog, sizeof(waAdcWatchdog), HIGHPRIO,
		    &adcWatchdog, NULL);

  return true;
}


bool Adc::loop()
{
  DebugTrace("ADC = %.2f; temp = %.1f",
	     getPowerSupplyVoltage(), getCoreTemp());
  chThdSleepMilliseconds(2500);
  
  return true;
}

  
adcsample_t Adc::samples[Adc::ADC_GRP1_NUM_CHANNELS * Adc::ADC_GRP1_BUF_DEPTH];


float Adc::getPowerSupplyVoltage(void) const
{
  return (VCC_33 * samples[1] / SAMPLE_MAX) *
    ((DIVIDER_R4+DIVIDER_R14) / DIVIDER_R4);
}

float Adc::getCoreTemp(void) const
{
  return scaleTemp(samples[0]);
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

