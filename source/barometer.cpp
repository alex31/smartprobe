#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"	
#include "barometer.hpp"
#include "hardwareConf.hpp"
#include "sdcard.hpp"
#include "confFile.hpp"
#include "threadAndEventDecl.hpp"

bool Barometer::init()
{
  static const LPS33HWConfig lpsConfig = {
					  .i2cp = &BaroI2CD,
					  .slaveAdr = LPS33HW_I2C_SLAVE_SA0_LOW,
					  .odr = lps33hw_odr_t(CONF("sensor.barometer.odr")),
					  .lpf = lps33hw_lpfp_t(CONF("sensor.barometer.lpf")),
					  .blockDataUpdateEnable = true,
					  .dataReadyItrEnable = true };

  bool retVal = true;
  i2cStart(&BaroI2CD, &i2ccfg_400);
  palEnableLineEvent(LINE_BARO_DRDY, PAL_EVENT_MODE_FALLING_EDGE);
  
  const msg_t status = lps33Start(&lpsDriver, &lpsConfig);
  
  if (status != MSG_OK) {
    retVal = false;
    SdCard::logSyslog(Severity::Fatal, "lps33hw init FAIL");
  }

  airSensorDelta = CONF("sensor.barometer.temperatureBias");
  return retVal;
}



bool Barometer::loop()
{
  palWaitLineTimeout(LINE_BARO_DRDY, TIME_MS2I(200));
  const msg_t status = lps33Fetch(&lpsDriver,
				  static_cast<LPS33HWFetch>(LPS33HW_FETCH_PRESS |
							    LPS33HW_FETCH_TEMP));
  
  if (status != MSG_OK) {
    SdCard::logSyslog(Severity::Fatal, "lps33hw init FAIL");
    return false;
  }
  wdata.pressure = lps33GetPressure(&lpsDriver);
  wdata.temp = lps33GetTemp(&lpsDriver);
  pressAverage.push(pressToInt(wdata.pressure));
  tempAverage.push(tempToInt(wdata.temp));
  estimateRho();
  blackBoard.write(wdata);

  return true;
}

/*
    
   ρ = p / {Rspecific * T} 

where:

    ρ =  = air density (kg/m³)[note 1]
    p = = absolute pressure (Pa)[note 1]
    T = absolute temperature (K)[note 1]
    Rspecific =  287.058 J/(kg·K) 
 */

constexpr float Rspecific =  287.058f;
constexpr float kelvinDiff = 273.15f;
void Barometer::estimateRho(void)
{
  const float meanPress = pressToFloat(pressAverage.getMean());
  const float meanTemp = tempToFloat(tempAverage.getMean());
  const float kelvinCorrectedTemp = kelvinDiff + meanTemp + airSensorDelta;
  wdata.rho =  (meanPress * 100) / (Rspecific * kelvinCorrectedTemp);
}
