#include <ch.h>
#include <hal.h>
#include "blinker.hpp"
#include "stdutil.h"	


bool Blinker::init()
{
  return true;
}


bool Blinker::loop()
{
  palSetLine(LINE_LED_GREEN);
  chThdSleepMilliseconds(100);
  palClearLine(LINE_LED_GREEN);
  return true;
}

  


