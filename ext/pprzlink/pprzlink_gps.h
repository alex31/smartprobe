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

#define PPRZ_MSG_ID_GPS 8

static inline void pprzlink_msg_send_GPS(struct pprzlink_device_tx *dev, uint8_t sender_id, uint8_t receiver_id, uint8_t *_mode, int32_t *_utm_east, int32_t *_utm_north, int16_t *_course, int32_t *_alt, uint16_t *_speed, int16_t *_climb, uint16_t *_week, uint32_t *_itow, uint8_t *_utm_zone, uint8_t *_gps_nb_err) {
  uint8_t size = 4+4+1+4+4+2+4+2+2+2+4+1+1;
  if (dev->check_space(size)) {
    dev->put_char(PPRZLINK_STX);
    dev->put_char(size);
    dev->ck_a = size;
    dev->ck_b = size;
    uint8_t head[4];
    head[0] = sender_id;
    head[1] = receiver_id;
    head[2] = (1 & 0x0F); // class id but no component id for now
    head[3] = PPRZ_MSG_ID_GPS;
    pprzlink_put_bytes(dev, head, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _mode, 1);
    pprzlink_put_bytes(dev, (uint8_t *) _utm_east, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _utm_north, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _course, 2);
    pprzlink_put_bytes(dev, (uint8_t *) _alt, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _speed, 2);
    pprzlink_put_bytes(dev, (uint8_t *) _climb, 2);
    pprzlink_put_bytes(dev, (uint8_t *) _week, 2);
    pprzlink_put_bytes(dev, (uint8_t *) _itow, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _utm_zone, 1);
    pprzlink_put_bytes(dev, (uint8_t *) _gps_nb_err, 1);
    dev->put_char(dev->ck_a);
    dev->put_char(dev->ck_b);
    if (dev->send_message != NULL) {
      dev->send_message();
    }
  }
}


static inline uint8_t pprzlink_get_GPS_mode(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint8_t(_payload, 4);
}


static inline int32_t pprzlink_get_GPS_utm_east(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 5);
}


static inline int32_t pprzlink_get_GPS_utm_north(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 9);
}


static inline int16_t pprzlink_get_GPS_course(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int16_t(_payload, 13);
}


static inline int32_t pprzlink_get_GPS_alt(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 15);
}


static inline uint16_t pprzlink_get_GPS_speed(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint16_t(_payload, 19);
}


static inline int16_t pprzlink_get_GPS_climb(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int16_t(_payload, 21);
}


static inline uint16_t pprzlink_get_GPS_week(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint16_t(_payload, 23);
}


static inline uint32_t pprzlink_get_GPS_itow(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint32_t(_payload, 25);
}


static inline uint8_t pprzlink_get_GPS_utm_zone(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint8_t(_payload, 29);
}


static inline uint8_t pprzlink_get_GPS_gps_nb_err(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint8_t(_payload, 30);
}


#define PPRZ_MSG_ID_GPS_INT 155

static inline void pprzlink_msg_send_GPS_INT(struct pprzlink_device_tx *dev, uint8_t sender_id, uint8_t receiver_id, int32_t *_ecef_x, int32_t *_ecef_y, int32_t *_ecef_z, int32_t *_lat, int32_t *_lon, int32_t *_alt, int32_t *_hmsl, int32_t *_ecef_xd, int32_t *_ecef_yd, int32_t *_ecef_zd, uint32_t *_pacc, uint32_t *_sacc, uint32_t *_tow, uint16_t *_pdop, uint8_t *_numsv, uint8_t *_fix, uint8_t *_comp_id) {
  uint8_t size = 4+4+4+4+4+4+4+4+4+4+4+4+4+4+4+2+1+1+1;
  if (dev->check_space(size)) {
    dev->put_char(PPRZLINK_STX);
    dev->put_char(size);
    dev->ck_a = size;
    dev->ck_b = size;
    uint8_t head[4];
    head[0] = sender_id;
    head[1] = receiver_id;
    head[2] = (1 & 0x0F); // class id but no component id for now
    head[3] = PPRZ_MSG_ID_GPS_INT;
    pprzlink_put_bytes(dev, head, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _ecef_x, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _ecef_y, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _ecef_z, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _lat, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _lon, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _alt, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _hmsl, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _ecef_xd, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _ecef_yd, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _ecef_zd, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _pacc, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _sacc, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _tow, 4);
    pprzlink_put_bytes(dev, (uint8_t *) _pdop, 2);
    pprzlink_put_bytes(dev, (uint8_t *) _numsv, 1);
    pprzlink_put_bytes(dev, (uint8_t *) _fix, 1);
    pprzlink_put_bytes(dev, (uint8_t *) _comp_id, 1);
    dev->put_char(dev->ck_a);
    dev->put_char(dev->ck_b);
    if (dev->send_message != NULL) {
      dev->send_message();
    }
  }
}


static inline int32_t pprzlink_get_GPS_INT_ecef_x(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 4);
}


static inline int32_t pprzlink_get_GPS_INT_ecef_y(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 8);
}


static inline int32_t pprzlink_get_GPS_INT_ecef_z(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 12);
}


static inline int32_t pprzlink_get_GPS_INT_lat(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 16);
}


static inline int32_t pprzlink_get_GPS_INT_lon(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 20);
}


static inline int32_t pprzlink_get_GPS_INT_alt(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 24);
}


static inline int32_t pprzlink_get_GPS_INT_hmsl(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 28);
}


static inline int32_t pprzlink_get_GPS_INT_ecef_xd(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 32);
}


static inline int32_t pprzlink_get_GPS_INT_ecef_yd(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 36);
}


static inline int32_t pprzlink_get_GPS_INT_ecef_zd(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_int32_t(_payload, 40);
}


static inline uint32_t pprzlink_get_GPS_INT_pacc(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint32_t(_payload, 44);
}


static inline uint32_t pprzlink_get_GPS_INT_sacc(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint32_t(_payload, 48);
}


static inline uint32_t pprzlink_get_GPS_INT_tow(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint32_t(_payload, 52);
}


static inline uint16_t pprzlink_get_GPS_INT_pdop(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint16_t(_payload, 56);
}


static inline uint8_t pprzlink_get_GPS_INT_numsv(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint8_t(_payload, 58);
}


static inline uint8_t pprzlink_get_GPS_INT_fix(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint8_t(_payload, 59);
}


static inline uint8_t pprzlink_get_GPS_INT_comp_id(uint8_t * _payload __attribute__((unused)))
{
  return _PPRZ_VAL_uint8_t(_payload, 60);
}



#ifdef __cplusplus
} // extern "C"
#endif


