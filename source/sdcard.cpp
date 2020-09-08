#include <ch.h>
#include <hal.h>
#include "sdcard.hpp"
#include "stdutil.h"	
#include "sdio.h"
#include <frozen/map.h>
#include <frozen/string.h>
#include "hardwareConf.hpp"
#include "confFile.hpp"
#include "threadAndEventDecl.hpp"
#include "receiveBaselink.hpp"

#define XSTR(s) STR(s)
#define STR(s) #s
  

namespace {
  BarometerData baroData{};
  DiffPressureData diffPressData{};
  ImuData imuData{};
  Vec3f   attitude{};
  AirSpeed relAirSpeed{};
  CommonGpsData gpsData{};
  SensorslogFormat logFormat{};
  std::string_view syslogName = "syslog";
  std::string_view sensorlogName = "sensors";

  constexpr auto severityName = frozen::make_map<Severity, frozen::string> ({
	{Severity::Debug, "DEBUG"},
	{Severity::Info, "INFO"},
	{Severity::Warning, "WARNING"},
	{Severity::Fatal, "FATAL"},
	{Severity::Internal, "INTERNAL (please report bug)"}
    });

  constexpr uint32_t magicNumber = 0xFACEC0DE;

  bool highTimeStampPrecision;

  static inline time_conv_t time_i2us_64(time_conv_t interval) {
    return (((interval * (time_conv_t)1000000UL) + (time_conv_t)CH_CFG_ST_FREQUENCY - (time_conv_t)1UL) / (time_conv_t)CH_CFG_ST_FREQUENCY);
  }
}

static constexpr uint32_t operator"" _seconde (unsigned long long int duration)
{
  return duration;
}

static constexpr uint32_t operator"" _megabyte (unsigned long long int size)
{
  return size;
}

bool SdCard::initHardware()
{
  if (not sdioIsCardResponding()) {
    DebugTrace("µSD card not present, or not reponding\r\n");
    return false;
  }
  
  const SdLiteStatus se = SdLiteLogBase::initOnce(&freeSpaceInKo);
  switch (se) {
  case SdLiteStatus::OK : DebugTrace(" freeSpaceInKo = %lu Mo", freeSpaceInKo/1024); break;
  case SdLiteStatus::FATFS_ERROR : DebugTrace("sdLogInit: Fatfs error"); return false;
  case SdLiteStatus::INTERNAL_ERROR : DebugTrace("sdLogInit: Internal error"); return false;
  default: break;
  }

  return (hardareInitialised = true);
}

bool SdCard::init()
{
  if (not hardareInitialised)
    initHardware();

  if (hardareInitialised) 
    self = this;
  else
    self = nullptr;
  
  return hardareInitialised;
}

bool SdCard::initInThreadContext()
{
  // registerEvt must be done in the thread that will wait on event,
  // so cannot be done in init method which is called by the parent thread
  syslogName = CONF("syslog.filename");
  sensorlogName = CONF("sensorslog.filename");
  ahrsType = static_cast<AhrsType>(CONF("ahrs.type"));
  serialMode = static_cast<SerialMode>(CONF("uart.mode"));
  logFormat = static_cast<SensorslogFormat>(CONF("sensorslog.format"));
  logGps =  (serialMode != SERIAL_NOT_USED);
  highTimeStampPrecision = CONF("thread.frequency.d_press") >= 100;
  
  if (sdLogInit() != true)
    return false;
  
  writeSyslogHeader();
  baro.blackBoard.registerEvt(&baroEvent, BARO_EVT);
  dp.blackBoard.registerEvt(&diffPressEvent, PDIF_EVT);
  imu.blackBoard.registerEvt(&imuEvent, IMU_EVT);

  switch (logFormat) {
  case SENSORS_TSV:
    writeTSVSensorlogHeader();
    break;
  case  SENSORS_BINARY:
     writeBinarySensorlogHeader();
     break;
  }
  return true;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"

bool SdCard::loop()
{
  // log each new differential sample
  const eventmask_t event = chEvtWaitOneTimeout(PDIF_EVT, TIME_MS2I(1000)); 

  if (event) {
    baro.blackBoard.read(baroData);
    dp.blackBoard.read(diffPressData);
    relwind.blackBoard.read(relAirSpeed);
    switch (serialMode) {
    case PPRZ_IN_OUT:
      receivePPL.blackBoard.read(gpsData);
      break;
    case NMEA_IN:
      receiveNMEA.blackBoard.read(gpsData);
      break;
    case UBX_IN:
      receiveUBX.blackBoard.read(gpsData);
      break;
    default:
      break;
    }
    switch (logFormat) {
    case SENSORS_TSV:
      return writeTSVSensorlog();
    case  SENSORS_BINARY:

      return writeBinarySensorlog();
    }
  }

  // timout is not considered as an error
  return true;
}


bool SdCard::writeTSVSensorlog(void)
{
  SdLiteStatus se;
  
  if (ahrsType == RAW_IMU) {
    imu.blackBoard.read(imuData);
    if (logGps == true) {
      se = writeTSVSensorlog_RAW_AND_GPS();
    } else {
      se = writeTSVSensorlog_RAW_NO_GPS();
    }
  } else /* (ahrsType == HEADLESS_AHRS */ {
    ahrs.blackBoard.read(attitude);
    if (logGps == true) {
      se =  writeTSVSensorlog_HEADLESS_AND_GPS();
    } else {
      se = writeTSVSensorlog_HEADLESS_NO_GPS();
    }
  }
  
  switch (se) {
  case SdLiteStatus::FATFS_ERROR : DebugTrace("sdWrite sensors: Fatfs error");
    return false;
  case SdLiteStatus::INTERNAL_ERROR : DebugTrace("sdWrite sensors: Internal error");
    return false;
  default: break;
  }
  return true;
}

SdLiteStatus SdCard::writeTSVSensorlog_RAW_AND_GPS(void)
{
  return  logSensors("%4.2f\t%3.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.3f\t%lu\t%lu\t%u\t"
		      "%ld\t%d\t%u\t%d\t"
		      "%.2f\t%.1f\t",
		      baroData.pressure,
		      baroData.temp,
		      diffPressData[0].pressure,
		      diffPressData[1].pressure,
		      diffPressData[2].pressure,
		      diffPressData[0].temp,
		      diffPressData[1].temp,
		      diffPressData[2].temp,
		      relAirSpeed.velocity,
		      relAirSpeed.alpha,
		      relAirSpeed.beta,
		      imuData.acc.v[0],
		      imuData.acc.v[1],
		      imuData.acc.v[2],
		      imuData.gyro.v[0],
		      imuData.gyro.v[1],
		      imuData.gyro.v[2],
		      gpsData.rtcTime.millisecond / 1000.0f,
		      gpsData.utm_east,
		      gpsData.utm_north,
		      gpsData.utm_zone,
		      gpsData.alt,
		      gpsData.course,
		      gpsData.speed,
		      gpsData.climb,
		      adc.getPowerSupplyVoltage(),
		      adc.getCoreTemp());
}

SdLiteStatus SdCard::writeTSVSensorlog_RAW_NO_GPS(void)
{
  return logSensors("%4.2f\t%3.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.2f\t%.1f\t",
		      baroData.pressure,
		      baroData.temp,
		      diffPressData[0].pressure,
		      diffPressData[1].pressure,
		      diffPressData[2].pressure,
		      diffPressData[0].temp,
		      diffPressData[1].temp,
		      diffPressData[2].temp,
		      relAirSpeed.velocity,
		      relAirSpeed.alpha,
		      relAirSpeed.beta,
		      imuData.acc.v[0],
		      imuData.acc.v[1],
		      imuData.acc.v[2],
		      imuData.gyro.v[0],
		      imuData.gyro.v[1],
		      imuData.gyro.v[2],
		      adc.getPowerSupplyVoltage(),
		      adc.getCoreTemp());
}

SdLiteStatus SdCard::writeTSVSensorlog_HEADLESS_AND_GPS(void)
{
  return logSensors("%4.2f\t%3.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.3f\t%lu\t%lu\t%u\t"
		      "%ld\t%d\t%u\t%d\t"
		      "%.2f\t%.1f\t",
		      baroData.pressure, baroData.temp,
		      diffPressData[0].pressure,
		      diffPressData[1].pressure,
		      diffPressData[2].pressure,
		      diffPressData[0].temp,
		      diffPressData[1].temp,
		      diffPressData[2].temp,
		      relAirSpeed.velocity,
		      relAirSpeed.alpha,
		      relAirSpeed.beta,
		      rad2deg(attitude.v[0]), rad2deg(attitude.v[1]), rad2deg(attitude.v[2]),
		      gpsData.rtcTime.millisecond / 1000.0f,
		      gpsData.utm_east,
		      gpsData.utm_north,
		      gpsData.utm_zone,
		      gpsData.alt,
		      gpsData.course,
		      gpsData.speed,
		      gpsData.climb,
		      adc.getPowerSupplyVoltage(),
		      adc.getCoreTemp());
}

SdLiteStatus SdCard::writeTSVSensorlog_HEADLESS_NO_GPS(void)
{
  return logSensors("%4.2f\t%3.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.2f\t%.2f\t%.2f\t"
		      "%.4f\t%.4f\t%.4f\t"
		      "%.2f\t%.1f\t",
		      baroData.pressure, baroData.temp,
		      diffPressData[0].pressure,
		      diffPressData[1].pressure,
		      diffPressData[2].pressure,
		      diffPressData[0].temp,
		      diffPressData[1].temp,
		      diffPressData[2].temp,
		      relAirSpeed.velocity,
		      relAirSpeed.alpha,
		      relAirSpeed.beta,
		      rad2deg(attitude.v[0]), rad2deg(attitude.v[1]), rad2deg(attitude.v[2]),
		      adc.getPowerSupplyVoltage(),
		      adc.getCoreTemp());
}



#pragma GCC diagnostic pop 


bool  SdCard::sdLogInit(void)
{
  SdLiteStatus se;
  std::string syslogN(syslogName.data(), syslogName.size());
  std::string sensorN(sensorlogName.data(), sensorlogName.size());


  if (logFormat == SENSORS_BINARY)
    sensorN += "_BIN_";

  
  se = syslog.openLog(ROOTDIR, syslogN.c_str());
  switch (se) {
  case SdLiteStatus::OK : DebugTrace("sdOpenLog syslog Ok"); break;
  case SdLiteStatus::FATFS_ERROR : DebugTrace("sdOpenLog syslog: Fatfs error");
    return false;
  case SdLiteStatus::INTERNAL_ERROR : DebugTrace("sdOpenLog syslog: Internal error");
    return false;
  default: break;
  }

  se = sensors.openLog(ROOTDIR, sensorN.c_str());
  switch (se) {
  case SdLiteStatus::OK : DebugTrace("sdOpenLog sensors Ok"); break;
  case SdLiteStatus::FATFS_ERROR : DebugTrace("sdOpenLog sensors: Fatfs error");
    return false;
  case SdLiteStatus::INTERNAL_ERROR : DebugTrace("sdOpenLog sensors: Internal error");
    return false;
  default: break;
  }

  
  return true;
}


void  SdCard::writeSyslogHeader(void)
{
  logSyslog(Severity::Info, "syslog start");
  logSyslog(Severity::Info, XSTR(GIT_VERSION));
  
    logSyslog(Severity::Info, "Kernel:       %s", CH_KERNEL_VERSION);
#ifdef HAL_VERSION
  logSyslog(Severity::Info, "Hal:          %s", HAL_VERSION);
#endif

#ifdef CH_COMPILER_NAME
  logSyslog(Severity::Info, "Compiler:     %s", CH_COMPILER_NAME);
#endif
#ifdef PORT_COMPILER_NAME
  logSyslog(Severity::Info, "Compiler:     %s", PORT_COMPILER_NAME);
#endif

#ifdef CH_ARCHITECTURE_NAME
  logSyslog(Severity::Info, "Architecture: %s", CH_ARCHITECTURE_NAME);
#endif
#ifdef PORT_ARCHITECTURE_NAME
  logSyslog(Severity::Info, "Architecture: %s", PORT_ARCHITECTURE_NAME);
#endif
  

#ifdef CH_CORE_VARIANT_NAME
  logSyslog(Severity::Info, "Core Variant: %s", CH_CORE_VARIANT_NAME);
#endif
#ifdef PORT_CORE_VARIANT_NAME
  logSyslog(Severity::Info, "Core Variant: %s", PORT_CORE_VARIANT_NAME);
#endif
#ifdef STM32_SYSCLK
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
  logSyslog(Severity::Info, "Main STM32_SYSCLK frequency %.2f Mhz", STM32_SYSCLK/1e6f);
#pragma GCC diagnostic pop
#endif

#ifdef CH_PORT_INFO
  logSyslog(Severity::Info, "Port Info:    %s", CH_PORT_INFO);
#endif
#ifdef PORT_INFO
  logSyslog(Severity::Info, "Port Info:    %s", PORT_INFO);
#endif

#ifdef PLATFORM_NAME
  logSyslog(Severity::Info, "Platform:     %s", PLATFORM_NAME);
#endif

#ifdef BOARD_NAME
  logSyslog(Severity::Info, "Board:        %s", BOARD_NAME);
#endif

  
#ifdef __DATE__
#ifdef __TIME__
  logSyslog(Severity::Info, "Build time:   %s%s%s", __DATE__, " - ", __TIME__);
#endif
#endif
}

void  SdCard::writeTSVSensorlogHeader(void)
{
  std::string header;
  
  header = "baro.p\t"
    "baro.t\t"
    "dp[0].p\t"
    "dp[1].p\t"
    "dp[2].p\t"
    "dp[0].t\t"
    "dp[1].t\t"
    "dp[2].t\t"
    "velocity\t";

  if (ahrsType == RAW_IMU) {
    header += "alpha\t"
      "beta\t"
      "acc.x\t"
      "acc.y\t"
      "acc.z\t"
      "gyro.x\t"
      "gyro.y\t"
      "gyro.z\t";
  } else  if (ahrsType == HEADLESS_AHRS) {
    header += 
      "alpha\t"
      "beta\t"
      "pitch  \t"
      "roll  \t"
      "yaw  \t"
      "vcc\t"
      "CoreTemp\t";
  } else {
    SdCard::logSyslog(Severity::Fatal, "ahrsType HEADLESS_COMPLETE not yet implemented");
  }
  
  if (logGps == true) {
    header += "ms_in_day\t"
      "utm_east\t"
      "utm_north\t"
      "utm_zone\t"
      "alt\t"
      "course\t"
      "speed\t"
      "climb\t";
  }
  
  header +=  "vcc\t"
    "CoreTemp\n" ;
  
  logSensors(header.c_str());
}

void  SdCard::writeBinarySensorlogHeader(void)
{
  static constexpr struct BinaryHeader {
    const uint32_t magicBegin = magicNumber;
    const uint8_t numField = serializerDescription.size();
    const uint8_t paddingSize = sizeof(Serializer::_padding);
    const uint16_t fieldSize = sizeof(SerializerDescriptionItem);
    const decltype(serializerDescription) description = serializerDescription;
    const uint32_t magicEnd = magicNumber;
  } binaryHeader;

  if (self != nullptr)  {
    auto [s, record] = sensors.borrow<uint8_t>(sizeof(binaryHeader));
    memcpy(record, &binaryHeader, sizeof(binaryHeader));
  }
}

constexpr float OINV(double quotient) {
  return static_cast<float>(1.0/quotient);
}
bool SdCard::writeBinarySensorlog(void)
{
  SdLiteStatus se;
  struct FramedBinaryRecord {
    Serializer	 data;
    uint32_t magicEnd = magicNumber;
  };
  
  if (self != nullptr) {
    imu.blackBoard.read(imuData);
    ahrs.blackBoard.read(attitude);

    auto [s, framedData] = sensors.borrow<FramedBinaryRecord>();
    if (s == SdLiteStatus::OK) {
      framedData.data.systime = time_i2us_64(chVTGetSystemTimeX())/100UL; //1
      framedData.data.baro_pressure = baroData.pressure; //2
      framedData.data.diff_pressure_central = diffPressData[0].pressure;//3
      framedData.data.diff_pressure_horizontal = diffPressData[1].pressure;//4
      framedData.data.diff_pressure_vertical = diffPressData[2].pressure;//5
      framedData.data.air_velocity = relAirSpeed.velocity;//6
      framedData.data.alpha_angle = relAirSpeed.alpha;//7
      framedData.data.beta_angle = relAirSpeed.beta;//8
      framedData.data.accel_x = imuData.acc.v[0];//9
      framedData.data.accel_y = imuData.acc.v[1];//10
      framedData.data.accel_z = imuData.acc.v[2];//11
      framedData.data.gyro_x = imuData.gyro.v[0];//12
      framedData.data.gyro_y = imuData.gyro.v[1];//13
      framedData.data.gyro_z = imuData.gyro.v[2];//14
      framedData.data.rtc_time = gpsData.rtcTime.millisecond; // 15
      framedData.data.utm_east = gpsData.utm_east; // 16
      framedData.data.utm_north = gpsData.utm_north; // 17
      framedData.data.altitude = gpsData.alt; // 18
      framedData.data.baro_temperature = baroData.temp * OINV(SC19); // 19
      framedData.data.diff_temperature_central = diffPressData[0].temp * OINV(SC20); // 20
      framedData.data.diff_temperature_horizontal = diffPressData[1].temp * OINV(SC21); // 21
      framedData.data.diff_temperature_vertical = diffPressData[2].temp * OINV(SC22); // 22
      framedData.data.attitude_x = rad2deg(attitude.v[0]) * OINV(SC23); // 23
      framedData.data.attitude_y = rad2deg(attitude.v[1]) * OINV(SC24); // 24
      framedData.data.attitude_z = rad2deg(attitude.v[2]) * OINV(SC25); // 25
      framedData.data.course = gpsData.course; // 26
      framedData.data.speed = gpsData.speed; // 27
      framedData.data.climb_speed = gpsData.climb; // 28
      framedData.data.ps_5V = adc.getPowerSupplyVoltage() * OINV(SC29); // 29
      framedData.data.core_temperature = adc.getCoreTemp() * OINV(SC30); // 30
      framedData.data.utm_zone = gpsData.utm_zone; // 31
      framedData.magicEnd = magicNumber;
    }
  }  else {
    se = SdLiteStatus::NOT_READY;
  }
  
  switch (se) {
  case SdLiteStatus::FATFS_ERROR : DebugTrace("sdWrite sensors: Fatfs error");
    return false;
  case SdLiteStatus::INTERNAL_ERROR : DebugTrace("sdWrite sensors: Internal error");
    return false;
  default: break;
  }
  return true;
}


SdLiteStatus SdCard::logSensors (const char* fmt, ...)
{
  va_list ap;

  if (self != nullptr) {
    va_start(ap, fmt);
    self->lock();
    self->sensors.writeFmt(16, highTimeStampPrecision ?
			    "[%.4f] : " :  "[%.3f] : ",
			    time_i2us_64(chVTGetSystemTimeX())/1e6);
    auto retVal = self->sensors.vwriteFmt(SYSLOG_BUFFER_SIZE/2, fmt, &ap);
    va_end(ap);
    self->unlock();
    return retVal;
  } else {
    return SdLiteStatus::NOT_READY;
  }
}


SdLiteStatus SdCard::logSyslog (const Severity severity, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char buffer[SYSLOG_BUFFER_SIZE/2];
  
#ifdef TRACE 
#include "printf.h"
  if ((reinterpret_cast<SerialDriver *>(chp))->state == SD_READY) {
    va_list apc;
    va_copy(apc, ap);
    chvprintf(chp, fmt, apc);
    va_end(apc);
    chprintf(chp, "\r\n");
  }
#endif
  
  if (self != nullptr) {
    int charIdx = snprintf(buffer, sizeof buffer, "[%.3f] %s : ",
			   TIME_I2MS(chVTGetSystemTimeX())/1000.0,
			   severityName.at(severity).data());
    if (charIdx >= static_cast<int>(sizeof buffer))
      goto  message_to_big;

    charIdx += vsnprintf(buffer + charIdx,
			 sizeof buffer - charIdx - 1,
			 fmt, ap);
    va_end(ap);
    if (charIdx >= static_cast<int>(sizeof buffer))
      goto  message_to_big;
   
    charIdx += snprintf(buffer + charIdx,
			sizeof buffer - charIdx - 1,
			"\r\n");
    if (charIdx >= static_cast<int>(sizeof buffer))
      goto  message_to_big;

    self->lock();
    auto retVal = self->syslog.writeFmt(charIdx + 1, "%s", buffer);
    self->unlock();
    return retVal;
  } else {
    va_end(ap);
    return SdLiteStatus::NOT_READY;
  }

 message_to_big:
  return SdLiteStatus::TOO_BIG;
}

SdLiteStatus SdCard::logSyslog (const char* fmt, ...)
{
  va_list ap;
  
  va_start(ap, fmt);
#ifdef TRACE 
#include "printf.h"
   if ((reinterpret_cast<SerialDriver *>(chp))->state == SD_READY) {
    va_list apc;
    va_copy(apc, ap);
    chvprintf(chp, fmt, apc);
    va_end(apc);
  }
#endif
  
  if (self != nullptr) {
    self->lock();
    auto retVal = self->syslog.vwriteFmt(SYSLOG_BUFFER_SIZE/2, fmt, &ap);
    va_end(ap);
    self->unlock();
    return retVal;
  } else {
    va_end(ap);
    return SdLiteStatus::NOT_READY;
  }
}


SdCard  *SdCard::self = nullptr;
