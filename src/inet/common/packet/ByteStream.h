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
#include "inet/common/packet/Defs.h"

namespace inet {

class ByteOutputStream {
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

    uint8_t operator[](int64_t i) { return bytes[i]; }
    int64_t getSize() const { return bytes.size(); }
    int64_t getRemainingSize() const { return bytes.size() - position; }
    int64_t getPosition() const { return position; }
    const std::vector<uint8_t>& getBytes() { return bytes; }
    std::vector<uint8_t> *copyBytes(int64_t offset = 0, int64_t length = -1) {
        return new std::vector<uint8_t>(bytes.begin() + offset, bytes.begin() + (length == -1 ? bytes.size() : offset + length));
    }

    void seek(int64_t position) { this->position = position; }

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

    void readBytes(std::vector<uint8_t>& bytes, int64_t offset = 0, int64_t length = -1) {
        if (length == -1)
            length = bytes.size();
        for (int64_t i = 0; i < length; i++) {
            if (position == bytes.size()) {
                isReadBeyondEnd_ = true;
                return;
            }
            bytes[offset + i] = bytes[position++];
        }
    }

    uint8_t readUint8() {
        return readByte();
    }

    uint16_t readUint16() {
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

