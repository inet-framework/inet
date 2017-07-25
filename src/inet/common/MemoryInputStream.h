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
 * This class provides an efficient in memory bit input stream. The stream
 * provides a set of read functions that read data at the current position of
 * the stream. Most functions are implemented in the header to allow inlining.
 */
// TODO: allow arbitrary mixed bit/byte reads
// TODO: add parameter checks
// TODO: review efficiency
class INET_API MemoryInputStream {
  protected:
    /**
     * This vector contains the bits that are read from this stream. The first
     * bit of the bit stream is stored in the most significant bit of the first
     * byte. For the longest possible bit stream given the same number of bytes,
     * the last bit of the bit stream is stored in the least significant bit of
     * the last byte. In other cases some of the lower bits of the last byte are
     * not used.
     */
    std::vector<uint8_t> data;
    /**
     * The length of the bit stream measured in bits.
     */
    bit length;
    /**
     * The position of the next bit that will be read measured in bits.
     */
    bit position;
    /**
     * This flag indicates if the stream has been read beyond the end of data.
     */
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
    /**
     * Returns true if a read operation ever read beyond the end of the stream.
     */
    bool isReadBeyondEnd() const { return isReadBeyondEnd_; }

    /**
     * Returns the total length of the stream measured in bits.
     */
    bit getLength() const { return length; }

    /**
     * Returns the remaining unread length of the stream measured in bits.
     */
    bit getRemainingLength() const { return length - position; }

    /**
     * Returns the current read position of the stream measured in bits.
     */
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
    /**
     * Updates the read position of the stream.
     */
    void seek(bit position) {
        assert(bit(0) <= position && position <= length);
        this->position = position;
    }
    //@}

    /** @name Bit streaming functions */
    //@{
    /**
     * Reads a bit at the current position of the stream.
     */
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

    /**
     * Reads the same bit repeatedly at the current position of the stream.
     */
    bool readBitRepeatedly(bool value, size_t count) {
        bool success = true;
        for (size_t i = 0; i < count; i++)
            success &= (value == readBit());
        return success;
    }

    /**
     * Reads a sequence of bits at the current position of the stream keeping
     * the original bit order.
     */
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
    /**
     * Reads a byte at the current position of the stream in MSB to LSB bit order.
     */
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

    /**
     * Reads the same byte repeatedly at the current position of the stream in
     * MSB to LSB bit order.
     */
    bool readByteRepeatedly(uint8_t value, size_t count) {
        bool success = true;
        for (size_t i = 0; i < count; i++)
            success &= (value == readByte());
        return success;
    }

    /**
     * Reads a sequence of bytes at the current position of the stream keeping
     * the original byte order in MSB to LSB bit order.
     */
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

    /**
     * Reads a sequence of bytes at the current position of the stream keeping
     * the original byte order in MSB to LSB bit order.
     */
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
    /**
     * Reads a 2 bit unsigned integer at the current position of the stream in
     * MSB to LSB bit order.
     */
    uint8_t readUint2() {
        uint8_t value = 0;
        if (readBit()) value |= 0x2;
        if (readBit()) value |= 0x1;
        return value;
    }

    /**
     * Reads a 4 bit unsigned integer at the current position of the stream in
     * MSB to LSB bit order.
     */
    uint8_t readUint4() {
        uint8_t value = 0;
        if (readBit()) value |= 0x8;
        if (readBit()) value |= 0x4;
        if (readBit()) value |= 0x2;
        if (readBit()) value |= 0x1;
        return value;
    }

    /**
     * Reads an 8 bit unsigned integer at the current position of the stream in
     * MSB to LSB bit order.
     */
    uint8_t readUint8() {
        return readByte();
    }

    /**
     * Reads a 16 bit unsigned integer at the current position of the stream in
     * big endian byte order and MSB to LSB bit order.
     */
    uint16_t readUint16Be() {
        uint16_t value = 0;
        value |= ((uint16_t)(readByte()) << 8);
        value |= ((uint16_t)(readByte()) << 0);
        return value;
    }

    /**
     * Reads a 16 bit unsigned integer at the current position of the stream in
     * little endian byte order and MSB to LSB bit order.
     */
    uint16_t readUint16Le() {
        uint16_t value = 0;
        value |= ((uint16_t)(readByte()) << 0);
        value |= ((uint16_t)(readByte()) << 8);
        return value;
    }

    /**
     * Reads a 32 bit unsigned integer at the current position of the stream in
     * big endian byte order and MSB to LSB bit order.
     */
    uint32_t readUint32Be() {
        uint32_t value = 0;
        value |= ((uint32_t)(readByte()) << 24);
        value |= ((uint32_t)(readByte()) << 16);
        value |= ((uint32_t)(readByte()) << 8);
        value |= ((uint32_t)(readByte()) << 0);
        return value;
    }

    /**
     * Reads a 32 bit unsigned integer at the current position of the stream in
     * little endian byte order and MSB to LSB bit order.
     */
    uint32_t readUint32Le() {
        uint32_t value = 0;
        value |= ((uint32_t)(readByte()) << 0);
        value |= ((uint32_t)(readByte()) << 8);
        value |= ((uint32_t)(readByte()) << 16);
        value |= ((uint32_t)(readByte()) << 24);
        return value;
    }

    /**
     * Reads a 64 bit unsigned integer at the current position of the stream in
     * big endian byte order and MSB to LSB bit order.
     */
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

    /**
     * Reads a 64 bit unsigned integer at the current position of the stream in
     * little endian byte order and MSB to LSB bit order.
     */
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
    /**
     * Reads a MAC address at the current position of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    MACAddress readMACAddress() {
        MACAddress address;
        for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
            address.setAddressByte(i, readByte());
        return address;
    }

    /**
     * Reads an IPv4 address at the current position of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    IPv4Address readIPv4Address() {
        return IPv4Address(readUint32Be());
    }

    /**
     * Reads an IPv6 address at the current position of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
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

