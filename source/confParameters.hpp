#define PARAMETERS_MAP							\
  {"filename.sensors", {"capteurs"sv, NONAMESET }},			\
  {"sensor.pressure.frequency", {1, NAMESET({1, "frequency_one"}) }}, \
  {"sensor.other", {2, NAMESET({1, "other_one"}, {2, "other_two"}) }},  \
  {"sensor.o2", {3, NAMESET({1, "o2_low"}, {3, "o2_high"}) }},	      \
  {"thread.waitmillisecond", {20.5f, RANGEINT(9, 1000) }},	\
  {"sensor.icm20600.init.cr1", {0, RANGEINT(0, 16) }},	\
  {"thread.enable.imu", {true, NONAMESET }},			\
  {"thread.frequency", {100, RANGEINT(10, 1000) }}
  
