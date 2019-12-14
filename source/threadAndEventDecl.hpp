#pragma once
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "showBlackboard.hpp"
#include "confFile.hpp"


extern  DifferentialPressure dp;
extern  Barometer baro;
extern  Adc adc;
extern  Imu imu;
extern	SdCard sdcard;
extern  ShowBlackboard showBB;
extern  ConfigurationFile confFile;
