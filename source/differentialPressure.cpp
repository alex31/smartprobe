#include "differentialPressure.hpp"
#include "stdutil.h"	
#include "hardwareConf.hpp"

namespace {

}


bool DifferentialPressure::init()
{
  i2cStart(&DiffPressI2CD, &i2ccfg_1000);
  if (sdp3xGeneralReset(&DiffPressI2CD) != MSG_OK) {
    DebugTrace("general reset FAILED");
    return false;
  }
  chThdSleepMilliseconds(SDP3X_WAIT_AFTER_RESET_MS);
  for (size_t i =0; i<sensorsd.size(); i++) {
    if (initSdp(sensorsd[i], SDP3X_ADDRESS1+i) != true) {
      DebugTrace("sensiron init @addr %u failed", SDP3X_ADDRESS1+i);
      return false;
    }
  }
  
  return true;
}


bool DifferentialPressure::loop()
{
  for (Sdp3xDriver &sdpd : sensorsd) {
    if ((sdp3xFetch(&sdpd, SDP3X_pressure_temp)) != MSG_OK)
      return false;
    DebugTrace("diffPress[%u] = press=%.3f temp=%.2f",
	       &sdpd-sensorsd.begin(),
	       sdp3xGetPressure(&sdpd),
	       sdp3xGetTemp(&sdpd));
  }
  chThdSleepSeconds(4);
  
  return true;
}

  

bool DifferentialPressure::initSdp(Sdp3xDriver &sdp, const uint8_t numSlave)
{
  Sdp3xIdent id;
  
  sdp3xStart(&sdp, &DiffPressI2CD, Sdp3xAddress(numSlave));
  if (sdp3xGetIdent(&sdp, &id) != MSG_OK) {
    DebugTrace("sdp3xGetIdent FAILED");
    return false;
  }

  if (sdp3xRequest(&sdp, SDP3X_pressure_temp) != MSG_OK) {
    DebugTrace("sdp3xRequest continuous FAILED");
    return false;
  }
  
  return true;
}

