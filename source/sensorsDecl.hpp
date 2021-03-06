#pragma once
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdCard.hpp"

extern  DifferentialPressure dp;
extern  Barometer baro;
extern  Adc adc;
extern  Imu imu;
extern	SdCard sdcard
