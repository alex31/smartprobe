#pragma once
#include "i2cMaster.h"
#include "spiPeriphICM20600.h"

using namespace std::literals;

enum AhrsType {RAW_IMU=0, HEADLESS_AHRS, COMPLETE_AHRS};
enum DPressureFetchedParameter {PRESSURE_ONLY=0, PRESSURE_TEMPERATURE};
enum SerialMode {SHELL, PPRZ_IN_OUT, NMEA_IN, UBLOX_IN};
constexpr double ESTIMATE_ROW = 0.0;

#ifdef OLD_EXAMPLE
 #define PARAMETERS_MAP                                         \
   {"sensor.pressure.frequency", {1, NAMESET({1, "frequency_one"}) }}, \
   {"sensor.other", {2, NAMESET({1, "other_one"}, {2, "other_two"}) }},  \
   {"sensor.o2", {3, NAMESET({1, "o2_low"}, {3, "o2_high"}) }},       \
   {"thread.waitmillisecond", {20.5f, RANGEINT(9, 1000) }},     \
   {"sensor.icm20600.init.cr1", {0, RANGEINT(0, 16) }}, \
   {"thread.enable.imu", {true, NONAMESET }},                   \
   {"thread.frequency", {100, RANGEINT(10, 1000) }}
#endif

#define PARAMETERS_MAP                                          \
  {"thread.frequency.d_press", {100, RANGEINT(10, 2000) }},	\
  {"thread.frequency.imu", {100, RANGEINT(10, 4000) }},		\
  {"thread.frequency.stream_console", {10, RANGEINT(1, 100) }},		\
  {"thread.frequency.transmit_uart", {10, RANGEINT(1, 100) }},		\
  {"sensor.barometer.lpf", {0, NAMESET({ LPS33HW_LPF_ODR_DIV_2, "div2"}, {LPS33HW_LPF_ODR_DIV_9, "div9"}, \
				       {LPS33HW_LPF_ODR_DIV_20, "div20"}) }}, \
  {"ahrs.type", {1, NAMESET({RAW_IMU, "raw_imu"}, \
			    {HEADLESS_AHRS, "headless_ahrs"}, \
			    {COMPLETE_AHRS, "complete_ahrs"}) }}, \
  {"uart.mode", {1, NAMESET({SHELL, "shell"},			  \
			    {PPRZ_IN_OUT, "pprz_in_out"}, \
			    {NMEA_IN, "nmea_in"}, \
			    {UBLOX_IN, "ublox_in"}) }}, \
  {"uart.baud", {115200, RANGEINT(9600, 460800) }},	\
  {"sensor.barometer.temperatureBias", {-5.0, RANGEDOUBLE(-15.0, 0.0) }}, \
  {"sensor.barometer.odr", {4, NAMESET({LPS33HW_POWER_DOWN, "powerdown"}, {LPS33HW_ODR_1_Hz, "1hz"}, \
				       {LPS33HW_ODR_10_Hz, "10hz"}, {LPS33HW_ODR_25_Hz, "25hz"}, \
				       {LPS33HW_ODR_50_Hz, "50hz"}, {LPS33HW_ODR_75_Hz, "75hz"}) }},	\
  {"sensor.d_press.fetched", {1, NAMESET({PRESSURE_ONLY , "P"}, \
					 {PRESSURE_TEMPERATURE, "P+T"}) }} ,	\
  {"sensor.d_press.fetchTempFrequency", {1, RANGEINT(1, 1000) }},		\
  {"sensor.imu.gyrorate", {1, NAMESET({ICM20600_GYRO_RATE_8K_BW_250, "8khz_bw250"},	\
				      {ICM20600_GYRO_RATE_1K_BW_176, "1khz_bw176"},	\
				      {ICM20600_GYRO_RATE_1K_BW_92 , "1khz_bw92"},		\
                                      {ICM20600_GYRO_RATE_1K_BW_41 , "1khz_bw41"},		\
				      {ICM20600_GYRO_RATE_1K_BW_20 , "1khz_bw20"},		\
				      {ICM20600_GYRO_RATE_1K_BW_10 , "1khz_bw10"},		\
				      {ICM20600_GYRO_RATE_1K_BW_5  , "1khz_bw5"},		\
				      {ICM20600_GYRO_RATE_8K_BW_3281, "8khz_bw3281"}) }},	\
  {"sensor.imu.accrate", {1, NAMESET({ICM20600_ACC_RATE_1K_BW_218, "1khz_bw218"},		\
				     {ICM20600_ACC_RATE_1K_BW_99,  "1khz_bw99"},		\
				     {ICM20600_ACC_RATE_1K_BW_44,  "1khz_bw44"},		\
                                     {ICM20600_ACC_RATE_1K_BW_21,  "1khz_bw21"},		\
				     {ICM20600_ACC_RATE_1K_BW_10,  "1khz_bw10"},		\
				     {ICM20600_ACC_RATE_1K_BW_5 ,  "1khz_bw5"},			\
				     {ICM20600_ACC_RATE_1K_BW_420, "1khz_bw420"},		\
				     {ICM20600_ACC_RATE_4K_BW_1046, "4khz_bw1046"}) }},		\
  {"sensor.imu.fchoicerate", {1, NAMESET({0, "nofchoice"},		\
					 {ICM20600_FCHOICE_RATE_32K_BW_8173, "32khz_bw8173"},   \
					 {ICM20600_FCHOICE_RATE_32K_BW_3281, "32Khz_bw3281"}) }}, \
  {"sensor.imu.gyrorange", {0, NAMESET({ICM20600_RANGE_250_DPS, "250_dps"}, \
				       {ICM20600_RANGE_500_DPS, "500_dps"}, \
				       {ICM20600_RANGE_1K_DPS, "1000_dps"},		\
				       {ICM20600_RANGE_2K_DPS, "2000_dps"}) }},		\
  {"sensor.imu.accrange", {0, NAMESET({ICM20600_RANGE_2G, "2g"}, \
				      {ICM20600_RANGE_4G, "4g"}, \
				      {ICM20600_RANGE_8G, "8g"}, \
				      {ICM20600_RANGE_16G, "16g"}) }}, \
  {"sensor.imu.estimationLoopDuration_ms", {500, RANGEINT(10, 4000) }}, \
  {"airspeed.rho", {ESTIMATE_ROW, RANGEDOUBLE(0.0, 2.0)}},			\
  {"airspeed.calibration.m11", {1.15, NONAMESET}}, \
  {"airspeed.calibration.m12", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m13", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m14", {6.0, NONAMESET}}, \
  {"airspeed.calibration.m15", {6.0, NONAMESET}}, \
  {"airspeed.calibration.m21", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m22", {15.0, NONAMESET}}, \
  {"airspeed.calibration.m23", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m24", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m25", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m31", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m32", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m33", {11.0, NONAMESET}}, \
  {"airspeed.calibration.m34", {0.0, NONAMESET}}, \
  {"airspeed.calibration.m35", {0.0, NONAMESET}}, \
  {"airspeed.calibration.bias.velocity", {0.0, NONAMESET}}, \
  {"airspeed.calibration.bias.alpha", {0.0, NONAMESET}}, \
  {"airspeed.calibration.bias.beta", {0.0, NONAMESET}}
