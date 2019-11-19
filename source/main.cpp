#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"		
#include "ttyConsole.hpp"       
#include "confFile.hpp"
#include "hardwareConf.hpp"
#include "workerClass.hpp"
#include "barometer.hpp"
#include "differentialPressure.hpp"
#include "blinker.hpp"
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
  DifferentialPressure dp(NORMALPRIO);
  Barometer baro(NORMALPRIO);
  Blinker bl(NORMALPRIO);

  bl.run();
  consoleInit();	// initialisation des objets liés au shell

  consoleLaunch();  // lancement du shell

  if (baro.run() == false) {
    DebugTrace("barometer fail");
    goto fail;
  }

  // if (dp.run() != true) {
  //    DebugTrace("differential pressure fail");
  //    goto fail;
  // }

 fail:
  chThdSleep(TIME_INFINITE);
}


