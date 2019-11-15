#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"	
#include "i2cMaster.h"
#include "hardwareConf.hpp"
#include "barometer.hpp"


namespace {
  constexpr LPS33HWConfig lpsConfig = {
				       .i2cp = &BaroI2CD,
				       .slaveAdr = LPS33HW_I2C_SLAVE_SA0_LOW,
				       .odr = LPS33HW_ODR_75_Hz,
				       .lpf = LPS33HW_LPF_ODR_DIV_2,
				       .blockDataUpdateEnable = true,
				       .dataReadyItrEnable = true };

  LPS33HWDriver lpsDriver;
  volatile  uint32_t baroCount=0U;
  volatile  uint32_t baroFreq=0U;


  THD_WORKING_AREA(waBaroIt, 1024);
  void baroIt(void *arg) 
  {
    (void) arg;
    
    chRegSetThreadName("baro acquire");
    palEnableLineEvent(LINE_BARO_DRDY, PAL_EVENT_MODE_FALLING_EDGE);
    
    while (true) {
      palWaitLineTimeout(LINE_BARO_DRDY, TIME_MS2I(200));
      baroCount++;
      lps33Fetch(&lpsDriver, LPS33HWFetch(LPS33HW_FETCH_PRESS | LPS33HW_FETCH_TEMP));
    }
  }

  
  bool baroInit(void)
  {
    bool retVal = true;
    msg_t status = lps33Start(&lpsDriver, &lpsConfig);

  
    if (status == MSG_OK) {
      DebugTrace("lps33hw init OK");
    } else {
      retVal = false;
      DebugTrace("lps33hw init FAIL");
    }

    return retVal;
  }


  THD_WORKING_AREA(waBaroAcquisition, 512);	
  void baroAcquisition (void *arg)	        
  {
    (void)arg;					
    chRegSetThreadName("baroAcquisition");	
    
    while (true) {				
      DebugTrace("BARO pressure [hPa]:%4.2f", lps33GetPressure(&lpsDriver));
      DebugTrace("BARO temperature [degC]:%3.2f", lps33GetTemp(&lpsDriver));
      DebugTrace("BARO freq = %ld", baroFreq);
      chThdSleepMilliseconds(1000);
      baroFreq=baroCount;
      baroCount=0;
    }
  }

} // namespace private


namespace barometer {



  bool    launchThd(void)
  {
    bool retVal = true;
    
    if ((retVal = baroInit()) == true) {
      chThdCreateStatic(waBaroIt, sizeof(waBaroIt),
			NORMALPRIO, baroIt, nullptr);
      chThdCreateStatic(waBaroAcquisition, sizeof(waBaroAcquisition),
			NORMALPRIO, &baroAcquisition, nullptr);
    }
    
    return retVal;
  }

} // namespace barometer


