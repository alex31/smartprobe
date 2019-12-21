#pragma once
#include "workerClass.hpp"

namespace TH_USBSTORAGE {
static constexpr size_t threadStackSize = 1512U;
}

class UsbStorage : public WorkerThread<TH_USBSTORAGE::threadStackSize, UsbStorage> {
public:
  UsbStorage(const tprio_t m_prio) :
    WorkerThread<TH_USBSTORAGE::threadStackSize, UsbStorage>("usb_storage",
							   m_prio) {};
  void setModeEmergency(void) {emergency=true;};
private:
  friend  WorkerThread<TH_USBSTORAGE::threadStackSize, UsbStorage>;
  bool init(void) final;
  [[noreturn]] bool loop(void) final;
  bool emergency=false;
};





