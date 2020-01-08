#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"

class DynSwdio : public WorkerThread<TH_DYNSWDIO::threadStackSize, DynSwdio> {
public:
  DynSwdio(const tprio_t m_prio) :
    WorkerThread<TH_DYNSWDIO::threadStackSize, DynSwdio>("dynSwdio", m_prio) {};
private:
  friend WorkerThread<TH_DYNSWDIO::threadStackSize, DynSwdio>;
  bool init(void) final;
  bool loop(void) final;
};




