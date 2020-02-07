#include <ch.h>
#include <hal.h>
#include "receiveBlackboard.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "sdcard.hpp"
#include "pprzlink/pprzlink_smartprobe.h"


#define PERIOD(k) (CH_CFG_ST_FREQUENCY / CONF(k))


namespace {
  struct pprzlink_device_rx dev_rx;
  uint8_t rx_buffer[255];
  void new_message_cb(uint8_t sender_id, uint8_t receiver_id, uint8_t class_id, uint8_t message_id, uint8_t *buf);
  uint8_t nextChar=0U;
};



bool ReceiveBlackboard::initInThreadContext()
{


  return true;
}

bool ReceiveBlackboard::init()
{
  dev_rx = pprzlink_device_rx_init(
				   [] (void) -> int { // char_available
				     return 1;
				   },
				   [] (void) -> uint8_t  {return nextChar;}, // get_char
				   rx_buffer
				   );

  while (ExtSD.state == SD_UNINIT)
    chThdSleepMilliseconds(1); // wait for emitter to configure uart
  return true;
}

  

bool ReceiveBlackboard::loop()
{
  /*
    ° we won't (ever !) do polling
      sdGet is blocking and we call  pprzlink_check_and_parse when one char is avalaible
    ° perhaps there is a way to avoid module scoped global var nextChar ?
   */
  
  nextChar = sdGet(&ExtSD);
  pprzlink_check_and_parse(&dev_rx, &new_message_cb);
  return true;
}
  

namespace {
  
  void new_message_cb([[maybe_unused]] uint8_t sender_id, [[maybe_unused]] uint8_t receiver_id,
		   [[maybe_unused]] uint8_t class_id, uint8_t message_id, uint8_t *buf) {
  // check message/class IDs to before extracting data from the messages
  if (message_id == PPRZ_MSG_ID_GPS) {
    // get data from GPS
    int32_t east = pprzlink_get_GPS_utm_east(buf);
    int32_t north = pprzlink_get_GPS_utm_north(buf);
    SdCard::logSyslog(Severity::Info, "DEBUG> gps east = %ld north = %ld", east, north);
  }
 
}


}
