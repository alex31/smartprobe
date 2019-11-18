#pragma once
#include "workerClass.hpp"


static constexpr size_t threadStackSize = 320U;

class Blinker : public WorkerThread<threadStackSize> {
public:
  Blinker(const tprio_t m_prio) : WorkerThread<threadStackSize>("blinker", m_prio) {};
private:
  bool init(void) final;
  bool loop(void) final;
};




