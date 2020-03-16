#pragma once
#include "receiveBaselink.hpp"
#include "nmeaFrame.h"


class ReceiveNmealink : public ReceiveBaselink {
                                              
public:
  ReceiveNmealink(const tprio_t m_prio) :
    ReceiveBaselink(m_prio) {};
private:
  friend WorkerThread<TH_RECEIVEBASELINK::threadStackSize, ReceiveNmealink>;
  bool init(void) final;
  bool loop(void) final;
};

