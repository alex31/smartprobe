#pragma once
#include <stdint.h>
#include <array>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#pragma GCC diagnostic ignored "-Wunused-result"
#include <Eigen/Dense>
#pragma GCC diagnostic pop


/*
#                 ______   _____         
#                /  ____| |_   _|        
#                | (___     | |          
#                 \___ \    | |          
#                .____) |  _| |_         
#                \_____/  |_____|        
#                                                              _      _                   
#                                                             | |    (_)                  
#                  ___    ___    _ __   __   __   ___   _ __  | |_    _     ___    _ __   
#                 / __|  / _ \  | '_ \  \ \ / /  / _ \ | '__| | __|  | |   / _ \  | '_ \  
#                | (__  | (_) | | | | |  \ V /  |  __/ | |    \ |_   | |  | (_) | | | | | 
#                 \___|  \___/  |_| |_|   \_/    \___| |_|     \__|  |_|   \___/  |_| |_| 
*/

using namespace std::literals;

static constexpr uint32_t operator"" _hz (unsigned long long int freq)
{
  return freq;
}
static constexpr uint32_t operator"" _khz (unsigned long long int freq)
{
  return freq * 1000UL;
}
static constexpr uint32_t operator"" _mhz (unsigned long long int freq)
{
  return freq * 1000_khz;
}
static constexpr long double operator"" _ohm (long double resistance)
{
  return resistance;
}
static constexpr long double operator"" _kohm (long double resistance)
{
  return resistance * 1000UL;
}
static constexpr uint32_t operator"" _percent (unsigned long long int freq)
{
  return freq * 100UL;
}

/*
#                 _                          _                                           
#                | |                        | |                                          
#                | |__     __ _   _ __    __| |  __      __   __ _   _ __    ___         
#                | '_ \   / _` | | '__|  / _` |  \ \ /\ / /  / _` | | '__|  / _ \        
#                | | | | | (_| | | |    | (_| |   \ V  V /  | (_| | | |    |  __/        
#                |_| |_|  \__,_| |_|     \__,_|    \_/\_/    \__,_| |_|     \___|        
#                                               _                     _            _            
#                                              | |                   (_)          | |           
#                  ___    ___    _ __    ___   | |_    _ __    __ _   _    _ __   | |_          
#                 / __|  / _ \  | '_ \  / __|  | __|  | '__|  / _` | | |  | '_ \  | __|         
#                | (__  | (_) | | | | | \__ \  \ |_   | |    | (_| | | |  | | | | \ |_          
#                 \___|  \___/  |_| |_| |___/   \__|  |_|     \__,_| |_|  |_| |_|  \__|         
*/


static constexpr float DIVIDER_R17 = 12.0_kohm;
static constexpr float DIVIDER_R18 = 2.2_kohm;
static constexpr float DIVIDER_R16 = 2.2_kohm;
static constexpr float DIVIDER_R19 = 2.2_kohm;
static constexpr float VCC_33 = 3.3f;
static constexpr size_t ADC_RESOLUTION_IN_BITS = 12U;
static constexpr uint32_t SAMPLE_MAX = (1<<ADC_RESOLUTION_IN_BITS) - 1;


static constexpr I2CDriver& BaroI2CD      = I2CD2;
static constexpr I2CDriver& DiffPressI2CD = I2CD4;
static constexpr SPIDriver& ImuSPID	  = SPID1;
static constexpr SerialDriver& ExtSD	  = SD1;

static constexpr uint32_t I2C_FAST_400KHZ_DNF3_R200NS_F50NS_PCLK54MHZ_TIMINGR = 0x10800C27;
static constexpr uint32_t I2C_FAST_1MHZ_DNF3_R100NS_F50NS_PCLK54MHZ_TIMINGR  = 0x00800617;
static constexpr uint32_t stm32_cr1_dnf(const uint32_t n) {
  return (n & 0x0f) << 8;
}

static constexpr I2CConfig i2ccfg_400 = {
  .timingr = I2C_FAST_400KHZ_DNF3_R200NS_F50NS_PCLK54MHZ_TIMINGR, // Refer to the STM32F7 reference manual
  .cr1 =  stm32_cr1_dnf(3U), // Digital noise filter activated (timingr should be aware of that)
  .cr2 = 0 // Only the ADD10 bit can eventually be specified here (10-bit addressing mode)
} ;

static constexpr I2CConfig i2ccfg_1000 = {
  .timingr = I2C_FAST_1MHZ_DNF3_R100NS_F50NS_PCLK54MHZ_TIMINGR, // Refer to the STM32F7 reference manual
  .cr1 = stm32_cr1_dnf(3U), // Digital noise filter activated (timingr should be aware of that)
  .cr2 = 0 // Only the ADD10 bit can eventually be specified here (10-bit addressing mode)
} ;

static constexpr SerialConfig serialDebugConsoleCfg =  {
							115200,
							0,
							USART_CR2_STOP1_BITS | USART_CR2_LINEN,
							0
};

static inline const adcsample_t* const vrefCalib33Ptr =
		       reinterpret_cast<adcsample_t *>(0x1FF0F44AU);
static constexpr uint32_t PowerLossAwakeTimeBeforeDeepSleep = 30U;

static const std::array<ioline_t, 12>
LineToStopInCaseOfPowerFailure= {LINE_LED_GREEN,	
				 LINE_LED_RED,
				 LINE_SPI1_NSS,
				 LINE_IMU_SPI_SCK1,	
				 LINE_IMU_SPI_MISO1,	
				 LINE_IMU_SPI_MOSI1,
				 LINE_AP_TX1,		
				 LINE_AP_RX1,		
				 LINE_PRESS_SCL4,	
				 LINE_PRESS_SDA4,	
				 LINE_BARO_SCL2,	
				 LINE_BARO_SDA2	
};

static inline void stopAllPeripherals (void) {
  // do not close peripherals : thread which use them will
  // chSysHalt. terminate thread will be too long  so just
  // put pins in highZ state and stop all clock peripherals but I²C

  for (const ioline_t line : LineToStopInCaseOfPowerFailure) {
    palSetLineMode(line, PAL_MODE_INPUT);
  }

  rccDisableSPI1();
  rccDisableADC1();
}

/*
#                                                    
#                                                    
#                 _   _   ___     ___   _ __         
#                | | | | / __|   / _ \ | '__|        
#                | |_| | \__ \  |  __/ | |           
#                 \__,_| |___/   \___| |_|           
#                            _    _    _              _       _                 
#                           | |  (_)  | |            | |     | |                
#                  ___    __| |   _   | |_     __ _  | |__   | |    ___         
#                 / _ \  / _` |  | |  | __|   / _` | | '_ \  | |   / _ \        
#                |  __/ | (_| |  | |  \ |_   | (_| | | |_) | | |  |  __/        
#                 \___|  \__,_|  |_|   \__|   \__,_| |_.__/  |_|   \___|        
*/
static constexpr float VOLTAGE_THRESHOLD = 10.0f;
static constexpr float VOLTAGE_ABSOLUTE_MINIMUM = 7.0f;
static constexpr float VOLTAGE_3_3_MINIMUM = 3.1f;
static constexpr float VOLTAGE_3_3_MAXIMUM = 3.5f;
static constexpr float NOMINAL_VOLTAGE_BY_ELEMENT = 3.6f;
static constexpr float MINIMUM_VOLTAGE_BY_ELEMENT = 2.9f;
static constexpr char ROOTDIR[] = "SMARTPROBE";
static constexpr char CONFIGURATION_FILENAME[] = "smartprobe.conf";
static constexpr time_secs_t SYSLOG_SYNC_PERIOD = 0U;
static constexpr time_secs_t SENSORS_SYNC_PERIOD = 60U;
static constexpr time_secs_t SENSORS_BUFFER_SIZE = 38400U;
static constexpr time_secs_t SYSLOG_BUFFER_SIZE = 512U;


namespace TH_BLINKER {
static constexpr size_t threadStackSize = 512U;
}
namespace TH_SDCARD {
static constexpr size_t threadStackSize = 5120U;
}
namespace TH_SHOWBLACKBOARD {
static constexpr size_t threadStackSize = 1536U;
}
namespace TH_TRANSMITPPRZLINK {
static constexpr size_t threadStackSize = 1536U;
}
namespace TH_RECEIVEBASELINK {
static constexpr size_t threadStackSize = 3072U;
}
namespace TH_ADC {
static constexpr size_t threadStackSize = 512U;
}
namespace TH_USBSTORAGE {
static constexpr size_t threadStackSize = 1536U;
}
namespace TH_IMU {
static constexpr size_t threadStackSize = 1024U;
}
namespace TH_DIFFPRESS {
static constexpr size_t threadStackSize = 1536U;
}
namespace TH_BARO {
  static constexpr size_t threadStackSize = 1536U;
}
namespace TH_AHRS {
static constexpr size_t threadStackSize = 1536U;
}
namespace TH_RELWIND {
static constexpr size_t threadStackSize = 1536U;
}
namespace TH_DYNSWDIO {
static constexpr size_t threadStackSize = 512U;
}
namespace TH_RTCSYNC {
static constexpr size_t threadStackSize = 1024U;
}
namespace TH_HEALTHCHECK {
static constexpr size_t threadStackSize = 2048U;
}
