#pragma once
#include "barometer.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "differentialPressure.hpp"
#include "sdcard.hpp"
#include "showBlackboard.hpp"
#include "confFile.hpp"
#include "ahrs.hpp"
#include "relativeWind.hpp"
#include "receivePprzlink.hpp"
#include "receiveNmealink.hpp"
#include "receiveUbxlink.hpp"

#define CONF(k)  (ConfigurationFile_AT(confFile, k))

extern  DifferentialPressure	dp;
extern  Barometer 		baro;
extern  Adc 			adc;
extern  Imu 			imu;
extern	SdCard 			sdcard;
extern  ShowBlackboard 		showBB;
extern  ConfigurationFile 	confFile;
extern  Ahrs			ahrs;
extern  Relwind			relwind;
extern  ReceivePprzlink		receivePPL;
extern  ReceiveNmealink		receiveNMEA;
extern  ReceiveUbxlink		receiveUBX;
