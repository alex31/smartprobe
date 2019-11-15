#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"		// necessaire pour initHeap
#include "ttyConsole.hpp"       // fichier d'entête du shell
#include "confFile.hpp"
#include "hardwareConf.hpp"
#include "barometer.hpp"
#include "printf.h"


/*
  Câbler une LED sur la broche C0


  ° connecter B6 (uart1_tx) sur PROBE+SERIAL Rx AVEC UN JUMPER
  ° connecter B7 (uart1_rx) sur PROBE+SERIAL Tx AVEC UN JUMPER
  ° connecter C0 sur led0

 */




static THD_WORKING_AREA(waBlinker, 304);	// declaration de la pile du thread blinker
static void blinker (void *arg)			// fonction d'entrée du thread blinker
{
  (void)arg;					// on dit au compilateur que "arg" n'est pas utilisé
  chRegSetThreadName("blinker");		// on nomme le thread
  
  while (true) {				// boucle infinie
    palToggleLine(LINE_LED1);			// clignotement de la led
    chThdSleepMilliseconds(500);
  }
}
   
void _init_chibios() __attribute__ ((constructor(101)));
void _init_chibios() {
  halInit();
  chSysInit();
  initHeap ();
}

int main (void)
{
  consoleInit();	// initialisation des objets liés au shell
  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO+2, &blinker, NULL); // lancement du thread

  consoleLaunch();  // lancement du shell
  if (barometer::launchThd() == false) {
    chSysHalt("barometer fail");
  }

  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}


