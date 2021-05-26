#include <ch.h>
#include <hal.h>
#include "transmitPprzlink.hpp"
#include "blackBoard.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "pprzlink/pprzlink_aeroprobe.h"
#include "pprzlink/pprzlink_smartprobe.h"
#include "util.hpp"

namespace {
  BarometerData    baroData{};
  DiffPressureData diffPressData{};
  ImuData  imuData{};
  Vec3f    attitude{};
  AirSpeed relAirSpeed{}, relAirSpeedSum{};
  size_t   sumCount;
  event_listener_t diffPressEvent;
  sysinterval_t delay;
  systime_t now;
  systime_t then;
  struct pprzlink_device_tx dev_tx;
  uint32_t freeCounter = 0U;
  PprzMsgType msgType;
};



bool TransmitPprzlink::initInThreadContext()
{
  dp.blackBoard.registerEvt(&diffPressEvent, PDIF_EVT);
  now = chVTGetSystemTimeX();
  then = chTimeAddX(now, delay);

  return true;
}

bool TransmitPprzlink::init()
{
  delay = PERIOD("thread.frequency.transmit_uart");
  msgType = static_cast<PprzMsgType>(CONF("pprz.msg"));
  sumCount = 0;
  dev_tx = pprzlink_device_tx_init(
				[] ([[maybe_unused]] uint8_t n) -> int  {return true;},
                                [] (uint8_t c)                  -> void {sdPut(&ExtSD, c);},
				nullptr);
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


bool TransmitPprzlink::loop()
{
  chEvtWaitOne(PDIF_EVT);

  baro.blackBoard.read(baroData);
  dp.blackBoard.read(diffPressData);
  imu.blackBoard.read(imuData);
  ahrs.blackBoard.read(attitude);
  relwind.blackBoard.read(relAirSpeed);

  if (chVTIsSystemTimeWithin(now, then)) {
    sumCount++;
    relAirSpeedSum += relAirSpeed;
  } else {
    now = chVTGetSystemTimeX();
    then = chTimeAddX(now, delay);
    relAirSpeedSum /= sumCount;
    sumCount = 0;
    
    switch (msgType) {
    case AEROPROBE : aeroprobeLoop(); break;
    case SMARTPROBE : smartprobeLoop(); break;
    default:
      chSysHalt("msgType not known");
    }
    relAirSpeedSum.clear();
  }
  return true;
}

void TransmitPprzlink::aeroprobeLoop()
{
  int16_t velocity =   relAirSpeedSum.velocity * 100U; 	 	
  int16_t a_attack =   relAirSpeedSum.alpha * 100; 	 		
  int16_t a_sideslip = relAirSpeedSum.beta * 100U; 	 		
  int32_t altitude =   0;	 	
  int32_t dynamic_p =  diffPressData[0].pressure * 100;	 	
  int32_t static_p =   baroData.pressure * 100U; // pascal	 	    	
  uint8_t checksum =   0;
  freeCounter++;
  
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



/*
<field name="accel" type="float[3]" unit="m/s^2">internal accelerometer</field>
     gyro type="float[3]" unit="rad/s">internal gyrometer</field>
     attitude type="float[4]">attitude in normalized quaternion or euler in radians 
                               (when last field is lower than -2)</field>
     tas type="float" unit="m/s">true airspeed</field>
     eas type="float" unit="m/s">equivalent airspeed</field>
     alpha type="float" unit="rad">angle of attack</field>
     beta type="float" unit="rad">andgle of sideslip</field>
     pressure type="float" unit="Pa">static absolute pressure</field>
     temperature type="float" unit="Celcius">static temperature</field>
     rho type="float" unit="kg/m^3">air density</field>
     p_diff_C_ra type="float" unit="Pa">raw differential pressure for central sensor</field>
     t_diff_C_ra type="float" unit="Celcius">raw temperature for central sensor</field>
     p_diff_H_ra type="float" unit="Pa">raw differential pressure for horizontal sensor</field>
     t_diff_H_ra type="float" unit="Celcius">raw temperature for horizontal sensor</field>
     p_diff_V_ra type="float" unit="Pa">raw differential pressure for vertical sensor</field>
     t_diff_V_ra type="float" unit="Celcius">raw temperature for vertical sensor</field>	 
 */
struct SmartprobePayload {
  float accel[3];
  float gyro[3];
  float attitude[4]; // euler or quaternion (quaternion : to be implemented)
  struct {
    float tas;
    float eas;
    float alpha;
    float beta;
    float pressure;
    float temperature;
    float rho;
  } airSpeed;
  struct {
    float pressure;
    float temperature;
  } diffPress[3]; // C,H,V
};


void TransmitPprzlink::smartprobeLoop()
{
  SmartprobePayload payload = {
			       .accel = {imuData.acc.v[0], imuData.acc.v[1], imuData.acc.v[2]},
			       .gyro = {imuData.gyro.v[0], imuData.gyro.v[1], imuData.gyro.v[2]},
			       .attitude = {attitude.v[0], attitude.v[1], attitude.v[2], -1000.0f},
			       .airSpeed =  {
					     .tas = relAirSpeedSum.velocity,
					     .eas = 1000.0f, // todo : calculate eas
					     .alpha =  relAirSpeedSum.alpha,
					     .beta = relAirSpeedSum.beta,
					     .pressure = baroData.pressure * 100,
					     .temperature = baroData.temp,
					     .rho = baroData.rho
					     },
			       .diffPress = {
					     {.pressure = diffPressData[0].pressure * 100,
					      .temperature = diffPressData[0].temp
					     },
					     {.pressure = diffPressData[1].pressure * 100,
					      .temperature = diffPressData[1].temp
					     },
					     {.pressure = diffPressData[2].pressure * 100,
					      .temperature = diffPressData[2].temp
					     }
					     }
  };
  pprzlink_msg_send_SMARTPROBE(&dev_tx,
			       0U, /* sender_id */
			       0U, /* receiver_id */
			       payload.accel, payload.gyro, payload.attitude,
			       &payload.airSpeed.tas,
			       &payload.airSpeed.eas,
			       &payload.airSpeed.alpha,
			       &payload.airSpeed.beta,
			       &payload.airSpeed.pressure,
			       &payload.airSpeed.temperature,
			       &payload.airSpeed.rho,
			       &payload.diffPress[0].pressure,
			       &payload.diffPress[0].temperature,
			       &payload.diffPress[1].pressure,
			       &payload.diffPress[1].temperature,
			       &payload.diffPress[2].pressure,
			       &payload.diffPress[2].temperature);
			       
}
  


