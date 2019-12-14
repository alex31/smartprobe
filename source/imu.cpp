#include <ch.h>
#include <hal.h>
#include "imu.hpp"
#include "hardwareConf.hpp" 
#include "stdutil.h"	
#include "sdcard.hpp"
#include "confFile.hpp"
#include "threadAndEventDecl.hpp"


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


  
}




bool Imu::init()
{
  static Icm20600Config icmCfg = {
	.spid = &ImuSPID,
	.sampleRate = uint32_t(CONF("thread.frequency.imu")),
	.config = Icm20600_config(CONF("sensor.imu.gyrorate")),
	.gyroConfig = Icm20600_gyroConf(CONF("sensor.imu.fchoicerate")) |
		      Icm20600_gyroConf(CONF("sensor.imu.gyrorange")),
	.accelConf = Icm20600_accelConf(CONF("sensor.imu.accrange")),
	.accelConf2 = Icm20600_accelConf2(CONF("sensor.imu.accrate"))
  };


  spiStart(&ImuSPID, &spiCfg);
  if (icm20600_init(&icmd, &icmCfg) == MSG_OK) {
    SdCard::logSyslog(Severity::Info, "icm20600 init OK");
  } else {
    SdCard::logSyslog(Severity::Fatal, "icm20600 init FAIL");
    return false;
  }

  const Icm20600TestResult res = icm20600_runSelfTests(&icmd);
  SdCard::logSyslog(Severity::Info, "icm20600 overall factory note = "
		    "%u bias=%d passed=%d",
		    res.factory, res.bias, res.passed);
  
  if (res.bias && res.passed) {
    SdCard::logSyslog(Severity::Info, "icm20600 factory test OK");
  } else {
    SdCard::logSyslog(Severity::Fatal, "icm20600 factory test FAIL");
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

  


