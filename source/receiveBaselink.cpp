#include <ch.h>
#include <hal.h>
#include "receiveBaselink.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "sdcard.hpp"
#include "util.hpp"


bool ReceiveBaselink::init()
{
  int count =0;
  while (ExtSD.state != SD_READY and ++count < 1000)
    chThdSleepMilliseconds(1); // wait for emitter to configure uart
  // after one second, nobody will do it for us; we start SD ourself
  if (ExtSD.state != SD_READY) {
    static const SerialConfig serialConfig =  {
					       static_cast<uint32_t>(CONF("uart.baud")),
					       0,
					       USART_CR2_STOP1_BITS | USART_CR2_LINEN,
					       0
    };
    sdStart(&ExtSD, &serialConfig);
  }
  return true;
}

  

