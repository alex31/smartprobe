#pragma once
#include <stdint.h>




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


static constexpr I2CDriver& BaroI2CD      = I2CD2;
static constexpr I2CDriver& DiffPressI2CD = I2CD4;

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
