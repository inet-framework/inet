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

#ifndef __BYTESTREAM_H_
#define __BYTESTREAM_H_

#include <assert.h>
#include <inttypes.h>
#include <vector>
#include "Defs.h"

namespace inet {

class ByteOutputStream {
  protected:
    std::vector<uint8_t> bytes;

  public:
    int64_t getSize() const { return bytes.size(); }
    int64_t getPosition() const { return bytes.size(); }
    const std::vector<uint8_t>& getBytes() { return bytes; }
    uint8_t operator[](int64_t i) { return bytes[i]; }

    void writeByte(uint8_t byte) {
        bytes.push_back(byte);
    }

    void writeByteRepeatedly(uint8_t byte, int64_t count) {
        for (int64_t i = 0; i < count; i++)
            bytes.push_back(byte);
    }

    void writeBytes(const std::vector<uint8_t>& bytes, int64_t byteOffset = 0, int64_t byteLength = -1) {
        if (byteLength == -1)
            byteLength = bytes.size();
        for (int64_t i = 0; i < byteLength; i++)
            this->bytes.push_back(bytes[byteOffset + i]);
    }

    void writeInt16(uint16_t value) {
        bytes.push_back((uint8_t)(value >> 8));
        bytes.push_back((uint8_t)value);
    }
};

class ByteInputStream {
  protected:
    std::vector<uint8_t> bytes;
    int64_t position = -1;
    bool isReadBeyondEnd_ = false;

  public:
    ByteInputStream(const std::vector<uint8_t>& bytes, int64_t position = 0) :
        bytes(bytes),
        position(position)
    {
    }

    bool isReadBeyondEnd() const { return isReadBeyondEnd_; }

    int64_t getSize() const { return bytes.size(); }
    int64_t getRemainingSize() const { return bytes.size() - position; }
    int64_t getPosition() const { return position; }
    const std::vector<uint8_t>& getBytes() { return bytes; }
    uint8_t operator[](int64_t i) { return bytes[i]; }

    uint8_t readByte() {
        if (position == bytes.size()) {
            isReadBeyondEnd_ = true;
            return 0;
        }
        else return bytes[position++];
    }

    void readByteRepeatedly(uint8_t byte, int64_t count) {
        for (int64_t i = 0; i < count; i++) {
            if (position == bytes.size()) {
                isReadBeyondEnd_ = true;
                return;
            }
            auto readByte = bytes[position++];
            assert(byte == readByte);
        }
    }

    void readBytes(std::vector<uint8_t>& bytes, int64_t byteOffset = 0, int64_t byteLength = -1) {
        if (byteLength == -1)
            byteLength = bytes.size();
        for (int64_t i = 0; i < byteLength; i++) {
            if (position == bytes.size()) {
                isReadBeyondEnd_ = true;
                return;
            }
            bytes[byteOffset + i] = bytes[position++];
        }
    }

    uint16_t readInt16() {
        uint16_t value = 0;
        if (position == bytes.size()) {
            isReadBeyondEnd_ = true;
            return 0;
        }
        value += ((uint16_t)(bytes[position++]) << 8);
        if (position == bytes.size()) {
            isReadBeyondEnd_ = true;
            return 0;
        }
        value += ((uint16_t)(bytes[position++]));
        return value;
    }
};

} // namespace

#endif // #ifndef __BYTESTREAM_H_

