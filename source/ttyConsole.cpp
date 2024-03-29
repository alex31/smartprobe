#define _GNU_SOURCE // enable strcasestr

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include "ch.h"
#include "hal.h"
#include "microrl/microrlShell.h"
#include "stdutil.h"
#include "printf.h"
#include "rtcAccess.h"
#include "ttyConsole.hpp"
#include "hardwareConf.hpp"
#include "cpp_heap_alloc.hpp"
#include "tlsf_malloc.h"
#include "threadAndEventDecl.hpp"
#include "healthCheck.hpp"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"

/*===========================================================================*/
/* START OF EDITABLE SECTION                                           */
/*===========================================================================*/

// declaration des prototypes de fonction
// ces declarations sont necessaires pour remplir le tableau commands[] ci-dessous
using cmd_func_t =  void  (BaseSequentialStream *lchp, int argc,const char * const argv[]);
static cmd_func_t cmd_mem, cmd_uid, cmd_restart, cmd_param, cmd_close, 
  cmd_rtc, cmd_toggleSendSerialMessages, cmd_eigen, cmd_conf;
#if CH_DBG_STATISTICS
static cmd_func_t cmd_threads;
#endif

static bool sendSerialMessages = true;

bool shouldSendSerialMessages(void)
{
  return sendSerialMessages;
}

static const ShellCommand commands[] = {
  {"mem", cmd_mem},		// affiche la mémoire libre/occupée
#if  CH_DBG_STATISTICS
  {"threads", cmd_threads},	// affiche pour chaque thread le taux d'utilisation de la pile et du CPU
#endif
  {"rtc", cmd_rtc},		// affiche l'heure contenue par la RTC
  {"uid", cmd_uid},		// affiche le numéro d'identification unique du MCU
  {"param", cmd_param},		// fonction à but pedagogique qui affiche les
				//   paramètres qui lui sont passés

  {"restart", cmd_restart},	// reboot MCU
  {"close", cmd_close},	// reboot MCU
  {"eigen", cmd_eigen},	// test eigen
  {"conf", cmd_conf},	// show conf file parameters
  {"t", cmd_toggleSendSerialMessages},	// reboot MCU
  {NULL, NULL}			// marqueur de fin de tableau
};



/*
  definition de la fonction cmd_param asociée à la commande param (cf. commands[])
  cette fonction a but pédagogique affiche juste les paramètres fournis, et tente
  de convertir les paramètres en entier et en flottant, et affiche le resultat de
  cette conversion. 
  une fois le programme chargé dans la carte, essayer de rentrer 
  param toto 10 10.5 0x10
  dans le terminal d'eclipse pour voir le résultat 
 */
static void cmd_param(BaseSequentialStream *lchp, int argc,const char* const argv[])
{
  if (argc == 0) {  // si aucun paramètre n'a été passé à la commande param 
    chprintf(lchp, "pas de paramètre en entrée\r\n");
  } else { // sinon (un ou plusieurs pararamètres passés à la commande param 
    for (int argn=0; argn<argc; argn++) { // pour tous les paramètres
      chprintf(lchp, "le parametre %d/%d est %s\r\n", argn, argc-1, argv[argn]); // afficher

      // tentative pour voir si la chaine peut être convertie en nombre entier et en nombre flottant
      int entier = atoi (argv[argn]); // atoi converti si c'est possible une chaine en entier
      float flottant = atof (argv[argn]); // atof converti si c'est possible une chaine en flottant

      chprintf(lchp, "atoi(%s) = %d ;; atof(%s) = %.3f\r\n",
		argv[argn], entier, argv[argn], flottant);
    }
  }
}

static void cmd_toggleSendSerialMessages(BaseSequentialStream *lchp,
					 int argc,const char* const argv[])
{
  (void) lchp;
  (void) argc;
  (void) argv;

  sendSerialMessages = not sendSerialMessages;
}

static void cmd_close(BaseSequentialStream *lchp, int argc,const char* const argv[])
{
  (void) lchp;
  (void) argc;
  (void) argv;
  DebugTrace("close all (FLUSH) wait 200 milliseconds");
  stopAllPeripherals();
  
  SdLiteLogBase::terminate(TerminateBehavior::WAIT);
  chThdSleepMilliseconds(300);
  f_mount(NULL, "", 0); // umount
}

/*
 a = np.array([[ 5, 1 ,3], 
                  [ 1, 1 ,1], 
                  [ 1, 2 ,1]])
 b = np.array([1, 2, 3])

 print a.dot(b)
       array([16, 6, 8])
 */
static void cmd_eigen(BaseSequentialStream *lchp, int argc,const char* const argv[])
{
  using namespace Eigen;
  (void) lchp;
  Vector2f v2f[3];

  if (argc != 0) {
    if (argc < 4) {
      chprintf(lchp, "not enough argument (need 4)\r\n");
      return;
    }
    
    v2f[0] << atof(argv[0]), atof(argv[1]);
    v2f[1] << atof(argv[2]), atof(argv[3]);
    v2f[2] = v2f[0] + v2f[1];
    chprintf(lchp, "[%f, %f] + [%f, %f] = [%f, %f]\r\n",
	     v2f[0](0), v2f[0](1), 
	     v2f[1](0), v2f[1](1), 
	     v2f[2](0), v2f[2](1));
  }

  Eigen::Matrix<float, 3, 3>  mat;
  mat << 5, 1, 3,
         1, 1, 1,
         1, 2, 1;
  
  Eigen::Matrix<float, 3, 1> u(1, 2, 3);

  auto r = mat * u;

  chprintf(lchp, "u[%d*%d] = [%f, %f, %f]\r\n", u.rows(), u.cols(), u(0), u(1), u(2));
  chprintf(lchp, "r[%d*%d] = [%f, %f, %f]\r\n", r.rows(), r.cols(), r(0), r(1), r(2));
}



static void cmd_conf(BaseSequentialStream *lchp, int argc,const char* const argv[])
{
  (void) argc;
  (void) argv;

  std::string_view syslogName = CONF("syslog.filename");
  
  chprintf(lchp, "%.*s", syslogName.size(), syslogName.data());
}

/*
  conf :

  commandes sans arguments
  show
  store
  load
  wipe
  erase

  pour les commandes suivantes : cmd val : affecte la valeur, cmd : affiche la valeur
  magnet 
  motor
  window
  median
  rate
  baud
  smin
  smax
 */

using pGetFunc_t = uint32_t (*) (void);
using pSetFunc_t  = void (*) (uint32_t);

static void cmd_restart(BaseSequentialStream *lchp, int argc,const char* const argv[])
{
  (void) lchp;
  (void) argc;
  (void) argv;
  systemReset();
}



/*
  
 */


/*===========================================================================*/
/* START OF PRIVATE SECTION  : DO NOT CHANGE ANYTHING BELOW THIS LINE        */
/*===========================================================================*/

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/


#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(4096)

#ifndef CONSOLE_DEV_USB
#define  CONSOLE_DEV_USB 0
#endif

static void cmd_uid(BaseSequentialStream *lchp, int argc,const char* const argv[]) {
  (void)argv;
  if (argc > 0) {
     chprintf(lchp, "Usage: uid\r\n");
    return;
  }

  for (uint32_t i=0; i< UniqProcessorIdLen; i++)
    chprintf(lchp, "[%x] ", UniqProcessorId[i]);
  chprintf(lchp, "\r\n");
}



static void cmd_mem(BaseSequentialStream *lchp, int argc,const char* const argv[]) {
  (void)argv;
  if (argc > 0) {
    chprintf(lchp, "Usage: mem\r\n");
    return;
  }

  chprintf(lchp, "ENTRY tlsf check = %d (0 is ok)\r\n",
	   tlsf_check_r(&HEAP_DEFAULT));

  chprintf(lchp, "core free memory : %u bytes\r\n", chCoreGetStatusX());
  chprintf(lchp, "heap free memory : %u bytes\r\n", getHeapFree());

  void * ptr1 = malloc_m (100);
  void * ptr2 = malloc_m (100);

  chprintf(lchp, "(2x) malloc_m(1000) = %p ;; %p\r\n", ptr1, ptr2);
  chprintf(lchp, "heap free memory : %d bytes\r\n", getHeapFree());

  free_m (ptr1);
  free_m (ptr2);

  chprintf(lchp, "EXIT tlsf check = %d (0 is ok)\r\n",
	   tlsf_check_r(&HEAP_DEFAULT));

}

static void cmd_rtc(BaseSequentialStream *lchp, int argc,const char* const argv[])
{
  if ((argc != 0) && (argc != 2) && (argc != 6)) {
     DebugTrace ("Usage: rtc [Hour Minute Second Year monTh Day day_of_Week Adjust] value or");
     DebugTrace ("Usage: rtc  Hour Minute Second Year monTh Day");
     return;
  }
 
  if (argc == 2) {
    const char timeVar = (char) tolower ((int) *(argv[0]));
    const int32_t varVal = strtol (argv[1], NULL, 10);
    
    switch (timeVar) {
    case 'h':
      setHour ((uint32_t)(varVal));
      break;
      
    case 'm':
       setMinute ((uint32_t)(varVal));
      break;
      
    case 's':
      setSecond ((uint32_t)(varVal));
      break;
      
    case 'y':
       setYear ((uint32_t)(varVal));
      break;
      
    case 't':
       setMonth ((uint32_t)(varVal));
      break;
      
    case 'd':
       setMonthDay ((uint32_t)(varVal));
      break;

    case 'w':
       setWeekDay ((uint32_t)(varVal));
      break;

    case 'a':
      {
	int32_t newSec =(int)(getSecond()) + varVal;
	if (newSec > 59) {
	  int32_t newMin =(int)(getMinute()) + (newSec/60);
	  if (newMin > 59) {
	    setHour ((getHour()+((uint32_t)(newMin/60))) % 24);
	    newMin %= 60;
	  }
	  setMinute ((uint32_t)newMin);
	}
	if (newSec < 0) {
	  int32_t newMin =(int)getMinute() + (newSec/60)-1;
	  if (newMin < 0) {
	    setHour ((getHour()-((uint32_t)newMin/60)-1) % 24);
	    newMin %= 60;
	  }
	  setMinute ((uint32_t)newMin);
	}
	setSecond ((uint32_t)newSec % 60);
      }
      break;
      
    default:
      DebugTrace ("Usage: rtc [Hour Minute Second Year monTh Day Weekday Adjust] value");
    }
  } else if (argc == 6) {
    setYear ((uint32_t) atoi(argv[3]));
    setMonth ((uint32_t) atoi(argv[4]));
    setMonthDay ((uint32_t) atoi(argv[5]));
    setHour ((uint32_t) atoi(argv[0]));
    setMinute ((uint32_t) atoi(argv[1]));
    setSecond ((uint32_t) atoi(argv[2]));
  }

  chprintf (lchp, "RTC : %s %.02lu/%.02lu/%.04lu  %.02lu:%.02lu:%.02lu\r\n",
	    getWeekDayAscii(), getMonthDay(), getMonth(), getYear(),
	    getHour(), getMinute(), getSecond());
}



#if  CH_DBG_STATISTICS
static void cmd_threads(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp = chRegFirstThread();
  (void)argv;
  (void)argc;
  float totalTicks=0;
  float idleTicks=0;

  static ThreadCpuInfo threadCpuInfo;
  
  stampThreadCpuInfo (&threadCpuInfo);
  
  chprintf (lchp, "    addr    stack  frestk prio refs  state        time \t percent        name\r\n");
  uint32_t idx=0;
  do {
    chprintf (lchp, "%.8lx %.8lx %6lu %4lu %4lu %9s %9lu   %.2f%%    \t%s\r\n",
	      (uint32_t)tp, (uint32_t)tp->ctx.sp,
	      get_stack_free (tp),
	      (uint32_t)tp->hdr.pqueue.prio, (uint32_t)(tp->refs - 1),
	      states[tp->state],
	      (uint32_t)RTC2MS(STM32_SYSCLK, tp->stats.cumulative),
	      stampThreadGetCpuPercent (&threadCpuInfo, idx),
	      chRegGetThreadNameX(tp));

    totalTicks+= (float)tp->stats.cumulative;
    if (strcmp(chRegGetThreadNameX(tp), "idle") == 0)
    idleTicks = (float)tp->stats.cumulative;
    tp = chRegNextThread ((thread_t *)tp);
    idx++;
  } while (tp != NULL);

  const float idlePercent = (idleTicks*100.f)/totalTicks;
  const float cpuPercent = 100.f - idlePercent;
  chprintf (lchp, "Interrupt Service Routine \t\t     %9lu   %.2f%%    \tISR\r\n",
	    (uint32_t)RTC2MS(STM32_SYSCLK,threadCpuInfo.totalISRTicks),
	    stampISRGetCpuPercent(&threadCpuInfo));
  chprintf (lchp, "\r\ncpu load = %.2f%%\r\n", cpuPercent);
}
#endif

static const ShellConfig shell_cfg1 = {
#if CONSOLE_DEV_USB == 0
  (BaseSequentialStream *) &CONSOLE_DEV_SD,
#else
  (BaseSequentialStream *) &SDU1,
#endif
  commands
};



void consoleInit (void)
{
  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * USBD1 : FS, USBD2 : HS
   */

#if CONSOLE_DEV_USB != 0
  usbSerialInit(&SDU1, &USBDRIVER); 
  chp = (BaseSequentialStream *) &SDU1;
#else
  sdStart(&CONSOLE_DEV_SD, &serialDebugConsoleCfg);
  chp = (BaseSequentialStream *) &CONSOLE_DEV_SD;
#endif
  /*
   * Shell manager initialization.
   */
  shellInit();
}


void consoleLaunch (void)
{
  thread_t *shelltp = NULL;

 
#if CONSOLE_DEV_USB != 0
  if (!shelltp) {
    while (usbGetDriver()->state != USB_ACTIVE) {
      chThdSleepMilliseconds(10);
    }
    
    // activate driver, giovani workaround
    chnGetTimeout(&SDU1, TIME_IMMEDIATE);
    while (!isUsbConnected()) {
      chThdSleepMilliseconds(10);
    }
    shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    palSetLine(LINE_USB_LED);
  } else if (shelltp && (chThdTerminated(shelltp))) {
    chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
    shelltp = NULL;           /* Triggers spawning of a new shell.        */
  }

#else // CONSOLE_DEV_USB == 0

   if (!shelltp) {
     shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
   } else if (chThdTerminatedX(shelltp)) {
     chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
     shelltp = NULL;           /* Triggers spawning of a new shell.        */
   }
   chThdSleepMilliseconds(100);
   
#endif //CONSOLE_DEV_USB

}


#pragma GCC diagnostic pop
