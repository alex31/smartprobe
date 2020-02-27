#include "differentialPressure.hpp"
#include "stdutil.h"	
#include "hardwareConf.hpp"
#include "sdcard.hpp"
#include "threadAndEventDecl.hpp"

namespace {
  DPressureFetchedParameter fetchedParam{};
  uint32_t temperatureFetchFrequency{};
  uint32_t tempFrequencyCount=0;
}

bool DifferentialPressure::init()
{
  fetchedParam = DPressureFetchedParameter(CONF("sensor.d_press.fetched"));
  temperatureFetchFrequency = CONF("sensor.d_press.fetchTempFrequency");

  i2cStart(&DiffPressI2CD, &i2ccfg_1000);
  if (sdp3xGeneralReset(&DiffPressI2CD) != MSG_OK) {
    SdCard::logSyslog(Severity::Fatal, "sensiron init general reset FAILED");
    return false;
  }
  chThdSleepMilliseconds(SDP3X_WAIT_AFTER_RESET_MS);
  for (size_t i =0; i<sensorsd.size(); i++) {
    if (initSdp(sensorsd[i], SDP3X_ADDRESS1+i) != true) {
      SdCard::logSyslog(Severity::Fatal, "sensiron init @addr %u failed", SDP3X_ADDRESS1+i);
      return false;
    }
  }
  
  return true;
}


bool DifferentialPressure::loop()
{
  const Sdp3xRequest request = (tempFrequencyCount++ % temperatureFetchFrequency) == 0 ?
    SDP3X_pressure_temp : SDP3X_pressure;
  
  for (Sdp3xDriver &sdpd : sensorsd) {
    if ((sdp3xFetch(&sdpd, request)) != MSG_OK) {
      SdCard::logSyslog(Severity::Warning, "sdp3xFetch failed");
    }
    const size_t index = &sdpd - sensorsd.begin();
    wdata[index].pressure = sdp3xGetPressure(&sdpd);
    if (request == SDP3X_pressure_temp) {
      wdata[index].temp = sdp3xGetTemp(&sdpd);
    }
  }
  
  blackBoard.write(wdata);
  return true;
}

  

bool DifferentialPressure::initSdp(Sdp3xDriver &sdp, const uint8_t numSlave)
{
  Sdp3xIdent id;
  const Sdp3xRequest request = (fetchedParam == PRESSURE_TEMPERATURE) ?
    SDP3X_pressure_temp : SDP3X_pressure;
  
  sdp3xStart(&sdp, &DiffPressI2CD, Sdp3xAddress(numSlave));
  if (sdp3xGetIdent(&sdp, &id) != MSG_OK) {
    SdCard::logSyslog(Severity::Fatal, "sdp3xGetIdent FAILED");
    return false;
  }

  if (sdp3xRequest(&sdp, request) != MSG_OK) {
    SdCard::logSyslog(Severity::Fatal, "sdp3xRequest continuous FAILED");
    return false;
  }
  
  SdCard::logSyslog(Severity::Info, "SDP3x id=0x%lx%lx scale=%f",
		    static_cast<uint32_t>(id.sn>>32),
		    static_cast<uint32_t>(id.sn & 0xffffffff),
		    static_cast<double>(sdp3xGetScale(&sdp)));
  return true;
}

