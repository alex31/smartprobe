#include <ch.h>
#include <hal.h>
#include "dynSwdio.hpp"
#include "stdutil.h"	
#include "threadAndEventDecl.hpp"


bool DynSwdio::init()
{
  palSetLineMode(LINE_SWCLK, PAL_MODE_INPUT_PULLDOWN);
  chThdSleepMilliseconds(1);
  palEnableLineEvent(LINE_SWCLK, PAL_EVENT_MODE_BOTH_EDGES);
  return true;
}


bool DynSwdio::loop()
{
  chRegSetThreadName("DynSwdio:polling");
  
  palWaitLineTimeout(LINE_SWCLK, TIME_INFINITE);

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
  DebugTrace("Enter SWDIO mode");
  chThdSleepMilliseconds(100); // wait for dma buffer to be flushed to sdcard
  palSetLineMode(LINE_SWCLK, PAL_MODE_ALTERNATE(AF_LINE_SWCLK));
  chSysHalt("swdio will take over");
 
  return true;
}

