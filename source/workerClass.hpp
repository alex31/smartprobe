#pragma once
#include <ch.h>
#include <hal.h>


template<size_t WSS>
class WorkerThread
{
public:
  WorkerThread(const tprio_t m_prio) : prio(m_prio) {};
  bool run(void);
protected:
  static void threadFunc(void *o);
private:
  virtual bool init(void) = 0;
  virtual bool loop(void) = 0;
    

  thread_t *handle = nullptr;
  tprio_t prio;

  static THD_WORKING_AREA(ws, WSS);
};


  
template<size_t WSS>
bool WorkerThread<WSS>::run(void)
{
  if (handle == nullptr)
    return false;
    
  if (init() == false)
    return false;
    
  chThdCreateStatic(ws, WSS, prio, threadFunc, this);
  return true;
}

template<size_t WSS>
void WorkerThread<WSS>::threadFunc(void *o) {
  while (!chThdShouldTerminateX()) {
    ((WorkerThread<WSS>*) o)->loop();
  }
}

template<size_t WSS>
ALIGNED_VAR(32) stkalign_t WorkerThread<WSS>::ws[THD_WORKING_AREA_SIZE(WSS) / sizeof(stkalign_t)];
