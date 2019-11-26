#include <ch.h>
#include <hal.h>
#include "imu.hpp"
#include "hardwareConf.hpp" 
#include "stdutil.h"	


namespace {
  const SPIConfig spiCfg = {
			    .circular = false,
			    .end_cb = NULL,
			    /* HW dependent part.*/
			    .ssline = LINE_SPI1_NSS,
			    /* 6.25 Mhz, 8 bits word, CPHA= second (rising) edge, 
			       CPOL= high level idle state */
			    .cr1 = SPI_CR1_CPHA | SPI_CR1_CPOL |
			           SPI_CR1_BR_1 | SPI_CR1_BR_0,
			    .cr2 = 0
  };

  Icm20600Config icmCfg = {
			   .spid = &ImuSPID,
			   .sampleRate = 100,
			   .config = ICM20600_GYRO_RATE_8K_BW_3281,
			   .gyroConfig = ICM20600_FCHOICE_RATE_32K_BW_8173 |
			                 ICM20600_RANGE_250_DPS,
			   .accelConf = ICM20600_RANGE_2G,
			   .accelConf2 = ICM20600_ACC_RATE_4K_BW_1046
  };

  
}




bool Imu::init()
{
  spiStart(&ImuSPID, &spiCfg);
  if (icm20600_init(&icmd, &icmCfg) == MSG_OK) {
    DebugTrace ("IMU init OK");
  } else {
    DebugTrace ("IMU init FAIL");
    return false;
  }

  const Icm20600TestResult res = icm20600_runSelfTests(&icmd);
  DebugTrace("overall factory note = %u bias=%d passed=%d",
	     res.factory, res.bias, res.passed);
  
  if (res.bias && res.passed) {
    DebugTrace ("IMU factory test OK");
  } else {
    DebugTrace ("IMU factory FAIL");
    return false;
  }
  
  return true;
}


bool Imu::loop()
{
  icm20600_fetch(&icmd);
  icm20600_getVal(&icmd, &wdata.temp, &wdata.gyro, &wdata.acc);
  blackBoard.write(wdata);
  
  return true;
}

  


