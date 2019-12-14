#pragma once
#include "frozen/map.h"
#include "frozen/string.h"
#include "frozen/set.h"
#include "confParameters.hpp"

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
 
 struct parameter_value_t {
    default_variant_t defaut;
    validator_variant_t   validator;
  };

  constexpr auto conf_dict = MAKEMAP(PARAMETERS_MAP);
