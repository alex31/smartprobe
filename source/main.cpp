#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"		
#include "ttyConsole.hpp"       
#include "confFile.hpp"
#include "hardwareConf.hpp"
#include "workerClass.hpp"
#include "threadAndEventDecl.hpp"
#include "blinker.hpp"
#include "usbStorage.hpp"
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


int main (void)
{
  Blinker bl(NORMALPRIO);
  UsbStorage usbStorage(NORMALPRIO);

  bl.run(TIME_MS2I(1000));
  consoleInit();    // initialisation des objets liés au shell
  consoleLaunch();  // lancement du shell

  if (not sdcard.run(TIME_IMMEDIATE)) {
     SdCard::logSyslog(Severity::Fatal, "SDCARD fail");
  } else  if (not adc.run(TIME_IMMEDIATE)) {
    SdCard::logSyslog(Severity::Fatal, "ADC fail");
  } else if (not baro.run(TIME_IMMEDIATE)) {
    SdCard::logSyslog(Severity::Fatal, "BARO fail");
  } else  if (not dp.run(TIME_MS2I(10))) {
    SdCard::logSyslog(Severity::Fatal, "DIFF PRESS fail");
  } else if (not imu.run(TIME_MS2I(2))) {
    SdCard::logSyslog(Severity::Fatal, "IMU fail");
  } else if (not showBB.run(TIME_MS2I(50))) {
    SdCard::logSyslog(Severity::Fatal, "Show Blackboard fail");
  } else if (not usbStorage.run(TIME_IMMEDIATE)) {
    SdCard::logSyslog(Severity::Fatal, "USB Storage fail");
  } else {
    // if all went ok, main thead now can rest
    chThdSleep(TIME_INFINITE);
  }

  // if something goes wrong, control finish here
  palSetLine(LINE_LED_RED);
  chThdSleep(TIME_INFINITE);
}


