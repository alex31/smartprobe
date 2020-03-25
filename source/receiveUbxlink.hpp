#pragma once
#include "receiveBaselink.hpp"
#include "ubxNavPvt.hpp"


class ReceiveUbxlink : public ReceiveBaselink {
  
  struct UbxNavPvt_valid {
  uint8_t validDate:1;
  uint8_t validTime:1;
  uint8_t fullyResolved:1;
  uint8_t validMag:1;
  uint8_t dummy:4;
};

struct UbxNavPvt_flag {
  uint8_t gnssFixOK:1;
  uint8_t diffSoln:1;
  uint8_t psmState:3;
  uint8_t headVehValid:1;
  uint8_t carrSoln:2;
};

struct UbxNavPvt_flag2 {
  uint8_t dummy:5;
  uint8_t confirmedAvai:1;
  uint8_t confirmedDate:1;
  uint8_t confirmedTime:1;
};


struct UbxNavPvt {
  uint32_t  iTOW;
  uint16_t  year;
  uint8_t   month;
  uint8_t   day;
  uint8_t   hour;
  uint8_t   min;
  uint8_t   sec;
  UbxNavPvt_valid valid;
  uint32_t  tAcc;
  int32_t   nano;
  uint8_t   fixType;
  UbxNavPvt_flag  flags;
  UbxNavPvt_flag2 flags2;
  uint8_t   numSv;
  int32_t   lon;
  int32_t   lat;
  int32_t   height;
  int32_t   hMSL;
  uint32_t  hAcc;
  uint32_t  vAcc;
  int32_t   velN;
  int32_t   velE;
  int32_t   velD;
  int32_t   gSpeed;
  int32_t   headMot;
  uint32_t  sAcc ;
  uint32_t  headAcc ;
  uint16_t  pDOP ;
  uint8_t   reserved1[6];
  int32_t   headVeh;
  int16_t   magDec;
  uint16_t  magAcc;
} __attribute__((packed));

  static_assert(sizeof(UbxNavPvt) == 92);
  

public:
  ReceiveUbxlink(const tprio_t m_prio) :
    ReceiveBaselink(m_prio) {};
private:
  static constexpr std::array<TopicLen_t, 1> topics = {{  {0x0107, sizeof(UbxNavPvt)} }};
  friend WorkerThread<TH_RECEIVEBASELINK::threadStackSize, ReceiveUbxlink>;
  bool init(void) final;
  bool loop(void) final;
  void propagateNavPvt(const UbxNavPvt& navPvt);
  UbxNavPvtSerialDecoder<98, topics.size()> *ubx{};
};

