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

#ifndef __INET_MEMORYINPUTSTREAM_H_
#define __INET_MEMORYINPUTSTREAM_H_

#include "inet/common/Units.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"

namespace inet {

using namespace units::values;

/**
 * This class provides an efficient in memory byte input stream.
 * Most functions are implemented in the header to allow inlining.
 */
// TODO: allow arbitrary mixed bit/byte reads
// TODO: add parameter checks
// TODO: review efficiency
class INET_API MemoryInputStream {
  protected:
    std::vector<uint8_t> data;
    bit length;
    bit position;
    bool isReadBeyondEnd_ = false;

  protected:
    bool isByteAligned() {
        return bit(position).get() % 8 == 0;
    }

  public:
    MemoryInputStream(const std::vector<uint8_t>& data, bit length = bit(-1), bit position = bit(0)) :
        data(data),
        length(length == bit(-1) ? bit(data.size() * 8) : length),
        position(position)
    {
        assert(bit(0) <= this->length);
        assert(bit(0) <= position && position <= this->length);
    }

    MemoryInputStream(const uint8_t *buffer, bit length, bit position = bit(0)) :
        data(buffer, buffer + byte(length).get()),
        length(length),
        position(position)
    {
        assert(bit(0) <= this->length);
        assert(bit(0) <= position && position <= this->length);
    }

    /** @name Stream querying functions */
    //@{
    bool isReadBeyondEnd() const { return isReadBeyondEnd_; }

    bit getLength() const { return length; }

    bit getRemainingLength() const { return length - position; }

    bit getPosition() const { return position; }

    const std::vector<uint8_t>& getData() const { return data; }

    void copyData(std::vector<bool>& result, bit offset = bit(0), bit length = bit(-1)) const {
        size_t end = bit(length == bit(-1) ? this->length : offset + length).get();
        for (size_t i = bit(offset).get(); i < end; i++) {
            size_t byteIndex = i / 8;
            size_t bitIndex = i % 8;
            uint8_t byte = data.at(byteIndex);
            uint8_t mask = 1 << (7 - bitIndex);
            bool bit = byte & mask;
            result.push_back(bit);
        }
    }

    void copyData(std::vector<uint8_t>& result, byte offset = byte(0), byte length = byte(-1)) const {
        auto end = length == byte(-1) ? byte(data.size()) : offset + length;
        result.insert(result.begin(), data.begin() + byte(offset).get(), data.begin() + byte(end).get());
    }
    //@}

    /** @name Stream updating functions */
    //@{
    void seek(bit position) {
        assert(bit(0) <= position && position <= length);
        this->position = position;
    }
    //@}

    /** @name Bit streaming functions */
    //@{
    bool readBit() {
        if (position == length) {
            isReadBeyondEnd_ = true;
            return false;
        }
        else {
            size_t i = bit(position).get();
            size_t byteIndex = i / 8;
            size_t bitIndex = i % 8;
            position += bit(1);
            uint8_t byte = data.at(byteIndex);
            uint8_t mask = 1 << (7 - bitIndex);
            return byte & mask;
        }
    }

    bool readBitRepeatedly(bool value, size_t count) {
        bool success = true;
        for (size_t i = 0; i < count; i++)
            success &= (value == readBit());
        return success;
    }

    bit readBits(std::vector<bool>& bits, bit length) {
        size_t i;
        for (i = 0; i < bit(length).get(); i++) {
            if (isReadBeyondEnd_)
                break;
            bits.push_back(readBit());
        }
        return bit(i);
    }
    //@}

    /** @name Byte streaming functions */
    //@{
    uint8_t readByte() {
        assert(isByteAligned());
        if (position + byte(1) > length) {
            isReadBeyondEnd_ = true;
            position = length;
            return 0;
        }
        else {
            uint8_t result = data[byte(position).get()];
            position += byte(1);
            return result;
        }
    }

    bool readByteRepeatedly(uint8_t value, size_t count) {
        bool success = true;
        for (size_t i = 0; i < count; i++)
            success &= (value == readByte());
        return success;
    }

    byte readBytes(std::vector<uint8_t>& bytes, byte length) {
        assert(isByteAligned());
        if (position + length > this->length) {
            length = this->length - position;
            isReadBeyondEnd_ = true;
        }
        bytes.insert(bytes.end(), data.begin() + byte(position).get(), data.begin() + byte(position + length).get());
        position += length;
        return length;
    }

    byte readBytes(uint8_t *buffer, byte length) {
        assert(isByteAligned());
        if (position + length > this->length) {
            length = this->length - position;
            isReadBeyondEnd_ = true;
        }
        std::copy(data.begin() + byte(position).get(), data.begin() + byte(position + length).get(), buffer);
        position += length;
        return length;
    }
    //@}

    /** @name Basic type streaming functions */
    //@{
    uint8_t readUint8() {
        return readByte();
    }

    uint16_t readUint16Be() {
        uint16_t value = 0;
        value |= ((uint16_t)(readByte()) << 8);
        value |= ((uint16_t)(readByte()) << 0);
        return value;
    }

    uint16_t readUint16Le() {
        uint16_t value = 0;
        value |= ((uint16_t)(readByte()) << 0);
        value |= ((uint16_t)(readByte()) << 8);
        return value;
    }

    uint32_t readUint32Be() {
        uint32_t value = 0;
        value |= ((uint32_t)(readByte()) << 24);
        value |= ((uint32_t)(readByte()) << 16);
        value |= ((uint32_t)(readByte()) << 8);
        value |= ((uint32_t)(readByte()) << 0);
        return value;
    }

    uint32_t readUint32Le() {
        uint32_t value = 0;
        value |= ((uint32_t)(readByte()) << 0);
        value |= ((uint32_t)(readByte()) << 8);
        value |= ((uint32_t)(readByte()) << 16);
        value |= ((uint32_t)(readByte()) << 24);
        return value;
    }

    uint64_t readUint64Be() {
        uint64_t value = 0;
        value |= ((uint64_t)(readByte()) << 56);
        value |= ((uint64_t)(readByte()) << 48);
        value |= ((uint64_t)(readByte()) << 40);
        value |= ((uint64_t)(readByte()) << 32);
        value |= ((uint64_t)(readByte()) << 24);
        value |= ((uint64_t)(readByte()) << 16);
        value |= ((uint64_t)(readByte()) << 8);
        value |= ((uint64_t)(readByte()) << 0);
        return value;
    }

    uint64_t readUint64Le() {
        uint64_t value = 0;
        value |= ((uint64_t)(readByte()) << 0);
        value |= ((uint64_t)(readByte()) << 8);
        value |= ((uint64_t)(readByte()) << 16);
        value |= ((uint64_t)(readByte()) << 24);
        value |= ((uint64_t)(readByte()) << 32);
        value |= ((uint64_t)(readByte()) << 40);
        value |= ((uint64_t)(readByte()) << 48);
        value |= ((uint64_t)(readByte()) << 56);
        return value;
    }
    //@}

    /** @name INET specific type streaming functions */
    //@{
    MACAddress readMACAddress() {
        MACAddress address;
        for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
            address.setAddressByte(i, readByte());
        return address;
    }

    IPv4Address readIPv4Address() {
        return IPv4Address(readUint32Be());
    }

    IPv6Address readIPv6Address() {
        uint32_t d[4];
        for (auto & element : d)
            element = readUint32Be();
        return IPv6Address(d[0], d[1], d[2], d[3]);
    }
    //@}
};

} // namespace

#endif // #ifndef __INET_MEMORYINPUTSTREAM_H_

