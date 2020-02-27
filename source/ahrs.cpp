#include <ch.h>
#include <hal.h>
#include "ahrs.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "sdcard.hpp"
#include "threadAndEventDecl.hpp"
#include "fusion6.h"
#include "math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
static systime_t 	chVTGetSystemTimeX (void)
 	Current system time. More...
 
static systime_t 	chVTGetSystemTime (void)
 	Current system time. More...
 
static sysinterval_t 	chVTTimeElapsedSinceX (systime_t start)
 	Returns the elapsed time since the specified start time. More...
 

 */

namespace {
  ImuData imuData{};
  //  systime_t dbgTimeStamp{};
}





bool Ahrs::init()
{
  Vec3f initalAcc =  imu.getInitialAcceleration();
  const float roll  = atan2f(-initalAcc.v[1], initalAcc.v[2]);
  const float pitch = atan2f(initalAcc.v[0], sqrt(initalAcc.v[1] * initalAcc.v[1] +
						  initalAcc.v[2] * initalAcc.v[2]));
  euler_rpy_t euler = {
		       .r = roll,
		       .p = pitch,
		       .y = 0.0f};
  SdCard::logSyslog(Severity::Info, "calibration loop, initial attitude "
		    ": pitch=%f roll=%f",
		    double(rad2deg(pitch)), double(rad2deg(roll)));

  sensfusion6Init(&euler);
  return true;
}

bool Ahrs::initInThreadContext()
{
  // registerEvt must be done in the thread that will wait on event,
  // so cannot be done in init method which is call by the parent thread
  imu.blackBoard.registerEvt(&imuEvent, IMU_EVT);

  return true;
}



bool Ahrs::loop()
{
  chEvtWaitOne(IMU_EVT);
  systime_t lastTimeStamp = timeStamp;
  timeStamp = chVTGetSystemTimeX();
  imu.blackBoard.read(imuData);
  sensfusion6UpdateQ(imuData.gyro.v[0], imuData.gyro.v[1], imuData.gyro.v[2],
		     imuData.acc.v[0], imuData.acc.v[1], imuData.acc.v[2],
		     TIME_I2US(timeStamp-lastTimeStamp) / 1e6);
  sensfusion6GetEulerRPY(&attitude.v[0], &attitude.v[1], &attitude.v[2]);
  blackBoard.write(attitude);

  // if (chVTTimeElapsedSinceX(dbgTimeStamp) > TIME_MS2I(1000)) {
  //   SdCard::logSyslog(Severity::Info, "dt = %.5f milliseconds",
  // 		      TIME_I2US(timeStamp-lastTimeStamp) / 1e3);
  //   dbgTimeStamp = chVTGetSystemTimeX();
  // }
  
  return true;
}
  


