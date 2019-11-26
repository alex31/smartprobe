#pragma once
#include "workerClass.hpp"

namespace TH_USBSTORAGE {
static constexpr size_t threadStackSize = 1024U;
}

class UsbStorage : public WorkerThread<TH_USBSTORAGE::threadStackSize, UsbStorage> {
public:
  UsbStorage(const tprio_t m_prio) :
    WorkerThread<TH_USBSTORAGE::threadStackSize, UsbStorage>("usb_storage",
							   m_prio) {};
private:
  bool init(void) final;
  bool loop(void) final;
};





