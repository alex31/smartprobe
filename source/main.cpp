#include <ch.h>
#include <hal.h>
#include <cstdio>
#include "stdutil.h"		// necessaire pour initHeap
#include "ttyConsole.hpp"       // fichier d'entête du shell
#include "whiteBoard.hpp"
#include "printf.h"


/*
  Câbler une LED sur la broche C0


  ° connecter B6 (uart1_tx) sur PROBE+SERIAL Rx AVEC UN JUMPER
  ° connecter B7 (uart1_rx) sur PROBE+SERIAL Tx AVEC UN JUMPER
  ° connecter C0 sur led0

 */


volatile size_t nbReads = 0U;
volatile size_t nbWrites = 0U;


WhiteBoard<size_t, UpdateBehavior::Verify> t;


static THD_WORKING_AREA(waBlinker, 304);	// declaration de la pile du thread blinker
static void blinker (void *arg)			// fonction d'entrée du thread blinker
{
  (void)arg;					// on dit au compilateur que "arg" n'est pas utilisé
  chRegSetThreadName("blinker");		// on nomme le thread
  
  while (true) {				// boucle infinie
    palToggleLine(LINE_LED1);			// clignotement de la led 
    chThdSleepMilliseconds(1000);		// à la féquence de 1 hertz
    chprintf(chp, "w=%d  r=%d\r\n", nbWrites, nbReads);
  }
}

static void reader (void *arg)			// fonction d'entrée du thread blinker
{
  char rname[] = "reader # ";
  rname[7] = '1' + (uint32_t) arg ;
  chRegSetThreadName(rname);		// on nomme le thread
  size_t r;

  event_listener_t t_update;
  t.registerEvt(&t_update, 1U);
  while (true) {
    t.read(r, 1U);
    nbReads++;
  }
}

static void writer (void *arg)			// fonction d'entrée du thread blinker
{
  (void) arg;
  size_t w=0;
  uint32_t inc=0;
    
  while (true) {
    w += inc++;
    inc = inc % 2;
    t.write(w);
    nbWrites++;
    chThdSleepMicroseconds(1000);
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

  for (size_t i=0; i<5; i++) {
    chThdCreateFromHeap(NULL, 512, NULL, NORMALPRIO, &reader, (void *) i);
  }
  
  chThdCreateFromHeap(NULL, 512, "writer", NORMALPRIO, &writer, NULL);

  // cette fonction en interne fait une boucle infinie, elle ne sort jamais
  // donc tout code situé après ne sera jamais exécuté.
  consoleLaunch();  // lancement du shell

  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}


