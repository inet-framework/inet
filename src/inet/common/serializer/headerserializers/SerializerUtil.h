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

#ifndef SERIALIZERUTIL_H_
#define SERIALIZERUTIL_H_

#include "inet/common/INETDefs.h"

namespace inet {
namespace serializer {

void setOneByte(const uint8_t value, unsigned char *buf, unsigned int offset = 0);
void setTwoByte(const uint16_t value, unsigned char *buf, unsigned int offset = 0);
void setFourByte(const uint32_t value, unsigned char *buf, unsigned int offset = 0);
void setNByte(const uint8_t *value, unsigned int n, unsigned char *buf, unsigned int offset = 0);
void setBit(const bool value, unsigned int bitOffset, unsigned char *buf, unsigned int offset = 0);

uint8_t getOneByte(const unsigned char *buf, unsigned int offset = 0);
uint16_t getTwoByte(const unsigned char *buf, unsigned int offset = 0);
uint32_t getFourByte(const unsigned char *buf, unsigned int offset = 0);
void getNByte(uint8_t *value, unsigned int n, const unsigned char *buf, unsigned int offset = 0);
bool getBit(unsigned int bitOffset, const unsigned char *buf, unsigned int offset = 0);

uint16_t swapByteOrder16(uint16_t v);
uint32_t swapByteOrder32(uint32_t v);

} // namespace serializer
} // namespace inet

#endif /* SERIALIZERUTIL_H_ */
