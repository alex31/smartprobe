#include "threadAndEventDecl.hpp"
#include "hardwareConf.hpp"

extern "C" {
__attribute__((used))
void _fini(void) { }
}

ConfigurationFile confFile(CONFIGURATION_FILENAME);
DifferentialPressure dp(NORMALPRIO);
Barometer 	  baro(NORMALPRIO);
Adc 		  adc(NORMALPRIO);
Imu		  imu(NORMALPRIO);
SdCard            sdcard(NORMALPRIO);
ShowBlackboard    showBB(NORMALPRIO);
Ahrs		  ahrs(NORMALPRIO);
Relwind		  relwind(NORMALPRIO);
ReceivePprzlink   receivePPL(NORMALPRIO);
ReceiveNmealink	  receiveNMEA(NORMALPRIO);
ReceiveUbxlink	  receiveUBX(NORMALPRIO);
FrontLed IN_DMA_SECTION(fl);
