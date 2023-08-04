#include <ch.h>
#include <hal.h>
#include "demoLed.hpp"
#include "blackBoard.hpp"
#include "threadAndEventDecl.hpp"
#include "util.hpp"
#include "led2812.hpp"

namespace {
  AirSpeed relAirSpeed{}, relAirSpeedSum{};
  virtual_timer_t            vt;
  constexpr HSV hsvzero {.h = 0, .s = 0, .v = 0};
  bool vtArmed = false;

};

bool DemoLed::initInThreadContext()
{
  chVTObjectInit(&vt);

  return true;
}

bool DemoLed::init()
{
  return true;
}


bool DemoLed::loop()
{
  HSV hsv;
  relwind.blackBoard.read(relAirSpeed);
  //  const auto& beta = relAirSpeed.beta;

  //  DebugTrace("cur = %.2f sum = %.2f", (double) relAirSpeed.tas, (double) relAirSpeedSum.tas);
  if ((relAirSpeed.tas > 5.0f) && (relAirSpeed.alpha > -20.0f) && (relAirSpeed.alpha < 20.0f)) {
    chVTReset(&vt);
    relAirSpeedSum =  (relAirSpeedSum * 0.99) + (relAirSpeed * 0.01);
    float alpha = fmodf(360.0, relAirSpeedSum.alpha+30); // [ -20 ... 20 ]
    hsv.h = std::min(1.0f, alpha / 50.0f);
    hsv.s = 1.0f;
    hsv.v = std::max(0.0f, std::min(1.0f, (relAirSpeedSum.tas) / 20.0f));
    
    fl.setDirectHsv(hsv);
    vtArmed = false;
    //  DebugTrace("h=%.2f v=%.2f");
  } else {
    // when the is no wind for one second, light off the led
    if (not vtArmed) {
      vtArmed = true;
      chVTSet(&vt, TIME_S2I(1),
	      [](ch_virtual_timer *, void *) {
		fl.setDirectHsv(hsvzero);
	      }
	      , NULL);
    }
  }
  return true;
}



