#include <ch.h>
#include <hal.h>
#include "showBlackboard.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"	
#include "threadAndEventDecl.hpp"



bool ShowBlackboard::init()
{
  return true;
}

bool ShowBlackboard::initInThreadContext()
{
  // registerEvt must be done in the thread that will wait on event,
  // so cannot be done in init method which is call by the parent thread
  
  baro.blackBoard.registerEvt(&baroEvent, BARO_EVT);
  dp.blackBoard.registerEvt(&diffPressEvent, PDIF_EVT);
  imu.blackBoard.registerEvt(&imuEvent, IMU_EVT);

  return true;
}



bool ShowBlackboard::loop()
{
  chEvtWaitAll(IMU_EVT | BARO_EVT | PDIF_EVT);
  baro.blackBoard.read(baroData);
  dp.blackBoard.read(diffPressData);
  imu.blackBoard.read(imuData);
  
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
  
  DebugTrace("IMU temp= %.2f\r\n"
	     "IMU gyro=[x=%.2f, y=%.2f, z=%.2f]\r\n"
	     "IMU acc= [x=%.2f, y=%.2f, z=%.2f]",
	     imuData.temp,
	     imuData.gyro.v[0],  imuData.gyro.v[1], imuData.gyro.v[2],
	     imuData.acc.v[0],  imuData.acc.v[1], imuData.acc.v[2]);

  return true;
}

  


