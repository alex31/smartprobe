#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"
#include "cpp_heap_alloc.hpp"
#include "ttyConsole.hpp"       
#include "confFile.hpp"
#include "workerClass.hpp"
#include "threadAndEventDecl.hpp"
//#include "blinker.hpp"
#include "usbStorage.hpp"
#include "dynSwdio.hpp"
#include "transmitPprzlink.hpp"
#include "receivePprzlink.hpp"
#include "receiveNmealink.hpp"
#include "rtcSync.hpp"
#include "healthCheck.hpp"
#include "util.hpp"
#include "printf.h"



/*
  Câbler une LED sur la broche C0


  ° connecter B6 (uart1_tx) sur PROBE+SERIAL Rx AVEC UN JUMPER
  ° connecter B7 (uart1_rx) sur PROBE+SERIAL Tx AVEC UN JUMPER
  ° connecter C0 sur led0

 */


void _init_chibios() __attribute__ ((constructor(101)));
void _init_chibios() {
  halInit();
  chSysInit();
  initHeap ();
}

extern FrontLed IN_DMA_SECTION(fl);

int main (void)
{
  //  Blinker       bl(NORMALPRIO+1);
  UsbStorage    usbStorage(NORMALPRIO);
  DynSwdio	dynSwdio(NORMALPRIO);
  TransmitPprzlink transmitPPL(NORMALPRIO);
  RtcSync	rtcSync(NORMALPRIO);
  HealthCheck   healthCheck(NORMALPRIO-1);
  
  fl.setError(LedCode::Starting);

  //  bl.run(TIME_MS2I(1000));
  
#ifdef TRACE
  consoleInit();    // initialisation des objets liés au shell
  consoleLaunch();  // lancement du shell
#endif

  // first time without loggin error to get early parameters
  if (not sdcard.initHardware()) {
    DebugTrace("sdcard.initHardware() has failed");
    fl.setError(LedCode::NoSdCard);
  } else if (not confFile.earlyReadConfFile()) {
    DebugTrace("early confFile.readConfFile() has failed");
    fl.setError(LedCode::ConfigError);
  } else if (not sdcard.run(TIME_IMMEDIATE)) {
    chprintf(chp, "SDCARD launch fail");
    fl.setError(LedCode::NoSdCard);
  } else if (chThdSleepMilliseconds(300); not confFile.populate()) { // second time to log errors
    chprintf(chp, "read CONFIGURATION file fail");
    fl.setError(LedCode::ConfigError);
  } else if (not baro.run(TIME_IMMEDIATE)) {
    SdCard::logSyslog(Severity::Fatal, "BARO fail");
    fl.setError(LedCode::HardFault);
  } else  if (not dp.run(PERIOD("thread.frequency.d_press"))) {
    SdCard::logSyslog(Severity::Fatal, "DIFF PRESS fail");
    fl.setError(LedCode::HardFault);
   } else if (not imu.run(PERIOD("thread.frequency.imu"))) {
     SdCard::logSyslog(Severity::Fatal, "IMU fail");
    fl.setError(LedCode::HardFault);
   } else if (not ahrs.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "Ahrs fail");
    fl.setError(LedCode::HardFault);
   } else if (not relwind.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "relative wind fail");
    fl.setError(LedCode::HardFault);
   } else if (not usbStorage.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "USB Storage fail");
    fl.setError(LedCode::HardFault);
   } else if (not adc.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "ADC fail");
    fl.setError(LedCode::HardFault);
  } else if (not dynSwdio.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "dynSwdio fail");
    fl.setError(LedCode::HardFault);
  } else if (not healthCheck.run(TIME_S2I(1))) { // check internal voltage, thread memory, cpu usage
     SdCard::logSyslog(Severity::Fatal, "healthCheck fail");
    fl.setError(LedCode::HardFault);
  } else if (not rtcSync.run(TIME_S2I(60))) { // sync rtc with gps every minutes
     SdCard::logSyslog(Severity::Fatal, "rtcSync fail");
    fl.setError(LedCode::HardFault);
  } else {
#ifdef TRACE
    if (not showBB.run(TIME_IMMEDIATE)) {
      SdCard::logSyslog(Severity::Fatal, "Show Blackboard fail");
      fl.setError(LedCode::HardFault);
    } 
#endif
    const SerialMode smode = static_cast<SerialMode>(CONF("uart.mode"));
    switch (smode) {
    case SERIAL_NOT_USED :
      SdCard::logSyslog(Severity::Warning, "mode serial line NOT USED");
      break;
    case PPRZ_IN_OUT:
      transmitPPL.run(TIME_IMMEDIATE);
      receivePPL.run(TIME_IMMEDIATE);
      break;
    case NMEA_IN:
     receiveNMEA.run(TIME_IMMEDIATE);
      break;
    case UBX_IN:
      receiveUBX.run(TIME_IMMEDIATE);
      break;
    }
    const LedMode ledmode = static_cast<LedMode>(CONF("led.mode"));
    if (ledmode == LED_STATUS) {
      // if all went ok, main thead now can rest
      fl.setError(LedCode::Optimal);
    } else {
      // demo mode where led reflect 3D airspeed measures
      fl.setError(LedCode::DirectColorSetting);
      demoled.run(TIME_MS2I(30));
    }
    chThdSleep(TIME_INFINITE);
  }

  // if something goes wrong, control finish here
  // still offer usb storage facility, so in case of configuration file
  // error, one can still mount the device to read syslog and fix conf file
  palSetLine(LINE_LED_RED);
  usbStorage.setModeEmergency();
  usbStorage.run(TIME_IMMEDIATE);
  chThdSleep(TIME_INFINITE);
}


