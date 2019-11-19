#pragma once
#include <ch.h>
#include <hal.h>


template<size_t WSS, typename T>
class WorkerThread
{
public:
  WorkerThread(const char *m_name, const tprio_t m_prio) : name(m_name), prio(m_prio) {};
  bool run();
  WorkerThread& terminate();
  WorkerThread& join();
  ~WorkerThread() {terminate().join();};
  
protected:
  static void threadFunc(void *o);
private:
  
  virtual bool init(void) = 0;
  virtual bool loop(void) = 0;

  const char *name;
  const tprio_t prio;

  static thread_t *handle;

  static THD_WORKING_AREA(ws, WSS);
};


  
template<size_t WSS, typename T>
bool WorkerThread<WSS, T>::run(void)
{
  // this will force that only one process can be run
  // in the same time
  if (handle != nullptr)
    return false;
    
  if (init() == false)
    return false;
    
  handle = chThdCreateStatic(ws, sizeof(ws), prio, threadFunc, this);
  chRegSetThreadNameX (handle, name);
  return true;
}

template<size_t WSS, typename T>
WorkerThread<WSS, T>& WorkerThread<WSS, T>::join(void)
{
  // this will force that only one process can be run
  // in the same time
  if (handle != nullptr) {
    chThdWait(handle);
    handle = nullptr;
  }
  return *this;
}

template<size_t WSS, typename T>
WorkerThread<WSS, T>& WorkerThread<WSS, T>::terminate(void)
{
  // this will force that only one process can be run
  // in the same time
  if (handle != nullptr) {
    chThdTerminate(handle);
  }
  return *this;
  
}


template<size_t WSS, typename T>
void WorkerThread<WSS, T>::threadFunc(void *o) {
  while (!chThdShouldTerminateX()) {
    if (((WorkerThread<WSS, T>*) o)->loop() == false)
      break;
  }
  chThdExit(0);
}

template<size_t WSS, typename T>
ALIGNED_VAR(32) stkalign_t WorkerThread<WSS, T>::ws[THD_WORKING_AREA_SIZE(WSS) / sizeof(stkalign_t)];

template<size_t WSS, typename T>
thread_t *WorkerThread<WSS, T>::handle = nullptr;
