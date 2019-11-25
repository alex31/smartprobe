#include "sensorsDecl.hpp"

extern "C" {
__attribute__((used))
void _fini(void) { }
}

DifferentialPressure dp(NORMALPRIO);
Barometer baro(NORMALPRIO);
Adc adc(NORMALPRIO);
Imu imu(NORMALPRIO);
