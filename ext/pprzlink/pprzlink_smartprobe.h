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

#define PPRZ_MSG_ID_SMARTPROBE 60

static inline void pprzlink_msg_send_SMARTPROBE(struct pprzlink_device_tx *dev, uint8_t sender_id, uint8_t receiver_id, float *_accel, float *_gyro, float *_attitude, float *_tas, float *_eas, float *_alpha, float *_beta, float *_pressure, float *_temperature, float *_rho, float *_p_diff_C_raw, float *_t_diff_C_raw, float *_p_diff_H_raw, float *_t_diff_H_raw, float *_p_diff_V_raw, float *_t_diff_V_raw) {
  uint8_t size = 4+4+3*4+3*4+4*4+4+4+4+4+4+4+4+4+4+4+4+4+4;
  if (dev->check_space(size)) {
    dev->put_char(PPRZLINK_STX);
    dev->put_char(size);
    dev->ck_a = size;
    dev->ck_b = size;
    uint8_t head[4];
    head[0] = sender_id;
    head[1] = receiver_id;
    head[2] = (2 & 0x0F); // class id but no component id for now
    head[3] = PPRZ_MSG_ID_SMARTPROBE;
    pprzlink_put_bytes(dev, head, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _accel, 3*4);
    pprzlink_put_bytes(dev, (uint8_t *) _gyro, 3*4);
    pprzlink_put_bytes(dev, (uint8_t *) _attitude, 4*4);
    pprzlink_put_bytes(dev, (uint8_t *) _tas, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _eas, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _alpha, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _beta, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _pressure, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _temperature, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _rho, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _p_diff_C_raw, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _t_diff_C_raw, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _p_diff_H_raw, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _t_diff_H_raw, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _p_diff_V_raw, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _t_diff_V_raw, 4);
    dev->put_char(dev->ck_a);
    dev->put_char(dev->ck_b);
    if (dev->send_message != NULL) {
      dev->send_message();
    }
  }
}

static inline uint8_t pprzlink_get_SMARTPROBE_accel_length(void* _payload __attribute__ ((unused))) {
  return 3;
}

static inline float * pprzlink_get_SMARTPROBE_accel(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float_array(_payload, 4);
}

static inline uint8_t pprzlink_get_SMARTPROBE_gyro_length(void* _payload __attribute__ ((unused))) {
  return 3;
}

static inline float * pprzlink_get_SMARTPROBE_gyro(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float_array(_payload, 16);
}

static inline uint8_t pprzlink_get_SMARTPROBE_attitude_length(void* _payload __attribute__ ((unused))) {
  return 4;
}

static inline float * pprzlink_get_SMARTPROBE_attitude(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float_array(_payload, 28);
}


static inline float pprzlink_get_SMARTPROBE_tas(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 44);
}


static inline float pprzlink_get_SMARTPROBE_eas(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 48);
}


static inline float pprzlink_get_SMARTPROBE_alpha(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 52);
}


static inline float pprzlink_get_SMARTPROBE_beta(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 56);
}


static inline float pprzlink_get_SMARTPROBE_pressure(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 60);
}


static inline float pprzlink_get_SMARTPROBE_temperature(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 64);
}


static inline float pprzlink_get_SMARTPROBE_rho(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 68);
}


static inline float pprzlink_get_SMARTPROBE_p_diff_C_raw(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 72);
}


static inline float pprzlink_get_SMARTPROBE_t_diff_C_raw(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 76);
}


static inline float pprzlink_get_SMARTPROBE_p_diff_H_raw(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 80);
}


static inline float pprzlink_get_SMARTPROBE_t_diff_H_raw(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 84);
}


static inline float pprzlink_get_SMARTPROBE_p_diff_V_raw(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 88);
}


static inline float pprzlink_get_SMARTPROBE_t_diff_V_raw(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_float(_payload, 92);
}



#ifdef __cplusplus
} // extern "C"
#endif


