#pragma once

using namespace std::literals;


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
  {"filename.sensors", {"sensors"sv, NONAMESET }},              \
  {"thread.frequency.d_press", {100, RANGEINT(10, 1000) }},	\
  {"thread.frequency.imu", {100, RANGEINT(10, 1000) }},		\
  {"thread.frequency.stream", {20, RANGEINT(1, 100) }},		\
  {"sensor.barometer.lpf", {0, NAMESET({0, "div2"}, {2, "div9"}, \
				       {3, "div20"}) }}, \
  {"sensor.barometer.odr", {4, NAMESET({0, "powerdown"}, {1, "1hz"}, {2, "10hz"}, \
				       {3, "25hz"}, {4, "50hz"}, {5, "75hz"}) }}, \
  {"sensor.imu.gyrorate", {1, NAMESET({0, "8khz_bw250"}, {1, "1khz_bw176"}, {2, "1khz_bw92"}, \
                                      {3, "1khz_bw41"}, {4, "1khz_bw20"}, {5, "1khz_bw10"}, \
				      {6, "1khz_bw5"}, {7, "8khz_bw3281"}) }}, \
  {"sensor.imu.accrate", {1, NAMESET({1, "1khz_bw218"}, {2, "1khz_bw99"}, {3, "1khz_bw44"}, \
                                      {4, "1khz_bw21"}, {5, "1khz_bw10"}, {6, "1khz_bw5"}, \
				      {7, "1khz_bw420"}, {8, "4khz_bw1046"}) }}, \
  {"sensor.imu.fchoicerate", {1, NAMESET({0, "nofchoice"}, \
			    {1, "32K_BW_8173"}, {2, "32K_BW_3281"}) }}, \
  {"sensor.imu.gyrorange", {0, NAMESET({0U<<3, "250_dps"}, {1U<<3, "500_dps"},	\
				      {2U<<3, "1000_dps"}, {3U<<3, "2000_dps"}) }}, \
  {"sensor.imu.accrange", {0, NAMESET({0U<<3, "2g"}, {1U<<3, "4g"},	\
				      {2U<<3, "8g"}, {3U<<3, "16g"}) }}
