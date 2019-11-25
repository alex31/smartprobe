#pragma once


/*
  template class to use a variable of type T as a blackboard :

  the blackboard include an event source, and a RW lock protection.
  the behavior can be modified by the second parameter UpdateBehavior: 
     should the update event be sent when write is called or 
     should the update event be sent when write is called AND the value is really changed
  
  on write :

	the write method acquire the RWlock, then update data, then broadcast event
  
  on read :
        the read method  wait for event to be received, then acquire the RWlock, 
	then copie data to a local cache

	if a more complex scheme of event wait is needed, a simple read that 
        just acquire the RW lock is present

  before first read, an event_listener_t with a local mask should be registered,
  then te read method need to know the mask to wait for the event

 */


/*
  TODO : 
 */

#include <ch.h>
#include <hal.h>

enum class UpdateBehavior {OnContentChange, Always}; 


template<typename T, UpdateBehavior ub = UpdateBehavior::Always>
class BlackBoard {
public:
  void write(const T &c);
  void read(T &c) const;
  void read(T &c, const eventmask_t events,
	    const sysinterval_t timout = TIME_INFINITE) const;
  void registerEvt(event_listener_t *lst, const eventmask_t events) const;
  
private:
  void writeLock(void) const;
  void writeUnlock(void) const;
  void readLock(void) const;
  void readUnlock(void) const;
  void broadcastUpdate(void) const;
  //  void broadcastUpdate(const eventflags_t flags) const;
  
private:
  T v{};
  mutable CONDVAR_DECL(cond);
  mutable MUTEX_DECL(mu);
  mutable volatile bool   writerWaiting=false;
  mutable volatile size_t readersWaiting=0U;
  mutable EVENTSOURCE_DECL(eventSource);
};


template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::write(const T &c) {
  writeLock();
  if constexpr (ub == UpdateBehavior::OnContentChange) {
      const bool change = v != c;
      if (change)
	v = c;
      writeUnlock();
      if (change)
	broadcastUpdate();
    } else {
    v = c;
    writeUnlock();
    broadcastUpdate();
  }
};

template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::read(T &c, const eventmask_t events,
			     const sysinterval_t timout) const {
  chEvtWaitOneTimeout(events, timout);
  read(c);
} ;

template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::read(T &c) const {
  readLock();
  c = v;
  readUnlock();
} ;


template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::readLock(void) const
{
  chMtxLock(&mu);
  while (writerWaiting == true) {
    chCondWait(&cond);
  }
  readersWaiting++;
  chMtxUnlock(&mu);
}

template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::readUnlock(void) const
{
  chMtxLock(&mu);
  if (--readersWaiting == 0) {
    chCondBroadcast(&cond);
  }
  chMtxUnlock(&mu);
}

template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::writeLock(void) const
{
  chMtxLock(&mu);
  while (writerWaiting == true) {
    chCondWait(&cond);
  }
  writerWaiting = true;
  while (readersWaiting > 0) {
    chCondWait(&cond);
  }
  chMtxUnlock(&mu);
}

template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::writeUnlock(void) const
{
  chMtxLock(&mu);
  writerWaiting = false;
  chCondBroadcast(&cond);
  chMtxUnlock(&mu);
}

template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::broadcastUpdate(void) const
{
  chEvtBroadcast(&eventSource);
}


template<typename T, UpdateBehavior ub>
void BlackBoard<T, ub>::registerEvt(event_listener_t *lst, const eventmask_t events) const
{
  chEvtRegisterMask(&eventSource, lst, events);
}
