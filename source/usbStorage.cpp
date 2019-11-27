#include "usb_msd.h"
#include "ch.h"
#include "hal.h"
#include "stdutil.h"
#include "sdcard.hpp"
#include "usbStorage.hpp"
#include "threadAndEventDecl.hpp"
#include "sdLog.h"
#include <stdio.h>
#include <string.h>
#include <sdio.h>


namespace {

/* USB mass storage configuration */
/* Turns on a LED when there is I/O activity on the USB port */
void usbActivity(bool active)
{
  palWriteLine(LINE_LED_GREEN, active ? PAL_HIGH : PAL_LOW);
}

USBMassStorageConfig msdConfig =
{
    &USBD,
    (BaseBlockDevice*) &SDCD1,
    USB_MS_DATA_EP,
    &usbActivity,
    "ChibiOS",
    "smartprobe",
    "0.1"
};

}

bool UsbStorage::init()
{
  palEnableLineEvent(LINE_USB_VBUS, PAL_EVENT_MODE_BOTH_EDGES);

  return true;
}

[[noreturn]] 
bool UsbStorage::loop()
{
  chRegSetThreadName("UsbStorage:polling");
  
  do {
    palWaitLineTimeout(LINE_USB_VBUS, TIME_INFINITE);
    chThdSleepMilliseconds(10);
  } while (palReadLine(LINE_USB_VBUS) == PAL_LOW);
  
  chRegSetThreadName("UsbStorage:wait join");
  sdLogCloseAllLogs(LOG_FLUSH_BUFFER);
  chThdSleepMilliseconds(200);
  sdLogFinish();
  chThdSleepMilliseconds(100);
  sdcard.terminate().join();
  showBB.terminate().join();
  SdCard::logSyslog(Severity::Info, "UsbStorage:connected");
  chRegSetThreadName("UsbStorage:connected");
  /* connect sdcard sdc interface sdio */
  if (sdioConnect() == false) 
    chThdExit(MSG_TIMEOUT);
  
  init_msd_driver(NULL, &msdConfig);
  
  
  // wait transition to LOW with rebound management
  do {
    palWaitLineTimeout(LINE_USB_VBUS, TIME_INFINITE);
    chThdSleepMilliseconds(10);
  } while (palReadLine(LINE_USB_VBUS) == PAL_HIGH);
  
  
  SdCard::logSyslog(Severity::Info, "UsbStorage:DEconnected");
  chThdSleepMilliseconds(100);
  
  deinit_msd_driver();
  
  chThdSleepMilliseconds(500);
  sdioDisconnect();
  
  systemReset();
  while(true);
}

