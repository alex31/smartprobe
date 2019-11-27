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
     DebugTrace("SDCARD fail");
     goto fail;
  }

  if (adc.run(TIME_IMMEDIATE) == false) {
    DebugTrace("ADC fail");
    goto fail;
  }

  if (baro.run(TIME_IMMEDIATE) == false) {
    DebugTrace("BARO fail");
    goto fail;
  }

  if (dp.run(TIME_MS2I(10)) != true) {
    DebugTrace("DIFF PRESS fail");
    goto fail;
  }
  
  if (imu.run(TIME_MS2I(10)) == false) {
    DebugTrace("IMU fail");
    goto fail;
  }

   if (showBB.run(TIME_MS2I(2000)) != true) {
     DebugTrace("Show Blackboard fail");
     goto fail;
  }
 
 fail:
   if (usbStorage.run(TIME_IMMEDIATE) != true) {
     DebugTrace("USB Storage fail");
  }
 
  chThdSleep(TIME_INFINITE);
}


