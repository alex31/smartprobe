#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"	
#include "barometer.hpp"
#include "hardwareConf.hpp"

namespace {
  constexpr LPS33HWConfig lpsConfig = {
				       .i2cp = &BaroI2CD,
				       .slaveAdr = LPS33HW_I2C_SLAVE_SA0_LOW,
				       .odr = LPS33HW_ODR_75_Hz,
				       .lpf = LPS33HW_LPF_ODR_DIV_2,
				       .blockDataUpdateEnable = true,
				       .dataReadyItrEnable = true };
  
  THD_WORKING_AREA(waBaroIt, 1024);
  volatile  uint32_t baroCount=0U;
  volatile  uint32_t baroFreq=0U;

  void baroIt(void *arg) 
  {
    LPS33HWDriver * lpsDriver = static_cast<LPS33HWDriver *>(arg);
    
    chRegSetThreadName("barometer: fetch on drdy");
    palEnableLineEvent(LINE_BARO_DRDY, PAL_EVENT_MODE_FALLING_EDGE);
    
    while (true) {
      palWaitLineTimeout(LINE_BARO_DRDY, TIME_MS2I(200));
      baroCount++;
      lps33Fetch(lpsDriver, static_cast<LPS33HWFetch>(LPS33HW_FETCH_PRESS |
						      LPS33HW_FETCH_TEMP));
    }
  }
  
}


bool Barometer::init()
{
  bool retVal = true;

  i2cStart(&BaroI2CD, &i2ccfg_1000);
  msg_t status = lps33Start(&lpsDriver, &lpsConfig);
 
  if (status == MSG_OK) {
    DebugTrace("lps33hw init OK");
    chThdCreateStatic(waBaroIt, sizeof(waBaroIt),
		      NORMALPRIO, baroIt, &lpsDriver);
  } else {
    retVal = false;
    DebugTrace("lps33hw init FAIL");
  }
  
  return retVal;
}


bool Barometer::loop()
{
  while (true) {				
    DebugTrace("BARO pressure [hPa]:%4.2f", lps33GetPressure(&lpsDriver));
    DebugTrace("BARO temperature [degC]:%3.2f", lps33GetTemp(&lpsDriver));
    DebugTrace("BARO freq = %ld", baroFreq);
    chThdSleepMilliseconds(1000);
    baroFreq=baroCount;
    baroCount=0;
  }
  return true;
}

  


