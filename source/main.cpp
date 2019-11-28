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

  if (sdcard.run(TIME_IMMEDIATE) != true) {
     SdCard::logSyslog(Severity::Fatal, "SDCARD fail");
     goto fail;
  }

  if (adc.run(TIME_IMMEDIATE) == false) {
    SdCard::logSyslog(Severity::Fatal, "ADC fail");
    goto fail;
  }

  if (baro.run(TIME_IMMEDIATE) == false) {
    SdCard::logSyslog(Severity::Fatal, "BARO fail");
    goto fail;
  }

  if (dp.run(TIME_MS2I(10)) != true) {
    SdCard::logSyslog(Severity::Fatal, "DIFF PRESS fail");
    goto fail;
  }
  
  if (imu.run(TIME_MS2I(2)) == false) {
    SdCard::logSyslog(Severity::Fatal, "IMU fail");
    goto fail;
  }

  if (showBB.run(TIME_MS2I(50)) != true) {
    SdCard::logSyslog(Severity::Fatal, "Show Blackboard fail");
    goto fail;
  }
  
  if (usbStorage.run(TIME_IMMEDIATE) != true) {
    SdCard::logSyslog(Severity::Fatal, "USB Storage fail");
    goto fail;
  }
  
  chThdSleep(TIME_INFINITE);

 fail:
   palSetLine(LINE_LED_RED);

   chThdSleep(TIME_INFINITE);
}


