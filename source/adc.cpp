#include <ch.h>
#include <hal.h>
#include "hardwareConf.hpp"
#include "adc.hpp"
#include "stdutil.h"	

namespace {

  constexpr ADCConversionGroup adcgrpcfg = {
  .circular     = true,   // continuous conversion
  .num_channels = Adc::ADC_GRP1_NUM_CHANNELS,
  .end_cb       = NULL, // adc completed callback,
  .error_cb     = NULL, // adc error callback,
  .cr1          = 0,   
  .cr2          = ADC_CR2_SWSTART,          
  .smpr1        = ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_480),
  .smpr2        = ADC_SMPR2_SMP_AN9(ADC_SAMPLE_480),
  .htr          = 0,
  .ltr          = 0,
  .sqr1         = 0,
  .sqr2         = 0,                                       
  .sqr3         = ADC_SQR3_SQ1_N(ADC_CHANNEL_SENSOR) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN9)
};
  
}

bool Adc::init()
{
  adcStart(&ADCD1, NULL);
  adcSTM32EnableTSVREFE();
  adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);

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
  constexpr float R4 = 1.5_kohm;
  constexpr float R14 = 2.2_kohm;

  return (3.3f * samples[1] / 4095.0f) * ((R4+R14) / R4);
}

float Adc::getCoreTemp(void) const
{
  return scaleTemp(samples[0]);
}



float Adc::scaleTemp (int fromTmp) 
{
  const float sampleNorm = (float) fromTmp / 4095.0f;
  const float sampleVolt = sampleNorm * 3.3f;
  const float deltaVolt = sampleVolt - 0.76f;
  const float deltaCentigrade = deltaVolt / 2.5e-3f;
  const float temp = 25.0f + deltaCentigrade;

  return temp;
}
