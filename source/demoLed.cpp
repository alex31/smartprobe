#include <ch.h>
#include <hal.h>
#include "demoLed.hpp"
#include "blackBoard.hpp"
#include "threadAndEventDecl.hpp"
#include "util.hpp"
#include "led2812.hpp"

namespace {
  AirSpeed relAirSpeed{};
};

bool DemoLed::initInThreadContext()
{

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
  const auto& tas = relAirSpeed.tas;
  const auto& alpha = relAirSpeed.alpha;
  //  const auto& beta = relAirSpeed.beta;

  hsv.h = std::min(360.0f, alpha * 10);
  hsv.s = 1.0f;
  hsv.v = std::min(1.0f, tas / 20);

  fl.setDirectHsv(hsv);
  
  return true;
}



