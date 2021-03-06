#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"

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





