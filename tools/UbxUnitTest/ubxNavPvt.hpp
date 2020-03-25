#pragma once

#include <functional>
#include <array>
#include <cstddef>
#include <cstring>


constexpr uint16_t syncBytes = 0x62B5;

enum class UbxError {
		     OK,
		     TOPIC_NOT_BIND,
		     TOPIC_LEN,
		     SIZE_TOO_BIG,
		     CHKSUM,
		     READ
};

enum class UbxState {
		     WAIT_FOR_STX,
		     WAIT_FOR_PREAMBLE,
		     WAIT_FOR_DATA_AND_CHKSUM,
};



struct TopicLen_t {
  uint16_t t;
  uint16_t l;
};


template <size_t BS, size_t ACS>
class UbxNavPvtSerialDecoder {
public:
  using Buffer_t = std::array<std::byte, BS>;
  using MsgCB_t = std::function<void(const TopicLen_t&, const std::byte*)>;
  using UbxTopics_t = std::array<TopicLen_t, ACS>;
  using ReadChannelCB_t = std::function<int(std::byte *bpt, size_t len)>;
  UbxNavPvtSerialDecoder(const UbxTopics_t& _topics,
			 const MsgCB_t& _msgCb,
			 const ReadChannelCB_t& _readCb) :
    topics(_topics),
    msgCb(_msgCb),
    readCb(_readCb)
  {} ;
  UbxError step(void);
  const char * getLastError(void) const {return lastError;};
private:
  UbxState state = UbxState::WAIT_FOR_STX;
  Buffer_t buffer{};
  const UbxTopics_t topics;
  const MsgCB_t msgCb;
  const ReadChannelCB_t readCb;
  UbxError  isValidTopic(const TopicLen_t tl);
  static uint16_t checksum(std::byte const *data, size_t bytes);
  char lastError[80];
};


template <size_t BS, size_t ACS>
UbxError UbxNavPvtSerialDecoder<BS, ACS>::step(void)
{
  switch (state)  {
  case UbxState::WAIT_FOR_STX :
    uint16_t stx;
    buffer[0] = buffer[1];
    if (const auto cnt = readCb(&(buffer[1]), 1); cnt != 1) {
      snprintf(lastError, sizeof(lastError)-1,
	       "Ubx : read %d bytes instead of requested 1", cnt);
      return UbxError::READ;
    }
    std::memcpy(&stx, &buffer, sizeof(stx));
    if (stx == syncBytes) 
      state =  UbxState::WAIT_FOR_PREAMBLE;
    break;
    
  case UbxState::WAIT_FOR_PREAMBLE :
    if (const auto cnt = readCb(&buffer[0], 4); cnt != 4) {
      state = UbxState::WAIT_FOR_STX;
      snprintf(lastError, sizeof(lastError)-1,
	       "Ubx : read %d bytes instead of requested 4", cnt);
      return UbxError::READ;
    }
    state =  UbxState::WAIT_FOR_DATA_AND_CHKSUM;
    break;
    
  case UbxState::WAIT_FOR_DATA_AND_CHKSUM :
    TopicLen_t toplen{};
    uint16_t chksum=0;
    state = UbxState::WAIT_FOR_STX;
    memcpy(&toplen.t, &buffer, sizeof(toplen.t));
    toplen.t = __builtin_bswap16(toplen.t);
    memcpy(&toplen.l, &buffer[2], sizeof(toplen.l));
    switch (isValidTopic(toplen)) {
    case UbxError::TOPIC_NOT_BIND :
      snprintf(lastError, sizeof(lastError)-1,
	       "Ubx : topic %x not bind", toplen.t);
      return UbxError::TOPIC_NOT_BIND;
    case UbxError::TOPIC_LEN :
      snprintf(lastError, sizeof(lastError)-1,
	       "Ubx : topic %x size incorrect received length  %u",
	       toplen.t, toplen.l);
      return UbxError::TOPIC_LEN;
    default : break;
    }
    if (toplen.l > (BS-6)) {
      snprintf(lastError, sizeof(lastError)-1,
	       "Ubx : topic %x need %u bytes, capacity is %lu bytes",
	       toplen.t, toplen.l+6, BS);
      return UbxError::SIZE_TOO_BIG;
    }
    if (const auto cnt = readCb(&buffer[4], toplen.l+2); cnt != toplen.l+2) {
      snprintf(lastError, sizeof(lastError)-1,
	       "Ubx : read %d bytes instead of requested %d", cnt, toplen.l+2);
      return UbxError::READ;
    }
    memcpy(&chksum, &buffer[toplen.l+4], sizeof(chksum));
    if (const auto f = checksum(buffer.data(), toplen.l+4); f != chksum) {
      snprintf(lastError, sizeof(lastError)-1,
	       "Ubx : chksum = calculated =%x, rec = %x", f, chksum);
      return UbxError::CHKSUM;
    }
    msgCb(toplen, &buffer[4]);
    break;
  }
  return UbxError::OK;
}

template <size_t BS, size_t ACS>
uint16_t UbxNavPvtSerialDecoder<BS, ACS>::checksum(std::byte const *data,
						   size_t bytes)
{
  uint8_t sum1 = 0, sum2 = 0;

  for (size_t i=0; i<bytes; i++) {
    sum1 += static_cast<uint8_t>(data[i]);
    sum2 += sum1;
  }

  return (sum2<<8) | sum1;
}


template <size_t BS, size_t ACS>
UbxError UbxNavPvtSerialDecoder<BS, ACS>::isValidTopic(const TopicLen_t tlToTest)
{
  //  [&toFind](const MyStruct& x) { return x.m_id == toFind.m_id;});
  
  auto tlfind = std::find_if(topics.begin(), topics.end(),
			     [tlToTest] (const auto& tl) { return tl.t == tlToTest.t;});

  if (tlfind == topics.end())
    return UbxError::TOPIC_NOT_BIND;
  else if ((*tlfind).l != tlToTest.l)
    return UbxError::TOPIC_LEN;
  else
    return UbxError::OK;
}
