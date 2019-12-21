#include <ch.h>
#include <hal.h>
#include "showBlackboard.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"



namespace {
  BarometerData baroData{};
  DiffPressureData diffPressData{};
  ImuData imuData{};
  Vec3f   attitude{};
  AirSpeed relAirSpeed{};
};

bool ShowBlackboard::init()
{
  return true;
}




bool ShowBlackboard::loop()
{
  baro.blackBoard.read(baroData);
  dp.blackBoard.read(diffPressData);
  imu.blackBoard.read(imuData);
  ahrs.blackBoard.read(attitude);
  relwind.blackBoard.read(relAirSpeed);

  if (shouldSendSerialMessages()) {
    chprintf(chp, "%4.2f\t%3.2f\t%f\t"
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
	     attitude.v[0],
	     attitude.v[1],
	     relAirSpeed.velocity,
	     relAirSpeed.alpha,
	     relAirSpeed.beta,
	     adc.getPowerSupplyVoltage(),
	     adc.getCoreTemp()   );
  }

  return true;
}
  


