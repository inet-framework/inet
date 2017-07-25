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

#ifndef __INET_MEMORYOUTPUTSTREAM_H_
#define __INET_MEMORYOUTPUTSTREAM_H_

#include "inet/common/Units.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"

namespace inet {

using namespace units::values;

/**
 * This class provides an efficient in memory bit output stream. The stream
 * provides a set of write functions that write data to the end of the stream.
 * Most functions are implemented in the header to allow inlining.
 */
// TODO: allow arbitrary mixed bit/byte writes
// TODO: add parameter checks
// TODO: review efficiency
class INET_API MemoryOutputStream {
  protected:
    /**
     * This vector contains the bits that were written to this stream so far.
     * The first bit of the bit stream is stored in the most significant bit
     * of the first byte. For the longest possible bit stream given the same
     * number of bytes, the last bit of the bit stream is stored in the least
     * significant bit of the last byte. In other cases some of the lower bits
     * of the last byte are not used.
     */
    std::vector<uint8_t> data;
    /**
     * The length of the bit stream measured in bits.
     */
    bit length;

  protected:
    bool isByteAligned() {
        return bit(length).get() % 8 == 0;
    }

  public:
    MemoryOutputStream(bit initialCapacity = byte(64)) :
        length(bit(0))
    {
        data.reserve((bit(initialCapacity).get() + 7) >> 3);
    }

    /** @name Stream querying functions */
    //@{
    /**
     * Returns the length of the bit stream measured in bits.
     */
    bit getLength() const { return length; }

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

    /** @name Bit streaming functions */
    //@{
    /**
     * Writes a bit to the end of the stream.
     */
    void writeBit(bool value) {
        size_t i = bit(length).get();
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;
        if (bitIndex == 0)
            data.push_back(0);
        if (value)
            data[byteIndex] |= 1 << (7 - bitIndex);
        length += bit(1);
    }

    /**
     * Writes the same bit repeatedly to the end of the stream.
     */
    void writeBitRepeatedly(bool value, size_t count) {
        for (size_t i = 0; i < count; i++)
            writeBit(value);
    }

    /**
     * Writes a sequence of bits to the end of the stream keeping the original
     * bit order.
     */
    void writeBits(const std::vector<bool>& bits, bit offset = bit(0), bit length = bit(-1)) {
        auto end = length == bit(-1) ? bits.size() : bit(offset + length).get();
        for (size_t i = bit(offset).get(); i < end; i++)
            writeBit(bits.at(i));
    }
    //@}

    /** @name Byte streaming functions */
    //@{
    /**
     * Writes a byte to the end of the stream in MSB to LSB bit order.
     */
    void writeByte(uint8_t value) {
        assert(isByteAligned());
        data.push_back(value);
        length += byte(1);
    }

    /**
     * Writes the same byte repeatedly to the end of the stream in MSB to LSB
     * bit order.
     */
    void writeByteRepeatedly(uint8_t value, size_t count) {
        for (size_t i = 0; i < count; i++)
            writeByte(value);
    }

    /**
     * Writes a sequence of bytes to the end of the stream keeping the original
     * byte order and in MSB to LSB bit order.
     */
    void writeBytes(const std::vector<uint8_t>& bytes, byte offset = byte(0), byte length = byte(-1)) {
        assert(isByteAligned());
        auto end = length == byte(-1) ? byte(bytes.size()) : offset + length;
        data.insert(data.end(), bytes.begin() + byte(offset).get(), bytes.begin() + byte(end).get());
        this->length += end - offset;
    }

    /**
     * Writes a sequence of bytes to the end of the stream keeping the original
     * byte order and in MSB to LSB bit order.
     */
    void writeBytes(uint8_t *buffer, byte length) {
        assert(isByteAligned());
        data.insert(data.end(), buffer, buffer + byte(length).get());
        this->length += length;
    }
    //@}

    /** @name Basic type streaming functions */
    //@{
    /**
     * Writes an 8 bit unsigned integer to the end of the stream in MSB to LSB
     * bit order.
     */
    void writeUint8(uint8_t value) {
        writeByte(value);
    }

    /**
     * Writes a 16 bit unsigned integer to the end of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint16Be(uint16_t value) {
        writeByte((uint8_t)(value >> 8));
        writeByte((uint8_t)(value >> 0));
    }

    /**
     * Writes a 16 bit unsigned integer to the end of the stream in little endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint16Le(uint16_t value) {
        writeByte((uint8_t)(value >> 0));
        writeByte((uint8_t)(value >> 8));
    }

    /**
     * Writes a 32 bit unsigned integer to the end of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint32Be(uint32_t value) {
        writeByte((uint8_t)(value >> 24));
        writeByte((uint8_t)(value >> 16));
        writeByte((uint8_t)(value >> 8));
        writeByte((uint8_t)(value >> 0));
    }

    /**
     * Writes a 32 bit unsigned integer to the end of the stream in little endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint32Le(uint32_t value) {
        writeByte((uint8_t)(value >> 0));
        writeByte((uint8_t)(value >> 8));
        writeByte((uint8_t)(value >> 16));
        writeByte((uint8_t)(value >> 24));
    }

    /**
     * Writes a 64 bit unsigned integer to the end of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint64Be(uint64_t value) {
        writeByte((uint8_t)(value >> 56));
        writeByte((uint8_t)(value >> 48));
        writeByte((uint8_t)(value >> 40));
        writeByte((uint8_t)(value >> 32));
        writeByte((uint8_t)(value >> 24));
        writeByte((uint8_t)(value >> 16));
        writeByte((uint8_t)(value >> 8));
        writeByte((uint8_t)(value >> 0));
    }

    /**
     * Writes a 64 bit unsigned integer to the end of the stream in little endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint64Le(uint64_t value) {
        writeByte((uint8_t)(value >> 0));
        writeByte((uint8_t)(value >> 8));
        writeByte((uint8_t)(value >> 16));
        writeByte((uint8_t)(value >> 24));
        writeByte((uint8_t)(value >> 32));
        writeByte((uint8_t)(value >> 40));
        writeByte((uint8_t)(value >> 48));
        writeByte((uint8_t)(value >> 56));
    }
    //@}

    /** @name INET specific type streaming functions */
    //@{
    /**
     * Writes a MAC address to the end of the stream in big endian byte order
     * and MSB to LSB bit order.
     */
    void writeMACAddress(MACAddress address) {
        for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
            writeByte(address.getAddressByte(i));
    }

    /**
     * Writes an IPv4 address to the end of the stream in big endian byte order
     * and MSB to LSB bit order.
     */
    void writeIPv4Address(IPv4Address address) {
        writeUint32Be(address.getInt());
    }

    /**
     * Writes an IPv6 address to the end of the stream in big endian byte order
     * and MSB to LSB bit order.
     */
    void writeIPv6Address(IPv6Address address) {
        for (int i = 0; i < 4; i++)
            writeUint32Be(address.words()[i]);
    }
    //@}
};

} // namespace

#endif // #ifndef __INET_MEMORYOUTPUTSTREAM_H_

