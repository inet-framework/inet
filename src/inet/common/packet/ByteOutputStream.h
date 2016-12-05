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

#ifndef __INET_BYTEOUTPUTSTREAM_H_
#define __INET_BYTEOUTPUTSTREAM_H_

#include <assert.h>
#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class provides an efficient in memory byte output stream.
 *
 * Most functions are implemented in the header to allow inlining.
 * TODO: review efficiency
 */
class INET_API ByteOutputStream {
  protected:
    std::vector<uint8_t> bytes;

  public:
    uint8_t operator[](int64_t i) { return bytes[i]; }
    int64_t getSize() const { return bytes.size(); }
    int64_t getPosition() const { return bytes.size(); }
    const std::vector<uint8_t>& getBytes() { return bytes; }
    std::vector<uint8_t> *copyBytes(int64_t offset = 0, int64_t length = -1) {
        return new std::vector<uint8_t>(bytes.begin() + offset, bytes.begin() + (length == -1 ? bytes.size() : offset + length));
    }

    void writeByte(uint8_t byte) {
        bytes.push_back(byte);
    }

    void writeByteRepeatedly(uint8_t byte, int64_t count) {
        for (int64_t i = 0; i < count; i++)
            bytes.push_back(byte);
    }

    void writeBytes(const std::vector<uint8_t>& bytes, int64_t offset = 0, int64_t length = -1) {
        if (length == -1)
            length = bytes.size();
        for (int64_t i = 0; i < length; i++)
            this->bytes.push_back(bytes[offset + i]);
    }

    void writeUint8(uint8_t byte) {
        writeByte(byte);
    }

    void writeUint16(uint16_t value) {
        bytes.push_back((uint8_t)(value >> 8));
        bytes.push_back((uint8_t)(value >> 0));
    }

    void writeUint32(uint32_t value) {
        bytes.push_back((uint8_t)(value >> 24));
        bytes.push_back((uint8_t)(value >> 16));
        bytes.push_back((uint8_t)(value >> 8));
        bytes.push_back((uint8_t)(value >> 0));
    }

    void writeUint64(uint64_t value) {
        bytes.push_back((uint8_t)(value >> 56));
        bytes.push_back((uint8_t)(value >> 48));
        bytes.push_back((uint8_t)(value >> 40));
        bytes.push_back((uint8_t)(value >> 32));
        bytes.push_back((uint8_t)(value >> 24));
        bytes.push_back((uint8_t)(value >> 16));
        bytes.push_back((uint8_t)(value >> 8));
        bytes.push_back((uint8_t)(value >> 0));
    }
};

} // namespace

#endif // #ifndef __INET_BYTEOUTPUTSTREAM_H_

