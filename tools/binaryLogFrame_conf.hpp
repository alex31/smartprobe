#pragma once

#define PT1 float
#define PM1 pressure
#define SC1 0
#define DC1 "pascal"

#define PT2 uint16_t
#define PM2 temperature
#define SC2 0.1
#define DC2 "degree_celcius"

#define PT3 uint8_t
#define PM3 zone
#define SC3 0
#define DC3 "umt_zone"

#define DECLS \
    DECL(1);  \
    DECL(2);  \
    DECL(3);  

#define DESCS \
  DESC(1),    \
  DESC(2),    \
  DESC(3)  

























