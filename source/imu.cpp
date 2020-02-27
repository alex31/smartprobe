#include <ch.h>
#include <hal.h>
#include "imu.hpp"
#include "hardwareConf.hpp" 
#include "stdutil.h"	
#include "sdcard.hpp"
#include "confFile.hpp"
#include "threadAndEventDecl.hpp"
#include <math.h>



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
  spiStart(&ImuSPID, &spiCfg);
  bool success = init20600(ImuKindOfInit::BiasEstimation);
  if (success) 
    success = estimateBiasAndPosition();
  if (success) 
    success = init20600(ImuKindOfInit::Measure);
    
  return success;
}

bool Imu::init20600(const ImuKindOfInit kind)
{
  static Icm20600Config icmCfg =
    (kind == ImuKindOfInit::Measure) ?
    Icm20600Config {
     .spid = &ImuSPID,
     .sampleRate = uint32_t(CONF("thread.frequency.imu")) * 2,
     .config = Icm20600_config(CONF("sensor.imu.gyrorate")),
     .gyroConfig = Icm20600_gyroConf(CONF("sensor.imu.fchoicerate")) |
     Icm20600_gyroConf(CONF("sensor.imu.gyrorange")),
     .accelConf = Icm20600_accelConf(CONF("sensor.imu.accrange")),
     .accelConf2 = Icm20600_accelConf2(CONF("sensor.imu.accrate"))
  } : // (kind == ImuKindOfInit::BiasEstimation)
  Icm20600Config {
   .spid = &ImuSPID,
   .sampleRate = 8000,
   .config = ICM20600_GYRO_RATE_8K_BW_3281,
   .gyroConfig = ICM20600_RANGE_250_DPS,
   .accelConf = ICM20600_RANGE_2G,
   .accelConf2 = ICM20600_ACC_RATE_4K_BW_1046
  };

  const size_t kind_sc = static_cast<size_t>(kind);
  
  
  if (icm20600_init(&icmd, &icmCfg) == MSG_OK) {
    SdCard::logSyslog(Severity::Info, "icm20600 init for %s OK",
		      imuKindOfInitStr[kind_sc]);
  } else {
    SdCard::logSyslog(Severity::Fatal, "icm20600 init for %s FAIL",
		      imuKindOfInitStr[kind_sc]);
    return false;
  }

  const Icm20600TestResult res = icm20600_runSelfTests(&icmd);
  SdCard::logSyslog(Severity::Info, "icm20600 overall factory note for %s = "
		    "%u bias=%d passed=%d",
		    imuKindOfInitStr[kind_sc],
		    res.factory, res.bias, res.passed);
  
  if (res.bias && res.passed) {
    SdCard::logSyslog(Severity::Info, "icm20600 factory test for %s OK",
		      imuKindOfInitStr[kind_sc]);
  } else {
    SdCard::logSyslog(Severity::Fatal, "icm20600 factory test for %s FAIL",
		      imuKindOfInitStr[kind_sc]);
    return false;
  }
  
  return true;
}

bool Imu::estimateBiasAndPosition(void)
{
  const int loopDurationMs = CONF("sensor.imu.estimationLoopDuration_ms");
  const systime_t now = chVTGetSystemTimeX();
  const systime_t then = chTimeAddX(now, TIME_MS2I(loopDurationMs));
  size_t count = 0;
  while (chTimeIsInRangeX(chVTGetSystemTimeX(), now, then)) {
    count ++;
    icm20600_fetch(&icmd);
    icm20600_getVal(&icmd, &wdata.temp, &wdata.gyro, &wdata.acc);
    bias = bias + wdata;
  }
  bias.gyro = bias.gyro / count;
  bias.acc = bias.acc / count;

  
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"

  SdCard::logSyslog(Severity::Info, "calibration loop : %d iteration in %d milliseconds",
		    count, loopDurationMs);
  SdCard::logSyslog(Severity::Info, "calibration loop : gyro bias = r:%f, p:%f, y:%f",
		    bias.gyro.v[0],  bias.gyro.v[1],  bias.gyro.v[2]);

#pragma GCC diagnostic pop
  return true;
}


bool Imu::loop()
{
  icm20600_fetch(&icmd);
  icm20600_getVal(&icmd, &wdata.temp, &wdata.gyro, &wdata.acc);
  wdata.gyro = wdata.gyro - bias.gyro;
  blackBoard.write(wdata);
  
  return true;
}

  


