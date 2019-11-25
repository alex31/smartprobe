#include <ch.h>
#include <hal.h>
#include "showBlackboard.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"	




bool ShowBlackboard::init()
{
  //  baro.blackBoard.registerEvt(&baroEvent, BARO_EVT);
  return true;
}


bool ShowBlackboard::loop()
{
  // registerEvt must be done in the thread that will wait on event,
  // so cannot be done in init method which is call by the parent thread
  baro.blackBoard.registerEvt(&baroEvent, BARO_EVT);
  dp.blackBoard.registerEvt(&diffPressEvent, PDIF_EVT);
  
  while (true) {
    chEvtWaitAny(PDIF_EVT);
    baro.blackBoard.read(baroData);
    dp.blackBoard.read(diffPressData);
    
    DebugTrace("BARO pressure [hPa]:%4.2f", baroData.pressure);
    DebugTrace("BARO temperature [degC]:%3.2f", baroData.temp);

    DebugTrace("ADC = %.2f ; core temp = %.1f",
	       adc.getPowerSupplyVoltage(),
	       adc.getCoreTemp());


    for (size_t i=0; i<3; i++) {
      DebugTrace("diffPress[%u] = press=%.3f temp=%.2f",
		 i,
		 diffPressData[i].pressure,
		 diffPressData[i].temp);
	      
    }

    
    chThdSleepSeconds(2);
  }
  return true;
}

  


