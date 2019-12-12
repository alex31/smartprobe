#include <cstdio>
#include <cstring>
#include <utility>
#include <variant>
#include <string>
#include <map>
#include <cassert>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string_view>
#include "ctre.hpp"
#include "frozen/map.h"
#include "frozen/string.h"
#include "frozen/set.h"

/*
  parse line : 


  si le fichier n'existe pas : le creer avec les valeurs par defaut


  faire de l'exemple une classe avec des noms de membres 
  et de methodes plus intelligents que dans l'exemple
*/

using namespace std::literals;

namespace {
#define NAMESET(...) frozen::make_set<named_val_t>({__VA_ARGS__})
#define RANGEINT(a, b) range_int_t{a,b}
#define RANGEDOUBLE(a, b) range_double_t{a,b}
#define MAKEMAP(...) frozen::make_map<frozen::string, parameter_value_t>({__VA_ARGS__})

  
  struct named_val_t  {
    int val;
    frozen::string valName;
    constexpr bool operator<(const named_val_t& other) const {
      return val < other.val;
    }
  };
  struct range_int_t {int min; int max;};
  struct range_double_t {double min; double max;};
  
  
  
  using validator_variant_t = std::variant<
    std::monostate, 
    range_int_t,    
    range_double_t,
    frozen::set<named_val_t, 1>,
    frozen::set<named_val_t, 2>,
    frozen::set<named_val_t, 3>,
    frozen::set<named_val_t, 4>,
    frozen::set<named_val_t, 5>,
    frozen::set<named_val_t, 6>,
    frozen::set<named_val_t, 7>,
    frozen::set<named_val_t, 8>,
    frozen::set<named_val_t, 9>,
    frozen::set<named_val_t, 10>
    >;

  using default_variant_t = std::variant<int, double, bool, std::string_view>;
  using value_variant_t = std::variant<int, double, bool, std::string, std::monostate>;
  constexpr std::monostate NONAMESET = {};
  
  constexpr std::array<const char* const, 5> variantName =
    { "integer", "floating point",
      "boolean", "string", "empty"};
  
  struct parameter_value_t {
    default_variant_t defaut;
    validator_variant_t   validator;
  };

  std::pair<bool, const parameter_value_t &> verifyKey(const std::string_view &k);
  bool resolveDefine(value_variant_t &retVal, const validator_variant_t   &validator);
  bool validate(const std::string_view &k, const std::string_view &v,
		value_variant_t &value, const validator_variant_t &validator);

  constexpr size_t firstSetIndex = 3U;
  constexpr size_t numberOfSets = std::variant_size_v<validator_variant_t> - firstSetIndex;

  template<size_t N>
  void printNamedSet(const validator_variant_t &vtor)
  {
    if (std::holds_alternative<frozen::set<named_val_t, N>>(vtor)) {
      const frozen::set<named_val_t, N> &set = std::get<frozen::set<named_val_t, N>>(vtor);
      printf("set = ");
      for (const named_val_t &nv : set) {
	printf("\"%s\"=%d,", nv.valName.data(), nv.val);
      }
      printf("\n");
    }
    if constexpr (N > 1) { // recurse over set size using template meta programming
	printNamedSet<N-1>(vtor);
      }
  }

  template<size_t N>
  std::string variant2str(const validator_variant_t &vtor)
  {
    std::string rep;
    char buffer[80];
    
    if (std::holds_alternative<frozen::set<named_val_t, N>>(vtor)) {
      const frozen::set<named_val_t, N> &set = std::get<frozen::set<named_val_t, N>>(vtor);
      for (const named_val_t &nv : set) {
	snprintf(buffer, sizeof(buffer), "\"%s\"=%d,", nv.valName.data(), nv.val);
	rep += buffer;
      }
      return rep;
    }
    if constexpr (N > 1) { // recurse over set size using template meta programming
	return variant2str<N-1>(vtor);
      }
    return "variant2str internal error";
  }

  
std::string variant2str(const default_variant_t &dvar)
{
   char buffer[80] = "{}";
   
   if (std::holds_alternative<int>(dvar)) {
     snprintf (buffer, sizeof(buffer), "%d", std::get<int>(dvar));
    } else  if (std::holds_alternative<double>(dvar)) {
     snprintf (buffer, sizeof(buffer), "%f", std::get<double>(dvar));
    } else  if (std::holds_alternative<bool>(dvar)) {
     snprintf (buffer, sizeof(buffer), "%s", std::get<bool>(dvar) ? "true" : "false");
    } else  if (std::holds_alternative<std::string_view>(dvar)) {
     strncpy(buffer, std::get<std::string_view>(dvar).data(), sizeof(buffer));
    } 

   return std::string(buffer);
}

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

  void printValue(const value_variant_t &value)
  {
    if (std::holds_alternative<std::string>(value)) {
      printf("S=%s\n", variant2str(value).c_str());
    } else if (std::holds_alternative<int>(value)) {
      printf("I=%s\n", variant2str(value).c_str());
    } else if (std::holds_alternative<bool>(value)) {
      printf("B=%s\n", variant2str(value).c_str());
    } else if (std::holds_alternative<double>(value)) {
      printf("F=%s\n", variant2str(value).c_str());
    } 
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


  constexpr auto conf_dict =
    MAKEMAP (
	     {"filename.sensors", {"capteurs"sv, NONAMESET }},
	     {"sensor.pressure.frequency", {1, NAMESET({1, "frequency_one"}) }},
	     {"sensor.other", {2, NAMESET({1, "other_one"}, {2, "other_two"}) }},
	     {"sensor.o2", {3, NAMESET({1, "o2_low"}, {3, "o2_high"}) }},
	     {"thread.waitmillisecond", {20.5f, RANGEINT(9, 1000) }},
	     {"sensor.icm20600.init.cr1", {0, RANGEINT(0, 16) }},
	     {"thread.enable.imu", {true, NONAMESET }},
	     {"thread.frequency", {100, RANGEINT(10, 1000) }} );
  

  std::pair<std::string, value_variant_t> parseLine(const std::string_view line)
  {
    using namespace ctre::literals;
    constexpr auto line_match =  ctre::match<R"(([\w\.]+)\s*=\s*([\+\-]?[\w\.]+)\s*#?.*)">;
    //    constexpr auto empty_match =  ctre::match<R"((\s*#?.*))">;
    constexpr auto empty_match =  ctre::match<R"((\s*)|(\s*#.*))">;
    constexpr auto double_match =  ctre::match<R"([\+\-]?[\d\.]+)">;
    constexpr auto integer_match =  ctre::match<R"([\+\-]?[\d]+)">;
    constexpr auto bool_true_match =  ctre::match<R"((?:true)|(?:TRUE))">;
    constexpr auto bool_false_match =  ctre::match<R"((?:false)|(?:FALSE))">;
    auto string_match = [&]  (auto s) -> bool {return (not double_match(s))
	and (not integer_match(s))
	and (not bool_true_match(s))
	and (not bool_false_match(s));};

    value_variant_t paramVal= std::monostate{};
    std::string_view k = "";
    
    if (auto [whole, key, val] = line_match(line); whole) {
      k = key;
      const std::string_view v(val);
      printf("key %.*s => val=%.*s :: ",
	     static_cast<int>(k.length()), k.data(),
	     static_cast<int>(v.length()), v.data());
      if (integer_match(v)) {
	paramVal = atoi(v.data());
      } else if (double_match(v)) {
	paramVal = atof(v.data());
      } else if (bool_true_match(v)) {
	paramVal = true;
      } else if (bool_false_match(v)) {
	paramVal = false;
      } else if (string_match(v)){
	paramVal = std::string(v.data());
      }

      auto [exists, param] = verifyKey(k);
      if (not exists) {
	printf("NOT KNOW key %.*s in conf_dict\r\n",
	       static_cast<int>(k.length()), k.data());
      } else {
	if (not resolveDefine(paramVal, param.validator)) {
	  printf("NOT KNOW define %s for key %.*s in conf_dict\r\n",
		 std::get<std::string>(paramVal).c_str(),
		 static_cast<int>(k.length()), k.data());
	} else {
	  // type of default should be compatible with value read in configuration file
	  if (param.defaut.index() != paramVal.index()) {
	    // authorized mismatch are : default is double and read value is int
	    if ((not std::holds_alternative<int>(paramVal)) ||
		(not std::holds_alternative<double>(param.defaut))) {
	      printf("mismatch type for key %.*s : read %s instead of specified %s\r\n",
		     static_cast<int>(k.length()), k.data(),
		     variantName[paramVal.index()],
		     variantName[param.defaut.index()]
		     );
	    }
	  }
	  if (not validate(k, v, paramVal, param.validator)) {
	    printf("value %.*s for key %.*s does not validate constraints\n",
		   static_cast<int>(v.length()), v.data(),
		   static_cast<int>(k.length()), k.data());   
	  }
	}
      }
      
      
    } else if (empty_match(line)) {
      paramVal = std::monostate{};
    } else {
      paramVal = std::string("parse line error on line : ") + std::string(line);      
    }
    return {std::string(k), paramVal};
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

    const std::string_view &sv = std::get<std::string>(value);
    const auto [exists, valueFromAlias] = getValueByName<numberOfSets>(sv, validator);
    if (exists == true) {
      value = valueFromAlias;
      printf("alias %s remplaced by %d\n", sv.data(),  valueFromAlias);
      return true;
    } else {
      printf("alias %s not found\n", sv.data());
      return false;
    }
  }

  bool validate(const std::string_view &k, const std::string_view &v,
		value_variant_t &value,
		const validator_variant_t &validator)
  {
    bool success = false;
    
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
	    printf("value %d for key %.*s is not in range [%d .. %d]",
		   std::get<int>(value),
		   static_cast<int>(k.length()), k.data(),
		   min, max);
	  }
	} else if (std::holds_alternative<double>(value)) {
	  const double val = std::get<double>(value);
	  success = ((val >= min) && (val <= max));
	  if (not success) {
	    printf("value %f for key %.*s is not in range [%d .. %d]",
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
	    printf("value %d for key %.*s is not in range [%f .. %f]",
		   std::get<int>(value),
		   static_cast<int>(k.length()), k.data(),
		   min, max);
	  }
	} else if (std::holds_alternative<double>(value)) {
	  const double val = std::get<double>(value);
	  success = ((val >= min) && (val <= max));
	  if (not success) {
	    printf("value %f for key %.*s is not in range [%f .. %f]",
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
	  printf("value %.*s for key %.*s is not in set %s",
		 static_cast<int>(v.length()), v.data(),
		 static_cast<int>(k.length()), k.data(),
		 variant2str<numberOfSets>(validator).c_str());
	}
      }
    }
    return success;
  }

  std::pair<bool, const parameter_value_t &> verifyKey(const std::string_view &k)
  {
    const auto &it =  conf_dict.find(k);
    //    return std::pair<bool, const parameter_value_t &>{it != conf_dict.end(), it->second};
    return {it != conf_dict.end(), it->second};
  }

  
  
  
} // end of anonymous namespace

  



int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;
  
  std::ifstream infile("conf.txt");
  std::string line;

  std::map<std::string, value_variant_t> dictionaire;

  while (std::getline(infile, line)) {
    //    std::cout << "L1:" << line << std::endl;
    const auto [key, value] = parseLine(line);
    dictionaire[key] = value;
    printValue(value);
  }

  for (auto const& [key, val] : conf_dict) {
    printf ("\n\n.................\nkey = %s : \n", key.data());
    if (not dictionaire.contains(std::string(key.data()))) {
      printf ("key %s not in file\n", key.data());
    }

    const default_variant_t& d = val.defaut;
    if (std::holds_alternative<int>(d)) {
      printf ("default<integer> = %s\n", variant2str(d).c_str());
    } else  if (std::holds_alternative<double>(d)) {
      printf ("default<double> = %s\n", variant2str(d).c_str());
    } else  if (std::holds_alternative<bool>(d)) {
      printf ("default<boolean> = %s\n", variant2str(d).c_str());
    } else  if (std::holds_alternative<std::string_view>(d)) {
      printf ("default<string> = %s\n", variant2str(d).c_str());
    } else {
      printf ("no default\n");
    }


    const validator_variant_t& vtor = val.validator;
    if (std::holds_alternative<std::monostate>(vtor)) {
      printf ("no validation");
    } else if (std::holds_alternative<range_int_t>(vtor)) {
      printf ("default<integer range> = [%d .. %d]\n",
	      std::get<range_int_t>(vtor).min,
	      std::get<range_int_t>(vtor).max);
    } else if (std::holds_alternative<range_double_t>(vtor)) {
      printf ("default<double range> = [%f .. %f]\n",
	      std::get<range_double_t>(vtor).min,
	      std::get<range_double_t>(vtor).max);
    } else {
      printNamedSet<numberOfSets>(vtor);
    }
    
  }

  
  
  return 0;
}
