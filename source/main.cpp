#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"
#include "cpp_heap_alloc.hpp"
#include "ttyConsole.hpp"       
#include "confFile.hpp"
#include "workerClass.hpp"
#include "threadAndEventDecl.hpp"
#include "blinker.hpp"
#include "usbStorage.hpp"
#include "dynSwdio.hpp"
#include "transmitPprzlink.hpp"
#include "receivePprzlink.hpp"
#include "printf.h"


/*
  Câbler une LED sur la broche C0


  ° connecter B6 (uart1_tx) sur PROBE+SERIAL Rx AVEC UN JUMPER
  ° connecter B7 (uart1_rx) sur PROBE+SERIAL Tx AVEC UN JUMPER
  ° connecter C0 sur led0

 */


#define PERIOD(k) (CH_CFG_ST_FREQUENCY / CONF(k))


void _init_chibios() __attribute__ ((constructor(101)));
void _init_chibios() {
  halInit();
  chSysInit();
  initHeap ();
}


int main (void)
{
  Blinker       bl(NORMALPRIO+1);
  UsbStorage    usbStorage(NORMALPRIO);
  DynSwdio	dynSwdio(NORMALPRIO);
  TransmitPprzlink transmitPPL(NORMALPRIO);
  ReceivePprzlink receivePPL(NORMALPRIO);
  
  bl.run(TIME_MS2I(1000));

#ifdef TRACE
  consoleInit();    // initialisation des objets liés au shell
  consoleLaunch();  // lancement du shell
#endif

  if (not sdcard.run(TIME_IMMEDIATE)) {
    chprintf(chp, "SDCARD fail");
  } else if (not confFile.populate()) {
    SdCard::logSyslog(Severity::Fatal, "Read configuration file fail");
  } else if (not baro.run(TIME_IMMEDIATE)) {
    SdCard::logSyslog(Severity::Fatal, "BARO fail");
  } else  if (not dp.run(PERIOD("thread.frequency.d_press"))) {
    SdCard::logSyslog(Severity::Fatal, "DIFF PRESS fail");
   } else if (not imu.run(PERIOD("thread.frequency.imu"))) {
     SdCard::logSyslog(Severity::Fatal, "IMU fail");
   } else if (not ahrs.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "Ahrs fail");
   } else if (not relwind.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "relative wind fail");
   } else if (not usbStorage.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "USB Storage fail");
   } else if (not adc.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "ADC fail");
  } else if (not dynSwdio.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "dynSwdio fail");
  } else {
    const SerialMode smode = static_cast<SerialMode>(CONF("uart.mode"));
    switch (smode) {
    case SHELL:
#ifndef TRACE
      consoleInit();    // initialisation des objets liés au shell
      consoleLaunch();  // lancement du shell
      SdCard::logSyslog(Severity::Warning, "mode shell : you should compile with "
			"-DTRACE for more verbosity");
      if (not showBB.run(TIME_IMMEDIATE)) {
	SdCard::logSyslog(Severity::Fatal, "Show Blackboard fail");
   } 
#endif
      break;
    case PPRZ_IN_OUT:
#ifdef TRACE
      SdCard::logSyslog(Severity::Fatal, "-DTRACE non compatible with mode PPRZ_IN_OUT");
      goto error;
#endif
      transmitPPL.run(TIME_IMMEDIATE);
      receivePPL.run(TIME_IMMEDIATE);
      break;
    case NMEA_IN:
#ifdef TRACE
      SdCard::logSyslog(Severity::Fatal, "-DTRACE non compatible with mode NMEA_IN");
      goto error;
#endif
      break;
    case UBLOX_IN:
#ifdef TRACE
      SdCard::logSyslog(Severity::Fatal, "-DTRACE non compatible with mode UBLOX_IN");
      goto error;
#endif
      break;
    }
    // if all went ok, main thead now can rest
    chThdSleep(TIME_INFINITE);
  }

  // if something goes wrong, control finish here
  // still offer usb storage facility, so in case of configuration file
  // error, one can still mount the device to read syslog and fix conf file
#ifdef TRACE
 error:
#endif
  
  palSetLine(LINE_LED_RED);
  usbStorage.setModeEmergency();
  usbStorage.run(TIME_IMMEDIATE);
  chThdSleep(TIME_INFINITE);
}


