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
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

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
    b length;
    /**
     * The position of the next bit that will be read measured in bits.
     */
    b position;
    /**
     * This flag indicates if the stream has been read beyond the end of data.
     */
    bool isReadBeyondEnd_ = false;

  protected:
    bool isByteAligned() {
        return b(position).get() % 8 == 0;
    }

  public:
    MemoryInputStream(const std::vector<uint8_t>& data, b length = b(-1), b position = b(0)) :
        data(data),
        length(length == b(-1) ? b(data.size() * 8) : length),
        position(position)
    {
        assert(b(0) <= this->length);
        assert(b(0) <= position && position <= this->length);
    }

    MemoryInputStream(const uint8_t *buffer, b length, b position = b(0)) :
        data(buffer, buffer + B(length).get()),
        length(length),
        position(position)
    {
        assert(b(0) <= this->length);
        assert(b(0) <= position && position <= this->length);
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
    b getLength() const { return length; }

    /**
     * Returns the remaining unread length of the stream measured in bits.
     */
    b getRemainingLength() const { return length - position; }

    /**
     * Returns the current read position of the stream measured in bits.
     */
    b getPosition() const { return position; }

    const std::vector<uint8_t>& getData() const { return data; }

    void copyData(std::vector<bool>& result, b offset = b(0), b length = b(-1)) const {
        size_t end = b(length == b(-1) ? this->length : offset + length).get();
        for (size_t i = b(offset).get(); i < end; i++) {
            size_t byteIndex = i / 8;
            size_t bitIndex = i % 8;
            uint8_t byte = data.at(byteIndex);
            uint8_t mask = 1 << (7 - bitIndex);
            bool bit = byte & mask;
            result.push_back(bit);
        }
    }

    void copyData(std::vector<uint8_t>& result, B offset = B(0), B length = B(-1)) const {
        auto end = length == B(-1) ? B(data.size()) : offset + length;
        assert(b(0) <= offset && offset <= B(data.size()));
        assert(b(0) <= end && end <= B(data.size()));
        assert(offset <= end);
        result.insert(result.begin(), data.begin() + B(offset).get(), data.begin() + B(end).get());
    }
    //@}

    /** @name Stream updating functions */
    //@{
    /**
     * Updates the read position of the stream.
     */
    void seek(b position) {
        assert(b(0) <= position && position <= length);
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
            size_t i = b(position).get();
            size_t byteIndex = i / 8;
            size_t bitIndex = i % 8;
            position += b(1);
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
    b readBits(std::vector<bool>& bits, b length) {
        b i;
        for (i = b(0); i < length; i++) {
            if (isReadBeyondEnd_)
                break;
            bits.push_back(readBit());
        }
        return b(i);
    }
    //@}

    /** @name Byte streaming functions */
    //@{
    /**
     * Reads a byte at the current position of the stream in MSB to LSB bit order.
     */
    uint8_t readByte() {
        if (position + B(1) > length) {
            isReadBeyondEnd_ = true;
            position = length;
            return 0;
        }
        else {
            uint8_t result;
            if (isByteAligned())
                result = data[B(position).get()];
            else {
                int l1 = b(position).get() % 8;
                int l2 = 8 - l1;
                result = data[B(position - b(l1)).get()] << l1;
                result |= data[B(position + b(l2)).get()] >> l2;
            }
            position += B(1);
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
    B readBytes(std::vector<uint8_t>& bytes, B length) {
        assert(isByteAligned());
        if (position + length > this->length) {
            length = this->length - position;
            isReadBeyondEnd_ = true;
        }
        auto end = position + length;
        assert(b(0) <= position && position <= B(data.size()));
        assert(b(0) <= end && end <= B(data.size()));
        assert(position <= end);
        bytes.insert(bytes.end(), data.begin() + B(position).get(), data.begin() + B(end).get());
        position += length;
        return length;
    }

    /**
     * Reads a sequence of bytes at the current position of the stream keeping
     * the original byte order in MSB to LSB bit order.
     */
    B readBytes(uint8_t *buffer, B length) {
        assert(isByteAligned());
        if (position + length > this->length) {
            length = this->length - position;
            isReadBeyondEnd_ = true;
        }
        std::copy(data.begin() + B(position).get(), data.begin() + B(position + length).get(), buffer);
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
        value |= (static_cast<uint16_t>(readByte()) << 8);
        value |= (static_cast<uint16_t>(readByte()) << 0);
        return value;
    }

    /**
     * Reads a 16 bit unsigned integer at the current position of the stream in
     * little endian byte order and MSB to LSB bit order.
     */
    uint16_t readUint16Le() {
        uint16_t value = 0;
        value |= (static_cast<uint16_t>(readByte()) << 0);
        value |= (static_cast<uint16_t>(readByte()) << 8);
        return value;
    }

    /**
     * Reads a 24 bit unsigned integer at the current position of the stream in
     * big endian byte order and MSB to LSB bit order.
     */
    uint32_t readUint24Be() {
        uint32_t value = 0;
        value |= (static_cast<uint32_t>(readByte()) << 16);
        value |= (static_cast<uint32_t>(readByte()) << 8);
        value |= (static_cast<uint32_t>(readByte()) << 0);
        return value;
    }

    /**
     * Reads a 24 bit unsigned integer at the current position of the stream in
     * little endian byte order and MSB to LSB bit order.
     */
    uint32_t readUint24Le() {
        uint32_t value = 0;
        value |= (static_cast<uint32_t>(readByte()) << 0);
        value |= (static_cast<uint32_t>(readByte()) << 8);
        value |= (static_cast<uint32_t>(readByte()) << 16);
        return value;
    }

    /**
     * Reads a 32 bit unsigned integer at the current position of the stream in
     * big endian byte order and MSB to LSB bit order.
     */
    uint32_t readUint32Be() {
        uint32_t value = 0;
        value |= (static_cast<uint32_t>(readByte()) << 24);
        value |= (static_cast<uint32_t>(readByte()) << 16);
        value |= (static_cast<uint32_t>(readByte()) << 8);
        value |= (static_cast<uint32_t>(readByte()) << 0);
        return value;
    }

    /**
     * Reads a 32 bit unsigned integer at the current position of the stream in
     * little endian byte order and MSB to LSB bit order.
     */
    uint32_t readUint32Le() {
        uint32_t value = 0;
        value |= (static_cast<uint32_t>(readByte()) << 0);
        value |= (static_cast<uint32_t>(readByte()) << 8);
        value |= (static_cast<uint32_t>(readByte()) << 16);
        value |= (static_cast<uint32_t>(readByte()) << 24);
        return value;
    }

    /**
     * Reads a 64 bit unsigned integer at the current position of the stream in
     * big endian byte order and MSB to LSB bit order.
     */
    uint64_t readUint64Be() {
        uint64_t value = 0;
        value |= (static_cast<uint64_t>(readByte()) << 56);
        value |= (static_cast<uint64_t>(readByte()) << 48);
        value |= (static_cast<uint64_t>(readByte()) << 40);
        value |= (static_cast<uint64_t>(readByte()) << 32);
        value |= (static_cast<uint64_t>(readByte()) << 24);
        value |= (static_cast<uint64_t>(readByte()) << 16);
        value |= (static_cast<uint64_t>(readByte()) << 8);
        value |= (static_cast<uint64_t>(readByte()) << 0);
        return value;
    }

    /**
     * Reads a 64 bit unsigned integer at the current position of the stream in
     * little endian byte order and MSB to LSB bit order.
     */
    uint64_t readUint64Le() {
        uint64_t value = 0;
        value |= (static_cast<uint64_t>(readByte()) << 0);
        value |= (static_cast<uint64_t>(readByte()) << 8);
        value |= (static_cast<uint64_t>(readByte()) << 16);
        value |= (static_cast<uint64_t>(readByte()) << 24);
        value |= (static_cast<uint64_t>(readByte()) << 32);
        value |= (static_cast<uint64_t>(readByte()) << 40);
        value |= (static_cast<uint64_t>(readByte()) << 48);
        value |= (static_cast<uint64_t>(readByte()) << 56);
        return value;
    }
    //@}

    /** @name INET specific type streaming functions */
    //@{
    /**
     * Reads a MAC address at the current position of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    MacAddress readMacAddress() {
        MacAddress address;
        for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
            address.setAddressByte(i, readByte());
        return address;
    }

    /**
     * Reads an Ipv4 address at the current position of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    Ipv4Address readIpv4Address() {
        return Ipv4Address(readUint32Be());
    }

    /**
     * Reads an Ipv6 address at the current position of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    Ipv6Address readIpv6Address() {
        uint32_t d[4];
        for (auto & element : d)
            element = readUint32Be();
        return Ipv6Address(d[0], d[1], d[2], d[3]);
    }
    //@}

    /** @name other useful streaming functions */
    //@{
    /**
     * Reads a string from the current position until a zero.
     */
    std::string readString() {
        std::vector<uint8_t> data;
        while (uint8_t b = readByte())
            data.push_back(b);
        return std::string(data.begin(), data.end());
    }

    /**
     * Reads n bits of a 64 bit unsigned integer at the current position of the
     * stream in big endian byte order and MSB to LSB bit order.
     */
    uint64_t readNBitsToUint64Be(uint8_t n) {
        if (n == 0 || n > 64)
            throw cRuntimeError("Can not read 0 bit or more than 64 bits.");
        uint64_t mul = 1 << (n - 1);
        uint64_t num = 0;
        for (int i = 0; i < n; ++i) {
            if (readBit())
                num |= mul;
            mul >>= 1;
        }
        return num;
    }

    /**
     * Reads a SimTime value at the current position from the next 9 bytes.
     */
    SimTime readSimTime() {
        uint64_t raw = readUint64Be();
        SimTimeUnit unit = static_cast<SimTimeUnit>(-1 * readByte());
        return SimTime(raw, unit);
    }
    //@}
};

} // namespace inet

#endif // #ifndef __INET_MEMORYINPUTSTREAM_H_

