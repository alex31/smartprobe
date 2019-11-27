#pragma once
#include "workerClass.hpp"

namespace TH_BLINKER {
static constexpr size_t threadStackSize = 320U;
}

class Blinker : public WorkerThread<TH_BLINKER::threadStackSize, Blinker> {
public:
  Blinker(const tprio_t m_prio) :
    WorkerThread<TH_BLINKER::threadStackSize, Blinker>("blinker", m_prio) {};
private:
  friend WorkerThread<TH_BLINKER::threadStackSize, Blinker>;
  bool init(void) final;
  bool loop(void) final;
};




