#include <ch.h>
#include <hal.h>
#include "dynSwdio.hpp"
#include "stdutil.h"	
#include "threadAndEventDecl.hpp"


bool DynSwdio::init()
{
#if SWDIO_DETECTION != 0
  palSetLineMode(LINE_SWCLK, PAL_MODE_INPUT_PULLDOWN);
  chThdSleepMilliseconds(1);
  palEnableLineEvent(LINE_SWCLK, PAL_EVENT_MODE_BOTH_EDGES);
#endif
  return true;
}


bool DynSwdio::loop()
{
  chRegSetThreadName("DynSwdio:polling");

#if SWDIO_DETECTION == 0
  chThdExit(0);
#else

  palWaitLineTimeout(LINE_SWCLK, TIME_INFINITE);
  fl.setError(LedCode::SwdioModeStart);

  chRegSetThreadName("DynSwdio:wait dp join");
  dp.terminate().join();
  chRegSetThreadName("DynSwdio:wait baro join");
  baro.terminate().join();
  chRegSetThreadName("DynSwdio:wait imu join");
  imu.terminate().join();
  chRegSetThreadName("DynSwdio:wait sdcard join");
  sdcard.terminate().join();
  chRegSetThreadName("DynSwdio:wait showBB join");
  showBB.terminate().join();

  SdLiteLogBase::terminate(TerminateBehavior::WAIT);
  f_mount(NULL, "", 0);
  palSetLineMode(LINE_SWCLK, PAL_MODE_ALTERNATE(AF_LINE_SWCLK));
  DebugTrace("Enter SWDIO mode");
  chThdSleepMilliseconds(200); // wait for dma buffer to be flushed to sdcard
  palSetLineMode(LINE_SWCLK, PAL_MODE_ALTERNATE(AF_LINE_SWCLK));
  // swdio will take over
  fl.setError(LedCode::SwdioModeWait);
  chThdSleep(TIME_INFINITE);
 #endif
  
  return true;
}

