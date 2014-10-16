//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/common/serializer/headerserializers/SerializerUtil.h"
#include <netinet/in.h>

namespace inet {
namespace serializer {

void setOneByte(const uint8_t value, unsigned char *buf, unsigned int offset)
{
    uint8_t *p = (uint8_t *) (buf + offset);
    *p = value;
}

void setTwoByte(const uint16_t value, unsigned char *buf, unsigned int offset)
{
    uint16_t *p = (uint16_t *) (buf + offset);
    *p = htons(value);
}

void setFourByte(const uint32_t value, unsigned char *buf, unsigned int offset)
{
    uint32_t *p = (uint32_t *) (buf + offset);
    *p = htonl(value);
}

void setNByte(const uint8_t *value, unsigned int n, unsigned char *buf, unsigned int offset)
{
    for (unsigned int i = 0; i < n; i++)
    {
        uint8_t *p = (uint8_t *) (buf + offset + i);
#  if BYTE_ORDER == LITTLE_ENDIAN
        *p = value[i];
#  elif BYTE_ORDER == BIG_ENDIAN
        *p = value[n-i-1];
#else
# error "Please check BYTE_ORDER declaration"
#  endif
    }
}

void setBit(const bool value, unsigned int bitOffset, unsigned char *buf, unsigned int offset)
{
    uint8_t *p = (uint8_t *) (buf + offset);
    uint8_t byteValue = 1U << bitOffset;
    *p = value ? *p | byteValue : *p & ~byteValue;
}

uint8_t getOneByte(const unsigned char *buf, unsigned int offset)
{
    uint8_t *p = (uint8_t *) (buf + offset);
    return *p;
}

uint16_t getTwoByte(const unsigned char *buf, unsigned int offset)
{
    uint16_t *p = (uint16_t *) (buf + offset);
    return ntohs(*p);
}

uint32_t getFourByte(const unsigned char *buf, unsigned int offset)
{
    uint32_t *p = (uint32_t *) (buf + offset);
    return ntohl(*p);
}

void getNByte(uint8_t *value, unsigned int n, const unsigned char *buf, unsigned int offset)
{
    for (unsigned int i = 0; i < n; i++)
    {
        uint8_t *p = (uint8_t *) (buf + offset + i);
#  if BYTE_ORDER == LITTLE_ENDIAN
        value[i] = *p;
#  elif BYTE_ORDER == BIG_ENDIAN
        value[n-i-1] = *p;
#else
# error "Please check BYTE_ORDER declaration"
#  endif
    }
}

bool getBit(unsigned int bitOffset, const unsigned char *buf, unsigned int offset)
{
    uint8_t *p = (uint8_t *) (buf + offset);
    uint8_t byteValue = 1U << bitOffset;
    return *p & byteValue;
}


uint16_t swapByteOrder16(uint16_t v){
  return ((v & 0xFF) << 8) | ((v & 0xFF00) >> 8);
}

uint32_t swapByteOrder32(uint32_t v){
  return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
}

} // namespace serializer
} // namespace inet
