#include <ch.h>
#include <hal.h>
#include "transmitBlackboard.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "pprzlink/pprzlink_smartprobe.h"

#define PERIOD(k) (CH_CFG_ST_FREQUENCY / CONF(k))


namespace {
  BarometerData    baroData{};
  DiffPressureData diffPressData{};
  ImuData  imuData{};
  Vec3f    attitude{},    attitudeSum{};
  AirSpeed relAirSpeed{}, relAirSpeedSum{};
  size_t   sumCount;
  event_listener_t diffPressEvent;
  sysinterval_t delay;
  systime_t now;
  systime_t then;
  struct pprzlink_device_tx dev_tx;
  uint32_t freeCounter = 0U;
};



bool TransmitBlackboard::initInThreadContext()
{
  dp.blackBoard.registerEvt(&diffPressEvent, PDIF_EVT);
  now = chVTGetSystemTimeX();
  then = chTimeAddX(now, delay);

  return true;
}

bool TransmitBlackboard::init()
{
  delay = PERIOD("thread.frequency.transmit_uart");
  sumCount = 0;
  dev_tx = pprzlink_device_tx_init(
				[] ([[maybe_unused]] uint8_t n) -> int  {return true;},
                                [] (uint8_t c)                  -> void {sdPut(&ExtSD, c);},
				nullptr);
  if (ExtSD.state == SD_UNINIT) {
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


bool TransmitBlackboard::loop()
{
  const eventmask_t event = chEvtWaitOneTimeout(PDIF_EVT, TIME_MS2I(1000));

  if (not event)
    return true;
  
  baro.blackBoard.read(baroData);
  dp.blackBoard.read(diffPressData);
  imu.blackBoard.read(imuData);
  ahrs.blackBoard.read(attitude);
  relwind.blackBoard.read(relAirSpeed);

  if (chVTIsSystemTimeWithin(now, then)) {
    sumCount++;
    attitudeSum = vec3fAdd(&attitudeSum, &attitude);
    relAirSpeedSum += relAirSpeed;
  } else {
    now = chVTGetSystemTimeX();
    then = chTimeAddX(now, delay);
    attitudeSum = vec3fDiv(&attitudeSum, sumCount);
    relAirSpeedSum /= sumCount;
    sumCount = 0;

#warning "verifier que les pressions soient bien en hectopascal"
    
    int16_t velocity =   relAirSpeedSum.velocity * 100U; 	 	
    int16_t a_attack =   relAirSpeedSum.alpha * 100; 	 		
    int16_t a_sideslip = relAirSpeedSum.beta * 100U; 	 		
    int32_t altitude =   0;	 	
    int32_t dynamic_p =  diffPressData[0].pressure * 100;	 	
    int32_t static_p =   baroData.pressure * 100U;	 	    	
    uint8_t checksum =   0;
    
    pprzlink_msg_send_AEROPROBE(&dev_tx,
				0U, /* sender_id */
				0U, /* receiver_id */
				&freeCounter,
				&velocity,
				&a_attack,
				&a_sideslip,
				&altitude,
				&dynamic_p,
				&static_p,
				&checksum
				);
				
  }

  freeCounter++;
  
  return true;
}
  


