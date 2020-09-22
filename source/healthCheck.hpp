#pragma once
#include "workerClass.hpp"
#include "adc.hpp"
#include "imu.hpp"
#include "sdcard.hpp"
#include "hardwareConf.hpp"


#if CH_DBG_THREADS_PROFILING
#define MAX_CPU_INFO_ENTRIES 30

typedef struct _ThreadCpuInfo {
  float    ticks[MAX_CPU_INFO_ENTRIES];
  float    cpu[MAX_CPU_INFO_ENTRIES];
  float    totalTicks;
  _ThreadCpuInfo () {
    for (auto i=0; i< MAX_CPU_INFO_ENTRIES; i++) {
      ticks[i] = 0.0f;
      cpu[i] = -1.0f;
    }
    totalTicks = 0.0f;
  }
} ThreadCpuInfo ;


void stampThreadCpuInfo (ThreadCpuInfo *ti);
float stampThreadGetCpuPercent (const ThreadCpuInfo *ti, const uint32_t idx);
#endif


class HealthCheck : public WorkerThread<TH_HEALTHCHECK::threadStackSize,
					   HealthCheck> {
public:
  HealthCheck(const tprio_t m_prio) :
    WorkerThread<TH_HEALTHCHECK::threadStackSize,
		 HealthCheck>("showBlackboard", m_prio) {};
private:
  friend WorkerThread<TH_HEALTHCHECK::threadStackSize, HealthCheck>;
  bool init(void) final;
  bool initInThreadContext(void) final;
  bool loop(void) final;
  void logThreadInfos(void);
  void logInternalVoltageInfo(void);
};




