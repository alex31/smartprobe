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
#include "receiveNmealink.hpp"
#include "rtcSync.hpp"
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


int main (void)
{
  Blinker       bl(NORMALPRIO+1);
  bl.run(TIME_MS2I(1000));

  consoleInit();    // initialisation des objets liés au shell
  consoleLaunch();  // lancement du shell


  // if something goes wrong, control finish here
  // still offer usb storage facility, so in case of configuration file
  // error, one can still mount the device to read syslog and fix conf file
  
  // palSetLine(LINE_LED_RED);
  // usbStorage.setModeEmergency();
  // usbStorage.run(TIME_IMMEDIATE);
  chThdSleep(TIME_INFINITE);
}


