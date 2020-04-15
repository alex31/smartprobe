#pragma once

#include <cstdint>
#include <array>
#include <experimental/array>

#define XSTR(s) STR(s)
#define STR(s) #s

#define IDESC(var,scale,unit) SerializerDescriptionItem{scale, getType(Serializer{}.var), XSTR(var), unit}
#define DESC(i) IDESC(PM##i, SC##i, DC##i)
#define DECL(i) PT##i PM##i

enum class SerializerType : uint32_t {U8=0, U16, U32, U64,
				      I8, I16, I32, I64,
				      C8,
				      F32, F64, ERROR
};

constexpr size_t serializerTypeSize[] = {
					 1,2,4,8,
					 1,2,4,8,
					 1,
					 4,8,0
};

template<typename T>
constexpr SerializerType getType([[maybe_unused]] const T& t) {
  if constexpr (std::is_same_v<T, char>)
		 return SerializerType::C8;
  if constexpr (std::is_same_v<T, uint8_t>)
		 return SerializerType::U8;
  if constexpr (std::is_same_v<T, uint16_t>)
		 return SerializerType::U16;
  if constexpr (std::is_same_v<T, uint32_t>)
		 return SerializerType::U32;
  if constexpr (std::is_same_v<T, uint64_t>)
		 return SerializerType::U64;
  if constexpr (std::is_same_v<T, int8_t>)
		 return SerializerType::I8;
  if constexpr (std::is_same_v<T, int16_t>)
		 return SerializerType::I16;
  if constexpr (std::is_same_v<T, int32_t>)
		 return SerializerType::I32;
  if constexpr (std::is_same_v<T, int64_t>)
		 return SerializerType::I64;
  if constexpr (std::is_same_v<T, float>)
		 return SerializerType::F32;
  if constexpr (std::is_same_v<T, double>)
		 return SerializerType::F64;
  return SerializerType::ERROR;
}


struct __attribute__((packed)) SerializerDescriptionItem {
  double scale;
  SerializerType type;
  char description[32];
  char unitName[32];
};

#include "binaryLogFrame_conf.hpp"

// pragma padded and PackedSerializer are there to trigger compilation error if
// field are not in the "right order" : sorted in size from bigger to smaller
// to avoid padding and unaligned access
// there will be padding between two messages, and we need to know the size of padding
// which is dependant on the bigger scalar in the structre, this explains the need to have
// normal and packed version of the structure.
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wpadded"

struct  __attribute__((packed)) PackedSerializer {
  DECLS;
}; 

struct Serializer {
  DECLS;
  char _padding[8-sizeof(PackedSerializer)%8] = {0};
}; 

constexpr static inline auto serializerDescription = std::experimental::make_array(DESCS);
#pragma GCC diagnostic pop













