#include <cstring>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "binaryLogFrame.hpp"

#if defined USE_FMT
#define FMT_HEADER_ONLY
#include <fmt/format.h>		
#endif

/*
LINUX:
g++-10 -DUSE_FMT  -Ofast --std=c++20 -Werror -Wall -Wextra -I../source \
                  -I/home/alex/DEV/STM32/fmt/include logbin2tsv.cpp -o logbin2tsv

OSX:
g++-mp-10 -static-libgcc -static-libstdc++ -DUSE_FMT -Ofast --std=c++20 -Werror -Wall -Wextra -I../source -I/Users/alex/DEV/STM32/fmt/include logbin2tsv.cpp -o logbin2tsv

*/

/*
  TODO : if slowiness is a concern (30Mb/s on a 2013 intel 4770), can be rewrited using 
         openMP to use all the cores. The bottleneck is the binary to ascii conversion
 */


constexpr uint32_t magicNumber = 0xFACEC0DE;


union BinaryTypeUnion {
  uint8_t  U8;
  uint16_t U16;
  uint32_t U32;
  uint64_t U64;

  int8_t  I8;
  int16_t I16;
  int32_t I32;
  int64_t I64;
  
  char    C8;
  float   F32;
  double  F64;
};

class BinaryElem {
public:
  BinaryElem(const SerializerType m_type, double m_scale,
	     std::ifstream& m_ifs,  std::ofstream& m_ofs) :
    type(m_type), scale(m_scale == 0 ? 1 : m_scale), ofs(m_ofs), ifs(m_ifs) {assert (scale != 0);}
  [[nodiscard]] bool translate(void) {return loadFromIstream() && writeToOstream();}
private:
  [[nodiscard]] bool loadFromIstream(void);
  [[nodiscard]] bool writeToOstream(void);
  template<typename T>
  inline void tmplWriteToOstream(const T& t);

  
  BinaryTypeUnion elem;
  const SerializerType  type;
  double scale;
  std::ofstream &ofs;
  std::ifstream &ifs;
};


class BinaryLogReader {
  struct BinaryHeaderStart {
    uint32_t magicBegin;
    uint8_t numField;
    uint8_t paddingSize;
    uint16_t fieldSize;
  };
  
public:
  BinaryLogReader(const std::filesystem::path &m_input,
		  const std::filesystem::path &m_output);
  ~BinaryLogReader(void) {ofs.close(); ifs.close();}
  [[nodiscard]] bool fail(void) {return hasFail && ifs.fail();}
  
private:
  [[nodiscard]] bool readHeader(void);
  [[nodiscard]] bool readData(void);
  template <typename T>
  void populate(T &t);
  std::ifstream ifs;
  std::ofstream ofs;
  const std::filesystem::path input{}, output{};
  std::vector<BinaryElem> elems{};
  size_t paddingSize{};
  bool hasFail = false;
};



BinaryLogReader::BinaryLogReader(const std::filesystem::path &m_input,
				 const std::filesystem::path &m_output) :
  input(m_input), output(m_output)
{
  ifs.open(input);
  if (ifs.fail()) {
    perror("open binary file for reading");
    exit(-1);
  }
  ofs.open(output);
  if (ofs.fail()) {
    perror("open tsv file for writing");
    exit(-1);
  }
  
  hasFail = readHeader() and readData();
}

bool BinaryLogReader::readHeader(void)
{
  BinaryHeaderStart headerStart{};
  uint32_t magicEnd{};
  
  if (fail()) return false;
  populate(headerStart);
  paddingSize = headerStart.paddingSize;
  if (headerStart.magicBegin != magicNumber) {
    std::cerr << "file format error : magic number (BEGIN) not recognised\n";
    return false;
  }

  if (headerStart.fieldSize != sizeof(SerializerDescriptionItem)) {
    std::cerr << "file format error : SerializerDescriptionItem size not known\n";
    return false;
  }
  
  printf("DBG> numfield = %u, fieldsize = %u, paddingSize =  %u\n",
	 headerStart.numField, headerStart.fieldSize, headerStart.paddingSize);

  for (size_t i=0; i < headerStart.numField; i++) {
    SerializerDescriptionItem sdi;
    populate(sdi);
    printf ("scale = %g type = %u, desc='%s' unit='%s'\n",
	    sdi.scale, static_cast<uint8_t>(sdi.type),
	    sdi.description, sdi.unitName);
    ofs << sdi.description << ":" << sdi.unitName << "\t";
    elems.push_back(BinaryElem(sdi.type, sdi.scale, ifs, ofs));
  }
  ofs << "\n";
  populate(magicEnd);
  if (magicEnd != magicNumber) {
    std::cerr << "file format error : magic number (END) not recognised\n";
    return false;
  }

	   
  return (not fail());
}

bool BinaryLogReader::readData(void)
{
  uint32_t magicEnd{};

  while (ifs.good()) {
    for (auto& e : elems)
      if (e.translate() == false) return false;

    ifs.ignore(paddingSize);
    ofs << "\n";
    populate(magicEnd);
    if (magicEnd != magicNumber) {
      std::cerr << "file format error : magic number (END DATA) not recognised\n";
      return false;
    }
  }
  return true;
}

template<typename T>
void BinaryLogReader::populate(T &t)
{
  ifs.read(reinterpret_cast<char *>(&t), sizeof(T));
}


template<typename T>
inline void BinaryElem::tmplWriteToOstream(const T& t)
{
  if constexpr (std::is_same_v<T, char>) {
    ofs << t << "\t";
  } else {
#if defined USE_FMT
    // more than two time faster than cstdio sprintf to convert binary to ascii
    // need fmt library
    std::string sfmt = fmt::format(FMT_STRING("{:.10g}\t"), t * scale);
    ofs.write(sfmt.c_str(), sfmt.length());
#else
    // slower, do not need fmt library
    char f2str[64];
    const auto len = snprintf(f2str, sizeof(f2str), "%.10g\t",  t * scale);
    ofs.write(f2str, len);
#endif
  }
}


bool BinaryElem::loadFromIstream(void)
{
  ifs.read(&elem.C8, serializerTypeSize[static_cast<size_t>(type)]);
  return ifs.good();
}


bool BinaryElem::writeToOstream(void)
{
  switch (type) {
  case SerializerType::U8:  tmplWriteToOstream(elem.U8); break;
  case SerializerType::U16: tmplWriteToOstream(elem.U16); break;
  case SerializerType::U32: tmplWriteToOstream(elem.U32); break;
  case SerializerType::U64: tmplWriteToOstream(elem.U64); break;
  case SerializerType::I8:  tmplWriteToOstream(elem.I8); break;
  case SerializerType::I16: tmplWriteToOstream(elem.I16); break;
  case SerializerType::I32: tmplWriteToOstream(elem.I32); break;
  case SerializerType::I64: tmplWriteToOstream(elem.I64); break;
  case SerializerType::C8:  tmplWriteToOstream(elem.C8); break;
  case SerializerType::F32: tmplWriteToOstream(elem.F32); break;
  case SerializerType::F64: tmplWriteToOstream(elem.F64); break;
  case SerializerType::ERROR: break;
  }

  return ifs.good();
}











int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "usage " << argv[0] << " binaryIn tsvOut\n";
    exit(-1);
  }
  BinaryLogReader blr(argv[1], argv[2]);
  
  return 0;
}


























