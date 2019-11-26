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
  palToggleLine(LINE_LED1);
  return true;
}

  


