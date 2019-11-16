#pragma once

#include "workerClass.hpp"

class DifferentialPressure : public WorkerThread<512U> {
public:
  DifferentialPressure(const tprio_t m_prio) : WorkerThread<512U>(m_prio) {};
  ~DifferentialPressure () {};
private:
  bool init(void) final;
  bool loop(void) final;
};


void testToutCa (void);
