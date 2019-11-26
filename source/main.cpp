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

  bl.run();
  consoleInit();    // initialisation des objets liés au shell
  consoleLaunch();  // lancement du shell

  if (sdcard.run() != true) {
     DebugTrace("SDCARD fail");
     goto fail;
  }

  if (adc.run() == false) {
    DebugTrace("ADC fail");
    goto fail;
  }

  if (baro.run() == false) {
    DebugTrace("BARO fail");
    goto fail;
  }

  if (dp.run() != true) {
    DebugTrace("DIFF PRESS fail");
    goto fail;
  }
  
  if (imu.run() == false) {
    DebugTrace("IMU fail");
    goto fail;
  }

   if (showBB.run() != true) {
     DebugTrace("Show Blackboard fail");
     goto fail;
  }
 
 fail:
   if (usbStorage.run() != true) {
     DebugTrace("USB Storage fail");
  }
 
   
  chThdSleep(TIME_INFINITE);
}


