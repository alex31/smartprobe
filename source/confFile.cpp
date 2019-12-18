#include "confFile.hpp"
#include <ch.h>
#include <hal.h>
#include "stdutil.h"
#include "printf.h"
#include "cpp_heap_alloc.hpp"
#include "ttyConsole.hpp"       
#include "frozenDictionary.hpp"
#include "hardwareConf.hpp"
#include "sdcard.hpp"
#include <cstring>
#include <utility>
#include <variant>
#include <string>
#include <tuple>
#include <map>
#include <string_view>
#include "ctre.hpp"

/*
  parse line : 


  si le fichier n'existe pas : le creer avec les valeurs par defaut


  faire de l'exemple une classe avec des noms de membres 
  et de methodes plus intelligents que dans l'exemple
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"

using namespace std::literals;

namespace {
 
  constexpr std::array<const char* const, 5> variantName =
    { "integer", "floating point",
      "boolean", "string", "empty"};
  
 

  std::pair<bool, const parameter_value_t &> verifyKey(const std::string_view k);
  bool resolveDefine(value_variant_t &retVal, const validator_variant_t   &validator);
  bool validate(const std::string_view k, const std::string_view v,
		value_variant_t &value, const validator_variant_t &validator);

  constexpr size_t firstSetIndex = 3U;
  constexpr size_t numberOfSets = std::variant_size_v<validator_variant_t> - firstSetIndex;

  template<size_t N>
  [[maybe_unused]]
  std::string variant2str(const validator_variant_t &vtor)
  {
    std::string rep;
    char buffer[80];
    
    if (std::holds_alternative<frozen::set<named_val_t, N>>(vtor)) {
      const frozen::set<named_val_t, N> &set = std::get<frozen::set<named_val_t, N>>(vtor);
      for (const named_val_t &nv : set) {
	snprintf(buffer, sizeof(buffer), "\"%.*s\"=%d,",
		 int(nv.valName.size()), nv.valName.data(), nv.val);
	rep += buffer;
      }
      return rep;
    }
    if constexpr (N > 1) { // recurse over set size using template meta programming
	return variant2str<N-1>(vtor);
      }
    return "variant2str internal error";
  }

  template<size_t N>
  std::pair<bool,  int> getValueByName(const std::string_view name,
				       const validator_variant_t &vtor)
  {
    if (std::holds_alternative<frozen::set<named_val_t, N>>(vtor)) {
      const frozen::set<named_val_t, N> &set = std::get<frozen::set<named_val_t, N>>(vtor);
      for (const named_val_t &nv : set) 
	if (nv.valName == name) 
	  return {true, nv.val};
    }
    if constexpr (N > 1) { // recurse over set size using template meta programming
	return getValueByName<N-1>(name, vtor);
      }
    return {false, 0};
  }

  template<size_t N>
  std::pair<bool, const std::string_view> getNameByValue(const int value,
							 const validator_variant_t &vtor)
  {
    if (std::holds_alternative<frozen::set<named_val_t, N>>(vtor)) {
      const frozen::set<named_val_t, N> &set = std::get<frozen::set<named_val_t, N>>(vtor);
      for (const named_val_t &nv : set) 
	if (nv.val == value) 
	  return {true, std::string_view(nv.valName.data(), nv.valName.size())};
    }
    if constexpr (N > 1) { // recurse over set size using template meta programming
	return getNameByValue<N-1>(value, vtor);
      }
    return {false, ""};
  }

  std::string variant2str(const default_variant_t &dvar, const validator_variant_t &vtor)
  {
    char buffer[80] = "{}";
    
    if (std::holds_alternative<int>(dvar)) {
      const auto [success, sv] = getNameByValue<numberOfSets>(std::get<int>(dvar), vtor);
      if (success) 
	snprintf (buffer, sizeof(buffer), "%.*s [%d]",
		  int(sv.length()), sv.data(),
		  std::get<int>(dvar));
       else 
	snprintf (buffer, sizeof(buffer), "%d", std::get<int>(dvar));
    } else  if (std::holds_alternative<double>(dvar)) {
      snprintf (buffer, sizeof(buffer), "%f", std::get<double>(dvar));
    } else  if (std::holds_alternative<bool>(dvar)) {
      snprintf (buffer, sizeof(buffer), "%s", std::get<bool>(dvar) ? "true" : "false");
    } else  if (std::holds_alternative<std::string_view>(dvar)) {
      const auto &sv = std::get<std::string_view>(dvar);
      strncpy(buffer, std::string(sv).c_str(), sizeof(buffer));
    } 
    
    return std::string(buffer);
  }
  
  [[maybe_unused]]
  std::string variant2str(const value_variant_t &vvar)
  {
    char buffer[80] = "{}";
    
    if (std::holds_alternative<int>(vvar)) {
      snprintf (buffer, sizeof(buffer), "%d", std::get<int>(vvar));
    } else  if (std::holds_alternative<double>(vvar)) {
      snprintf (buffer, sizeof(buffer), "%f", std::get<double>(vvar));
    } else  if (std::holds_alternative<bool>(vvar)) {
      snprintf (buffer, sizeof(buffer), "%s", std::get<bool>(vvar) ? "true" : "false");
    } else  if (std::holds_alternative<std::string>(vvar)) {
      strncpy(buffer, std::get<std::string>(vvar).c_str(), sizeof(buffer));
    } 
    
    return std::string(buffer);
  }

  std::pair<bool, value_variant_t> default2Value(const default_variant_t dvar)
  {
    value_variant_t value;
    bool success = true;
    
    if (std::holds_alternative<int>(dvar)) {
      value =  std::get<int>(dvar);
    } else  if (std::holds_alternative<double>(dvar)) {
      value =  std::get<double>(dvar);
    } else  if (std::holds_alternative<bool>(dvar)) {
      value =  std::get<bool>(dvar);
    } else  if (std::holds_alternative<std::string_view>(dvar)) {
      value =  std::string(std::get<std::string_view>(dvar));
    } else {
      success = false;
    }
    
    return {success, value};
  }

  template<size_t N>
  void syslogNamedSet(const validator_variant_t &vtor)
  {
    if (std::holds_alternative<frozen::set<named_val_t, N>>(vtor)) {
      const frozen::set<named_val_t, N> &set = std::get<frozen::set<named_val_t, N>>(vtor);
      SdCard::logSyslogRaw("set of possible value is : ");
      for (const named_val_t &nv : set) {
	SdCard::logSyslogRaw("\"%.*s\"=%d, ",
			      nv.valName.size(), nv.valName.data(), nv.val);
      }
      SdCard::logSyslogRaw("\r\n");
    }
    if constexpr (N > 1) { // recurse over set size using template meta programming
	syslogNamedSet<N-1>(vtor);
      }
  }

  
 template<size_t N>
  bool isPresentInSet(const int value,
		      const validator_variant_t &vtor)
  {
    if (std::holds_alternative<frozen::set<named_val_t, N>>(vtor)) {
      const frozen::set<named_val_t, N> &set = std::get<frozen::set<named_val_t, N>>(vtor);
      for (const named_val_t &nv : set) 
	if (nv.val == value) 
	  return true;
      return false;
    }
    if constexpr (N > 1) { // recurse over set size using template meta programming
	return isPresentInSet<N-1>(value, vtor);
      }
    return false;
  }



std::tuple<bool, std::string, value_variant_t> parseLine(const std::string &line)
  {
    using namespace ctre::literals;
    constexpr auto line_match =  ctre::match<R"(([\w\.]+)\s*=\s*([\+\-]?[\w\.]+)\s*#?.*)">;
    constexpr auto empty_match =  ctre::match<R"((\s*)|(\s*#.*))">;
    constexpr auto double_match =  ctre::match<R"([\+\-]?[\d\.]+)">;
    constexpr auto integer_match =  ctre::match<R"([\+\-]?[\d]+)">;
    constexpr auto bool_true_match =  ctre::match<R"((?:true)|(?:TRUE))">;
    constexpr auto bool_false_match =  ctre::match<R"((?:false)|(?:FALSE))">;
    auto string_match = [&]  (auto s) -> bool {return (not double_match(s))
    	and (not integer_match(s))
    	and (not bool_true_match(s))
    	and (not bool_false_match(s));};

    bool success = true;
    value_variant_t paramVal= std::monostate{};
    std::string k = "";
    
    if (auto [whole, key, val] = line_match(line); whole) {
      k = key;
      const std::string  v(val);
      // DebugTrace("key %.*s => val=%.*s :: ",
      // 	     static_cast<int>(k.length()), k.data(),
      // 	     static_cast<int>(v.length()), v.data());
      if (integer_match(v)) {
    	paramVal = atoi(v.c_str());
      } else if (double_match(v)) {
    	paramVal = atof(v.c_str());
      } else if (bool_true_match(v)) {
    	paramVal = true;
      } else if (bool_false_match(v)) {
    	paramVal = false;
      } else if (string_match(v)){
    	paramVal = v;
      }

      auto [exists, param] = verifyKey(k);
      if (not exists) {
	success = false;
    	SdCard::logSyslog(Severity::Fatal, "parameter %s NOT KNOWN", k.c_str());
      } else {
    	if (not resolveDefine(paramVal, param.validator)) {
	  success = false;
    	  SdCard::logSyslog(Severity::Fatal, "define %s for key %s is NOT KNOWN",
    		 std::get<std::string>(paramVal).c_str(), k.c_str());
    	} else {
    	  // type of default should be compatible with value read in configuration file
    	  if (param.defaut.index() != paramVal.index()) {
	     success = false;
    	    // authorized mismatch are : default is double and read value is int
    	    if ((not std::holds_alternative<int>(paramVal)) ||
    		(not std::holds_alternative<double>(param.defaut))) {
    	      SdCard::logSyslog(Severity::Fatal, "mismatch type for key %s : "
				"read %s instead of specified %s",
				k.c_str(),
				variantName[paramVal.index()],
				variantName[param.defaut.index()]
				);
    	    }
    	  }
    	  if (not validate(k, v, paramVal, param.validator)) {
	    success = false;
    	    SdCard::logSyslog(Severity::Fatal, "value %s for key %s "
			      "does not validate constraints\n",
			      v.c_str(), k.c_str());
    	  }
    	}
      }
    } else if (empty_match(line)) {
      paramVal = std::monostate{};
    } else {
      success = false;
      paramVal = std::string("syntax error on line : ") + line;
      SdCard::logSyslog(Severity::Fatal, "%s", std::get<std::string>(paramVal).c_str());
    }
    return {success, k, paramVal};
  }

  bool resolveDefine(value_variant_t &value, const validator_variant_t   &validator)
  {
    if (not std::holds_alternative<std::string>(value)) {
      // if there is no name to resolve, it's fine 
      return true;
    }
    if (std::holds_alternative<std::monostate>(validator))  {
      // no validator : don't try to resolve define
      return true;
    }
    if (validator.index() < firstSetIndex) {
      // only set<named_val_t, ...> contains define aliases, and first set is @ index 3
      return false;
    }

    const std::string_view sv = std::get<std::string>(value);
    const auto [exists, valueFromAlias] = getValueByName<numberOfSets>(sv, validator);
    if (exists == true) {
      value = valueFromAlias;
      //      DebugTrace("alias %s remplaced by %d\n", sv.data(),  valueFromAlias);
      return true;
    } else {
      //      DebugTrace("alias %s not found\n", sv.data());
      return false;
    }
  }

  bool validate(const std::string_view k, const std::string_view v,
		value_variant_t &value,
		const validator_variant_t &validator)
  {
    bool success = false;
#if not defined(TRACE)
    (void) k;
    (void) v;
#endif
    
    if (std::holds_alternative<std::string>(value)) {
      success = true;
    } else {
      switch (validator.index()) {
      case 0: // std::monostate, all values are valid
	success= true;
	break;
      case 1: {// range_int_t,
	const auto [min, max] = std::get<range_int_t>(validator);
	if (std::holds_alternative<int>(value)) {
	  const int val = std::get<int>(value);
	  success = ((val >= min) && (val <= max));
	  if (not success) {
	    SdCard::logSyslog(Severity::Fatal, "value %d for key %.*s is not in range [%d .. %d]",
			      std::get<int>(value),
			      static_cast<int>(k.length()), k.data(),
			      min, max);
	  }
	} else if (std::holds_alternative<double>(value)) {
	  const double val = std::get<double>(value);
	  success = ((val >= min) && (val <= max));
	  if (not success) {
	     SdCard::logSyslog(Severity::Fatal, "value %f for key %.*s is not in range [%d .. %d]",
		   std::get<double>(value),
		   static_cast<int>(k.length()), k.data(),
		   min, max);
	  }
	}
	break;
      }
      case 2: {// range_double_t,
	const auto [min, max] = std::get<range_double_t>(validator);
	if (std::holds_alternative<int>(value)) {
	  const int val = std::get<int>(value);
	  success = ((val >= min) && (val <= max));
	  if (not success) {
	     SdCard::logSyslog(Severity::Fatal, "value %d for key %.*s is not in range [%f .. %f]",
		   std::get<int>(value),
		   static_cast<int>(k.length()), k.data(),
		   min, max);
	  }
	} else if (std::holds_alternative<double>(value)) {
	  const double val = std::get<double>(value);
	  success = ((val >= min) && (val <= max));
	  if (not success) {
	     SdCard::logSyslog(Severity::Fatal, "value %f for key %.*s is not in range [%f .. %f]",
		   std::get<double>(value),
		   static_cast<int>(k.length()), k.data(),
		   min, max);
	  }
	}
	break;
      }
      default: 
	success = isPresentInSet<numberOfSets>(std::get<int>(value), validator);
	if (not success) {
	   SdCard::logSyslog(Severity::Fatal, "value %.*s for key %.*s is not in set %s",
			     static_cast<int>(v.length()), v.data(),
			     static_cast<int>(k.length()), k.data(),
			     variant2str<numberOfSets>(validator).c_str());
	}
      }
    }
    return success;
  }

  std::pair<bool, const parameter_value_t &> verifyKey(const std::string_view k)
  {
    const auto &it =  conf_dict.find(k);
    return {it != conf_dict.end(), it->second};
  }

  
} // end of anonymous namespace

bool ConfigurationFile::populate(void)
{
  bool success = readConfFile();
  if (not success)
    success = writeConfFile();

  if (success) {
    success = verifyNotFilledParameters();
  }
  syslogInfoParameters();

  return success;
}

bool ConfigurationFile::readConfFile(void)
{
  FIL fil;
  FRESULT rc;
  char lineBuffer[120] = {0};
  bool readFileSuccess = true;

  //  auto m_test = ConfigurationFile_AT(*this, "filename.sensors");
  rc = f_open(&fil, fileName, FA_READ | FA_OPEN_EXISTING);
  if (rc != FR_OK) {
    SdCard::logSyslog(Severity::Fatal, "configuration file %s not found", fileName);
    goto fail;
  }
   
  do {
    f_gets(lineBuffer, sizeof(lineBuffer)-1, &fil);
    strtok(lineBuffer, "\r\n"); // remove CRLN
    const auto [success, key, value] = parseLine(lineBuffer);
    readFileSuccess = success & readFileSuccess;
    if (success && not std::holds_alternative<std::monostate>(value)) {
      dictionary[key] = value;
    }
    SdCard::logSyslog(Severity::Info, "read %s", lineBuffer);
  } while (not f_eof(&fil));

   rc = f_close(&fil);
   if (rc != FR_OK) {
     SdCard::logSyslog(Severity::Warning,
		       "fatfs close error on file %s", fileName);
     goto fail;
   }

   return readFileSuccess;
 fail:
   return false;
}

bool ConfigurationFile::writeConfFile(void)
{
  FIL fil;
  FRESULT rc;
  char lineBuffer[120] = {0};
  bool writeFileSuccess = true;

  //  auto m_test = ConfigurationFile_AT(*this, "filename.sensors");
  rc = f_open(&fil, fileName, FA_CREATE_NEW | FA_WRITE);
  if (rc != FR_OK) {
    SdCard::logSyslog(Severity::Fatal, "configuration file %s cannot be created", fileName);
    goto fail;
  }
  

  for (auto const& [key, param] : conf_dict) {
    UINT nbwf =0;
    int32_t nbw = snprintf(lineBuffer, sizeof(lineBuffer), "%.*s = %s\n",
			    static_cast<int>(key.size()), key.data(),
			   variant2str(param.defaut, param.validator).c_str()
			    );
    if ((nbw <= 0) || (nbw >= static_cast<int32_t>(sizeof(lineBuffer)))) {
      SdCard::logSyslog(Severity::Internal, "ConfigurationFile::writeConfFile : "
			"lineBuffer too small");
      writeFileSuccess = false;
    } else {
      rc = f_write(&fil, lineBuffer, nbw, &nbwf);
      if ((rc != FR_OK) || (nbw != static_cast<int32_t>(nbwf))) {
	SdCard::logSyslog(Severity::Fatal, "ConfigurationFile::writeConfFile : "
			  "f_write fails");
	writeFileSuccess = false;
      }
    }
  }
  
  rc = f_close(&fil);
  if (rc != FR_OK) {
    SdCard::logSyslog(Severity::Warning,
		      "fatfs close error on file %s", fileName);
    writeFileSuccess = false;
  }

  return writeFileSuccess;

 fail:
  return false;
}


bool ConfigurationFile::verifyNotFilledParameters(void)
{
  bool success = true;
  
  for (auto const& [key, param] : conf_dict) {
    std::string ks(key.data(), key.size());
    
    if (not dictionary.contains(ks)) {
      auto [m_success, defaut] = default2Value(param.defaut);
      success = m_success & success;
      if (m_success) {
	SdCard::logSyslog(Severity::Warning, "parameter %s is not supplied in %s: "
			  "using default %s",
			  ks.c_str(), CONFIGURATION_FILENAME,
			  variant2str(param.defaut, param.validator).c_str());
	dictionary[ks] = defaut;
      } else {
	SdCard::logSyslog(Severity::Fatal, "parameter %s should be supplied in %s: "
			  "no defaut for this parameter",
			  ks.c_str(),
			  CONFIGURATION_FILENAME);
      }
    }
  }
  return success;
}


void ConfigurationFile::syslogInfoParameters(void)
{
   for (auto const& [key, param] : conf_dict) {
     const validator_variant_t& vtor = param.validator;
     SdCard::logSyslog(Severity::Info, ".................\n");
     SdCard::logSyslogRaw("key = %s : ", key.data());
     const default_variant_t& d = param.defaut;
     if (std::holds_alternative<int>(d)) {
       SdCard::logSyslogRaw("default<integer> = %s; ", variant2str(d, vtor).c_str());
    } else  if (std::holds_alternative<double>(d)) {
      SdCard::logSyslogRaw("default<double> = %s; ", variant2str(d, vtor).c_str());
    } else  if (std::holds_alternative<bool>(d)) {
      SdCard::logSyslogRaw("default<boolean> = %s; ", variant2str(d, vtor).c_str());
    } else  if (std::holds_alternative<std::string_view>(d)) {
      SdCard::logSyslogRaw("default<string> = %s; ", variant2str(d, vtor).c_str());
    } else {
      SdCard::logSyslogRaw("no default; ");
    }

     if (std::holds_alternative<std::monostate>(vtor)) {
      SdCard::logSyslogRaw("no validation");
    } else if (std::holds_alternative<range_int_t>(vtor)) {
      SdCard::logSyslogRaw("RANGE is [%d .. %d]",
	      std::get<range_int_t>(vtor).min,
	      std::get<range_int_t>(vtor).max);
    } else if (std::holds_alternative<range_double_t>(vtor)) {
      SdCard::logSyslogRaw("RANGE is [%f .. %f]",
	      std::get<range_double_t>(vtor).min,
	      std::get<range_double_t>(vtor).max);
    } else {
      syslogNamedSet<numberOfSets>(vtor);
    }

    SdCard::logSyslogRaw("\r\n"); 
   }
}

const value_variant_t& ConfigurationFile::operator[]  (const std::string_view key) 
{
  return dictionary[std::string(key)];
}

#pragma GCC diagnostic pop
