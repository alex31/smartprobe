#include <ch.h>
#include <hal.h>
#include "sdcard.hpp"
#include "adc.hpp"
#include "healthCheck.hpp"
#include "stdutil.h"
#include "printf.h"
#include "threadAndEventDecl.hpp"
#include "ttyConsole.hpp"
#include "util.hpp"


namespace {

};

bool HealthCheck::initInThreadContext()
{
  return true;
}

bool HealthCheck::init()
{
  return true;
}


bool HealthCheck::loop()
{
  logThreadInfos();
  logInternalVoltageInfo();
  return true;
}

void HealthCheck::logThreadInfos(void)
{
#if  CH_DBG_THREADS_PROFILING
  static const char *states[] = {CH_STATE_NAMES};
  Thread *tp = chRegFirstThread();
  float totalTicks=0;
  float idleTicks=0;
  
  static ThreadCpuInfo threadCpuInfo ;
  
  stampThreadCpuInfo (&threadCpuInfo);
  
  uint32_t idx=0;
  do {
       totalTicks+= (float) tp->time;
    if (strcmp (chRegGetThreadName(tp), "idle") == 0) {
      idleTicks =  (float) tp->time;
    } else if (get_stack_free(tp) < 300) {
      SdCard::logSyslog(Severity::Info, "    addr    stack  frestk prio refs  "
			"state        time \t percent        name\r\n");
      SdCard::logSyslog(Severity::Warning, "%.8lx %.8lx %6lu %4lu %4lu %9s %9lu   %.1f    \t%s\r\n",
			(uint32_t)tp, (uint32_t)tp->ctx.sp,
			get_stack_free(tp),
			(uint32_t)tp->hdr.pqueue.prio, (uint32_t)(tp->refs - 1),
			states[tp->state], (uint32_t)tp->time, 
			(double) stampThreadGetCpuPercent (&threadCpuInfo, idx),
			chRegGetThreadName(tp));
    }
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while (tp != NULL);
  
  const float idlePercent = (idleTicks*100.f)/totalTicks;
  const float cpuPercent = 100.f - idlePercent;
  if (cpuPercent > 10) {
    SdCard::logSyslog(cpuPercent > 50 ? Severity::Warning : Severity::Info,
		      "cpu load = %.2f%%\r\n", (double) cpuPercent);
  }
#endif
}

void HealthCheck::logInternalVoltageInfo(void)
{
  const float averageSensorsVdd = adc.getSensorsVoltage();
  const float averageCoreVdd = adc.getCoreVdd();
  const float coreTemp = adc.getCoreTemp();

  if (coreTemp > 50)
    SdCard::logSyslog(Severity::Warning, "Core temp = %.1f°", (double) coreTemp);

  if (averageSensorsVdd != std::clamp(averageSensorsVdd,
				      VOLTAGE_3_3_MINIMUM,  VOLTAGE_3_3_MAXIMUM)) {
    SdCard::logSyslog(Severity::Fatal, "Sensors internal Voltage = %.2f is "
		      "not in range [%.2f .. %.2f]", 
		      double(averageSensorsVdd), double(VOLTAGE_3_3_MINIMUM),
		      double(VOLTAGE_3_3_MAXIMUM));
  }


  if (averageCoreVdd != std::clamp(averageCoreVdd,
				   VOLTAGE_3_3_MINIMUM,  VOLTAGE_3_3_MAXIMUM)) {
    SdCard::logSyslog(Severity::Fatal, "Core internal Voltage = %.2f is "
		      "not in range [%.2f .. %.2f]", 
		      double(averageCoreVdd), double(VOLTAGE_3_3_MINIMUM),
		      double(VOLTAGE_3_3_MAXIMUM));
  }

      
}


#if  CH_DBG_THREADS_PROFILING
void stampThreadCpuInfo (ThreadCpuInfo *ti)
{
  const Thread *tp =  chRegFirstThread();
  uint32_t idx=0;
  
  float totalTicks =0;
  do {
    totalTicks+= (float) tp->time;
    ti->cpu[idx] = (float) tp->time - ti->ticks[idx];;
    ti->ticks[idx] = (float) tp->time;
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while ((tp != NULL) && (idx < MAX_CPU_INFO_ENTRIES));
  
  const float diffTotal = totalTicks- ti->totalTicks;
  ti->totalTicks = totalTicks;
  
  tp =  chRegFirstThread();
  idx=0;
  do {
    ti->cpu[idx] =  (ti->cpu[idx]*100.f)/diffTotal;
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while ((tp != NULL) && (idx < MAX_CPU_INFO_ENTRIES));
}


float stampThreadGetCpuPercent (const ThreadCpuInfo *ti, const uint32_t idx)
{
  if (idx >= MAX_CPU_INFO_ENTRIES) 
    return -1.f;

  return ti->cpu[idx];
}
#endif
