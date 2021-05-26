#include <ch.h>
#include <hal.h>
#include "relativeWind.hpp"
#include "hardwareConf.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "sdcard.hpp"
#include "threadAndEventDecl.hpp"


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
  //convention for matrix size is Line x Col
  using Matrix53f = Eigen::Matrix<float, 3, 5>;
  using Vector5f = Eigen::Matrix<float, 5, 1>;
  using Vector3f = Eigen::Matrix<float, 3, 1>;

  Matrix53f cal;
  Vector5f vec;
  Vector3f bias;
  DiffPressureData dpData{};
  BarometerData baroData{};
  float stdRho{};
}


bool Relwind::init()
{
  cal <<
    CONF("airspeed.calibration.m11"),
    CONF("airspeed.calibration.m12"),
    CONF("airspeed.calibration.m13"),
    CONF("airspeed.calibration.m14"),
    CONF("airspeed.calibration.m15"),
    CONF("airspeed.calibration.m21"),
    CONF("airspeed.calibration.m22"),
    CONF("airspeed.calibration.m23"),
    CONF("airspeed.calibration.m24"),
    CONF("airspeed.calibration.m25"),
    CONF("airspeed.calibration.m31"),
    CONF("airspeed.calibration.m32"),
    CONF("airspeed.calibration.m33"),
    CONF("airspeed.calibration.m34"),
    CONF("airspeed.calibration.m35");
  
  bias <<
    CONF("airspeed.calibration.bias.velocity"),
    CONF("airspeed.calibration.bias.alpha"),
    CONF("airspeed.calibration.bias.beta");
  
  stdRho = CONF("airspeed.rho");
  
  return true;
}

bool Relwind::initInThreadContext()
{
  // registerEvt must be done in the thread that will wait on event,
  // so cannot be done in init method which is call by the parent thread
  dp.blackBoard.registerEvt(&pDiffEvent, PDIF_EVT);

  return true;
}



bool Relwind::loop()
{
  constexpr float isaRho =1.225f; // (density at sea level in ISA condition)
  float rho;
  chEvtWaitOne(PDIF_EVT);
  dp.blackBoard.read(dpData);
  
  if (stdRho == 0.0f) {
    baro.blackBoard.read(baroData);
    rho = baroData.rho;
  } else {
    rho = stdRho;
  }

  const float& p0 = dpData[0].pressure;
  const float& p1 = dpData[1].pressure;
  const float& p2 = dpData[2].pressure;
  const float p10 = p1 / p0;
  const float p11=  p10 * p10;
  const float p20=  p2 / p0;
  const float p22=  p20 * p20;
  
  vec << p0, p10, p20, p11, p22;
  const Vector3f X = cal * vec;
  const float q = X[0] + bias[0];
  
  if ((q < 0.0f) or isnan(X[0]) or isnan(X[1]) or isnan(X[2])) {
    airSpeed.tas =  airSpeed.beta = airSpeed.alpha = 0.0f;
  } else {
    airSpeed.tas = sqrtf(q / (0.5f * rho));
    airSpeed.eas = sqrtf(q / (0.5f * isaRho));
    airSpeed.beta = X[1] + bias[1];
    airSpeed.alpha = X[2] + bias[2];
  }
  blackBoard.write(airSpeed);
  
  return true;
}
  


