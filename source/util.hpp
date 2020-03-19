#pragma once


#define PERIOD(k) (CH_CFG_ST_FREQUENCY / CONF(k))

template <typename T>
constexpr T rad2deg(const T rad) {return rad * static_cast<T>(180.0) / static_cast<T>(M_PI);}
template <typename T>
constexpr T deg2rad(const T deg) {return deg * static_cast<T>(M_PI) / static_cast<T>(180.0);}
