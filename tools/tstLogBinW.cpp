#include <fcntl.h> 
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cstdint>


/*
g++ -Og --std=c++17 -Werror -Wall -Wextra -I../source tstLogBinW.cpp -o genLog

MAGIC_NUMBER.NUMFIELDS.FIELDSIZE.{type->codage sur 1 octet, name\0, name\0, ..., MAGIC_NUMBER}
+ MAGIC.RECORD.MAGIC etc etc [données packées, alignée]

*/


#include "binaryLogFrame.hpp"

constexpr uint32_t magicNumber = 0xFACEC0DE;

constexpr struct BinaryHeader {
  const uint32_t magicBegin = magicNumber;
  const uint8_t numField = serializerDescription.size();
  const uint8_t paddingSize = sizeof(Serializer::_padding);
  const uint16_t fieldSize = sizeof(SerializerDescriptionItem);
  const decltype(serializerDescription) description = serializerDescription;
  const uint32_t magicEnd = magicNumber;
} binaryHeader;

struct FramedBinaryRecord {
  Serializer	 data;
  const uint32_t magicEnd = magicNumber;
};

int main()
{
  printf ("sizeof wp = %lu\n", sizeof(Serializer));
  for (const auto& [s,t,d,u] : serializerDescription) {
    printf("var %s of type %u in unit %s with scale %f\n", d, static_cast<uint8_t>(t), u, s);
    printf("sizeof binary header = %lu\n", sizeof(binaryHeader));
  }

  const int fd = open("/tmp/binaryLog.LOG", O_WRONLY | O_CREAT, 0644);
  if (write(fd, &binaryHeader, sizeof(binaryHeader)) != sizeof(binaryHeader)) {
    perror("write");
  }

  FramedBinaryRecord framedData[] = {
				     {.data = {.pressure = 10150000, .temperature=20, .zone=31}},
				     {.data = {.pressure = 10160000, .temperature=30, .zone=32}},
				     {.data = {.pressure = 10170000, .temperature=40, .zone=33}},
				     {.data = {.pressure = 10180000, .temperature=35, .zone=34}}  };  


  for (size_t j=0; j<1000000; j++)
  for (size_t i=0; i< 4; i++) {
    if (write(fd, &framedData[i], sizeof(FramedBinaryRecord)) != sizeof(FramedBinaryRecord)) {
      perror("write");
    }
  }
  
  close(fd);
  
  return 0;
}


























