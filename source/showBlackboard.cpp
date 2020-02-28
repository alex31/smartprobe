#include <ch.h>
#include <hal.h>
#include "showBlackboard.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"

#define PERIOD(k) (CH_CFG_ST_FREQUENCY / CONF(k))


namespace {
  BarometerData    baroData{};
  DiffPressureData diffPressData{};
  ImuData  imuData{};
  Vec3f    attitude{},    attitudeSum{};
  AirSpeed relAirSpeed{}, relAirSpeedSum{};
  size_t   sumCount;
  event_listener_t diffPressEvent;
  sysinterval_t delay;
  systime_t now;
  systime_t then;
};

bool ShowBlackboard::initInThreadContext()
{
  dp.blackBoard.registerEvt(&diffPressEvent, PDIF_EVT);
  now = chVTGetSystemTimeX();
  then = chTimeAddX(now, delay);

  return true;
}

bool ShowBlackboard::init()
{
  delay = PERIOD("thread.frequency.stream_console");
  sumCount = 0;
  return true;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
bool ShowBlackboard::loop()
{
  const eventmask_t event = chEvtWaitOneTimeout(PDIF_EVT, TIME_MS2I(1000));

  if (not event)
    return true;
  
  baro.blackBoard.read(baroData);
  dp.blackBoard.read(diffPressData);
  imu.blackBoard.read(imuData);
  ahrs.blackBoard.read(attitude);
  relwind.blackBoard.read(relAirSpeed);

  if (chVTIsSystemTimeWithin(now, then)) {
    sumCount++;
    attitudeSum = vec3fAdd(&attitudeSum, &attitude);
    relAirSpeedSum += relAirSpeed;
  } else if (shouldSendSerialMessages()) {
    now = chVTGetSystemTimeX();
    then = chTimeAddX(now, delay);
    attitudeSum = vec3fDiv(&attitudeSum, sumCount);
    relAirSpeedSum /= sumCount;
    sumCount = 0;
    chprintf(chp,
		"%4.2f\t%3.2f\t%f\t"
		"%.4f\t%.4f\t%.4f\t"
		"%.2f\t%.2f\t%.2f\t"
		"%.2f\t%.2f\t"
		"%.2f\t%.2f\t%.2f\t"
		"%.2f\t%.1f\t\r\n",
		baroData.pressure,
		baroData.temp,
		baroData.rho,
		diffPressData[0].pressure,
		diffPressData[1].pressure,
		diffPressData[2].pressure,
		diffPressData[0].temp,
		diffPressData[1].temp,
		diffPressData[2].temp,
		rad2deg(attitudeSum.v[0]),
		rad2deg(attitudeSum.v[1]),
		relAirSpeedSum.velocity,
		relAirSpeedSum.alpha,
		relAirSpeedSum.beta,
		adc.getPowerSupplyVoltage(),
		adc.getCoreTemp()   );
  }

  
  return true;
}
 #pragma GCC diagnostic pop 


