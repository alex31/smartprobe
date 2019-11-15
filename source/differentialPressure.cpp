#include "differentialPressure.hpp"
#include "stdutil.h"	

namespace {

}



bool DifferentialPressure::init()
{
  return true;
}

bool DifferentialPressure::loop()
{
  return true;
}


void testToutCa (void) {
  DifferentialPressure dp(NORMALPRIO);
  if (dp.run() != true) {
    DebugTrace("ça merde");
  }
}
  
