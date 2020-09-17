#include <ch.h>
#include <hal.h>
#include "stdutil.h"	
#include "hardwareConf.hpp"
#include "adc.hpp"
#include "sdLiteLog.hpp"	
#include "sdcard.hpp"	
#include "threadAndEventDecl.hpp"

namespace {
  void adcErrorCb(ADCDriver *adcp, adcerror_t err);
  
  constexpr adcsample_t psVoltToSample(const float voltage)  {
    return (voltage * SAMPLE_MAX * DIVIDER_R18) /
      ((VCC_33 * (DIVIDER_R17 + DIVIDER_R18)));
  }

  constexpr float sampleToPsVoltage(const adcsample_t sample ) {
    return (VCC_33 * sample / SAMPLE_MAX) *
      ((DIVIDER_R17+DIVIDER_R18) / DIVIDER_R18);
  }
  
  constexpr float sampleToSensorsVoltage(const adcsample_t sample ) {
    return (VCC_33 * sample / SAMPLE_MAX) *
      ((DIVIDER_R16+DIVIDER_R19) / DIVIDER_R19);
  }

  float sampleToCoreVoltage(const adcsample_t sample ) {
    return (SAMPLE_MAX * Adc::vrefCalib()) / sample;
  }
  
  constexpr ADCConversionGroup adcgrpcfgThreshold = {
	.circular     = false,
	.num_channels = Adc::ADC_GRP1_NUM_CHANNELS,
	.end_cb       = NULL, // adc completed callback,
	.error_cb     = nullptr,
	.cr1		= 0,
	.cr2          = ADC_CR2_SWSTART,          
	.smpr1        = ADC_SMPR1_SMP_AN12(ADC_SAMPLE_480) |
			ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_480) |
			ADC_SMPR1_SMP_VREF(ADC_SAMPLE_480),
	.smpr2        = ADC_SMPR2_SMP_AN9(ADC_SAMPLE_480),
	.htr          = 0,
	.ltr          = 0,
	.sqr1         = 0,
	.sqr2         = 0,                                       
	.sqr3         = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN9) |
	                ADC_SQR3_SQ2_N(ADC_CHANNEL_IN12) |
	                ADC_SQR3_SQ3_N(ADC_CHANNEL_SENSOR) |
	                ADC_SQR3_SQ4_N(ADC_CHANNEL_VREFINT)
  };

  ADCConversionGroup adcgrpcfg = {
	.circular     = true,   // continuous conversion
	.num_channels = Adc::ADC_GRP1_NUM_CHANNELS,
	.end_cb       = NULL, // adc completed callback,
	.error_cb     = &adcErrorCb, // adc error callback,
	.cr1		= ADC_CR1_AWDSGL | ADC_CR1_AWDEN | ADC_CR1_AWDIE |
	(9 << ADC_CR1_AWDCH_Pos), 
	.cr2          = ADC_CR2_SWSTART,          
	.smpr1        = adcgrpcfgThreshold.smpr1,
	.smpr2        = adcgrpcfgThreshold.smpr2,
	.htr          = SAMPLE_MAX,
	.ltr          = 0,
	.sqr1         = 0,
	.sqr2         = 0,                                       
	.sqr3         = adcgrpcfgThreshold.sqr3
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

  if (calculateThreshold() == false)
    return false;
  
  adcgrpcfg.ltr = psThreshold;
  adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);

  return true;
}

bool Adc::calculateThreshold()
{
  constexpr size_t loop = 20'000U;

  chThdSleepMilliseconds(100); // wait for VSS to stabilize
  uint32_t accum[ADC_GRP1_NUM_CHANNELS] = {0U};
  for (size_t i=0; i<loop; i++) {
    adcConvert(&ADCD1, &adcgrpcfgThreshold, samples, ADC_GRP1_BUF_DEPTH);
    for (size_t j=0; j<Adc::ADC_GRP1_NUM_CHANNELS; j++) {
      accum[j] += samples[j];
    }
  }
  const float averageVoltage = sampleToPsVoltage(accum[0] / loop);
  const float averageSensorsVdd = sampleToSensorsVoltage(accum[1] / loop);
  const float averageCoreVdd = sampleToCoreVoltage(accum[3] / loop);
  
  // try to guess number of elements
  const uint32_t numberOfElem = averageVoltage / NOMINAL_VOLTAGE_BY_ELEMENT;

  // 2.9v by element is the absolute minium before bettery destruction
  // 7V is the minimal voltage needed by the 5V regulator
  const float minVoltageThreshold = std::max(VOLTAGE_ABSOLUTE_MINIMUM,
			 numberOfElem * MINIMUM_VOLTAGE_BY_ELEMENT);
  
  psThreshold = psVoltToSample(minVoltageThreshold);

  SdCard::logSyslog(Severity::Info, "Power supply Voltage = %.2f, "
		    "shutdown threshold voltage  = %.2f, " 
		    "ps ltr  = %u", 
		    double(averageVoltage), double(minVoltageThreshold),
		    psThreshold);

  SdCard::logSyslog(Severity::Info, "Internal Voltage : Sensors = %.2f, Core = %.2f",
		    double(averageSensorsVdd), double(averageCoreVdd));

  
  if (averageVoltage < VOLTAGE_ABSOLUTE_MINIMUM) {
    SdCard::logSyslog(Severity::Fatal, "Power supply Voltage = %.2f is "
	      "less than absolute minimum %.2f", 
	      double(averageVoltage), double(VOLTAGE_ABSOLUTE_MINIMUM));
    return false;
  }

  if (averageSensorsVdd != std::clamp(averageSensorsVdd,
				      VOLTAGE_3_3_MINIMUM,  VOLTAGE_3_3_MAXIMUM)) {
    SdCard::logSyslog(Severity::Fatal, "Sensors internal Voltage = %.2f is "
	      "not in range [%.2f .. %.2f]", 
		      double(averageSensorsVdd), double(VOLTAGE_3_3_MINIMUM),
		      double(VOLTAGE_3_3_MAXIMUM));
    return false;
  }

  if (averageCoreVdd != std::clamp(averageCoreVdd,
				      VOLTAGE_3_3_MINIMUM,  VOLTAGE_3_3_MAXIMUM)) {
    SdCard::logSyslog(Severity::Fatal, "Core internal Voltage = %.2f is "
	      "not in range [%.2f .. %.2f]", 
		      double(averageCoreVdd), double(VOLTAGE_3_3_MINIMUM),
		      double(VOLTAGE_3_3_MAXIMUM));
    return false;
  }
  
  chThdSleepMilliseconds(200);
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
  fl.powerOff(); // power off front rgb led
  stopAllPeripherals();
  SdLiteLogBase::terminate(TerminateBehavior::DONT_WAIT);
  f_mount(NULL, "", 0);
  chThdSleepMilliseconds(PowerLossAwakeTimeBeforeDeepSleep);
  systemDeepSleep();
  while (true);
}

  
float Adc::getPowerSupplyVoltage(void) const
{
  return sampleToPsVoltage(samples[0]);
}

float Adc::getSensorsVoltage(void) const
{
  return sampleToSensorsVoltage(samples[1]);
}

float Adc::getCoreTemp(void) const
{
  return scaleTemp(samples[2]);
}

float Adc::getCoreVdd(void) const
{
  return sampleToCoreVoltage(samples[3]);
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

