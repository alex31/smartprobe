/** @file
 *  @brief PPRZLink message header built from ../../message_definitions/v1.0/messages.xml
 *  @see http://paparazziuav.org
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "pprzlink/pprzlink_standalone.h"
#include "pprzlink/pprzlink_utils.h"

#define PPRZ_MSG_ID_AEROPROBE 179

static inline void pprzlink_msg_send_AEROPROBE(struct pprzlink_device_tx *dev, uint8_t sender_id, uint8_t receiver_id, uint32_t *_counter, int16_t *_velocity, int16_t *_a_attack, int16_t *_a_sideslip, int32_t *_altitude, int32_t *_dynamic_p, int32_t *_static_p, uint8_t *_checksum) {
  uint8_t size = 4+4+4+2+2+2+4+4+4+1;
  if (dev->check_space(size)) {
    dev->put_char(PPRZLINK_STX);
    dev->put_char(size);
    dev->ck_a = size;
    dev->ck_b = size;
    uint8_t head[4];
    head[0] = sender_id;
    head[1] = receiver_id;
    head[2] = (1 & 0x0F); // class id but no component id for now
    head[3] = PPRZ_MSG_ID_AEROPROBE;
    pprzlink_put_bytes(dev, head, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _counter, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _velocity, 2);
    pprzlink_put_bytes(dev, (uint8_t *) _a_attack, 2);
    pprzlink_put_bytes(dev, (uint8_t *) _a_sideslip, 2);
    pprzlink_put_bytes(dev, (uint8_t *) _altitude, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _dynamic_p, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _static_p, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _checksum, 1);
    dev->put_char(dev->ck_a);
    dev->put_char(dev->ck_b);
    if (dev->send_message != NULL) {
      dev->send_message();
    }
  }
}


static inline uint32_t pprzlink_get_AEROPROBE_counter(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint32_t(_payload, 4);
}


static inline int16_t pprzlink_get_AEROPROBE_velocity(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int16_t(_payload, 8);
}


static inline int16_t pprzlink_get_AEROPROBE_a_attack(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int16_t(_payload, 10);
}


static inline int16_t pprzlink_get_AEROPROBE_a_sideslip(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int16_t(_payload, 12);
}


static inline int32_t pprzlink_get_AEROPROBE_altitude(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 14);
}


static inline int32_t pprzlink_get_AEROPROBE_dynamic_p(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 18);
}


static inline int32_t pprzlink_get_AEROPROBE_static_p(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 22);
}


static inline uint8_t pprzlink_get_AEROPROBE_checksum(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint8_t(_payload, 26);
}



#ifdef __cplusplus
} // extern "C"
#endif


