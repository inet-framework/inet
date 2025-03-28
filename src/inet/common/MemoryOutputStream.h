//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MEMORYOUTPUTSTREAM_H
#define __INET_MEMORYOUTPUTSTREAM_H

#include "inet/common/Units.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

using namespace units::values;

/**
 * This class provides an efficient in memory bit output stream. The stream
 * provides a set of write functions that write data to the end of the stream.
 * Most functions are implemented in the header to allow inlining.
 */
// TODO allow arbitrary mixed bit/byte writes
// TODO add parameter checks
// TODO review efficiency
class INET_API MemoryOutputStream
{
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
    b length;

  protected:
    bool isByteAligned() const {
        return length.get<b>() % 8 == 0;
    }

  public:
    MemoryOutputStream(b initialCapacity = B(64)) :
        length(b(0))
    {
        data.reserve((initialCapacity.get<b>() + 7) >> 3);
    }

    void clear() { data.clear(); length = b(0); }

    /** @name Stream querying functions */
    //@{
    /**
     * Returns the length of the bit stream measured in bits.
     */
    b getLength() const { return length; }

    void setCapacity(b capacity) {
        data.reserve((capacity.get<b>() + 7) >> 3);
    }

    const std::vector<uint8_t>& getData() const { return data; }

    void writeData(const std::vector<uint8_t>& src, b srcOffset, b srcLength) {
        assert(srcOffset + srcLength <= B(src.size()));
        size_t srcPosInBits = srcOffset.get<b>();
        size_t srcEndPosInBits = (srcOffset + srcLength).get<b>();

        for ( ; srcPosInBits < srcEndPosInBits && ((srcPosInBits & 7) != 0); srcPosInBits++)
            writeBit(src.at(srcPosInBits >> 3) & (1 << (7 - (srcPosInBits & 7))));
        size_t remainedBytes = (srcEndPosInBits - srcPosInBits) >> 3;
        if (remainedBytes != 0) {
            writeBytes(&src.at(srcPosInBits >> 3), B(remainedBytes));
            srcPosInBits += remainedBytes << 3;
        }
        for ( ; srcPosInBits < srcEndPosInBits; srcPosInBits++)
            writeBit(src.at(srcPosInBits >> 3) & (1 << (7 - (srcPosInBits & 7))));
    }

    void copyData(std::vector<bool>& result, b offset = b(0), b length = b(-1)) const {
        size_t end = (length == b(-1) ? this->length : offset + length).get<b>();
        for (size_t i = offset.get<b>(); i < end; i++) {
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
        ASSERT(b(0) <= offset && offset <= B(data.size()));
        ASSERT(b(0) <= end && end <= B(data.size()));
        ASSERT(offset <= end);
        result.insert(result.begin(), data.begin() + offset.get<B>(), data.begin() + end.get<B>());
    }
    //@}

    /** @name Bit streaming functions */
    //@{
    /**
     * Writes a bit to the end of the stream.
     */
    void writeBit(bool value) {
        size_t i = length.get<b>();
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;
        if (bitIndex == 0)
            data.push_back(0);
        if (value)
            data[byteIndex] |= 1 << (7 - bitIndex);
        length += b(1);
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
    void writeBits(const std::vector<bool>& bits, b offset = b(0), b length = b(-1)) {
        ASSERT(b(0) <= offset && offset <= b(bits.size()));
        ASSERT(length == b(-1) || (b(0) <= length && offset + length <= b(bits.size())));
        auto end = length == b(-1) ? bits.size() : b(offset + length).get<b>();
        for (size_t i = offset.get<b>(); i < end; i++)
            writeBit(bits.at(i));
    }
    //@}

    /** @name Byte streaming functions */
    //@{
    /**
     * Writes a byte to the end of the stream in MSB to LSB bit order.
     */
    void writeByte(uint8_t value) {
        uint8_t bitOffset = length.get<b>() % 8;
        if (bitOffset == 0)
            data.push_back(value);
        else {
            data.back() |= value >> bitOffset;
            data.push_back(value << (8 - bitOffset));
        }
        length += B(1);
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
    void writeBytes(const std::vector<uint8_t>& bytes, B offset = B(0), B length = B(-1)) {
        auto end = length == B(-1) ? B(bytes.size()) : offset + length;
        ASSERT(b(0) <= offset && offset <= B(bytes.size()));
        ASSERT(b(0) <= end && end <= B(bytes.size()));
        ASSERT(offset <= end);
        writeBytes(bytes.data() + offset.get<B>(), end - offset);
    }

    /**
     * Writes a sequence of bytes to the end of the stream keeping the original
     * byte order and in MSB to LSB bit order.
     */
    void writeBytes(const uint8_t *buffer, B length) {
        ASSERT(buffer != nullptr);
        ASSERT(B(0) <= length);
        if (length == B(0))
            return;
        if (isByteAligned()) {
            data.insert(data.end(), buffer, buffer + length.get<B>());
            this->length += length;
        }
        else {
            for (B::value_type i = 0; i < length.get<B>(); i++)
                writeByte(buffer[i]);
        }
    }
    //@}

    /** @name Basic type streaming functions */
    //@{
    /**
     * Writes a 2 bit unsigned integer to the end of the stream in MSB to LSB
     * bit order.
     */
    void writeUint2(uint8_t value) {
        ASSERT(value >> 2 == 0);
        writeBit(value & 0x2);
        writeBit(value & 0x1);
    }

    /**
     * Writes a 4 bit unsigned integer to the end of the stream in MSB to LSB
     * bit order.
     */
    void writeUint4(uint8_t value) {
        ASSERT(value >> 4 == 0);
        writeBit(value & 0x8);
        writeBit(value & 0x4);
        writeBit(value & 0x2);
        writeBit(value & 0x1);
    }

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
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 0));
    }

    /**
     * Writes a 16 bit unsigned integer to the end of the stream in little endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint16Le(uint16_t value) {
        writeByte(static_cast<uint8_t>(value >> 0));
        writeByte(static_cast<uint8_t>(value >> 8));
    }

    /**
     * Writes a 24 bit unsigned integer to the end of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint24Be(uint32_t value) {
        ASSERT(value >> 24 == 0);
        writeByte(static_cast<uint8_t>(value >> 16));
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 0));
    }

    /**
     * Writes a 24 bit unsigned integer to the end of the stream in little endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint24Le(uint32_t value) {
        ASSERT(value >> 24 == 0);
        writeByte(static_cast<uint8_t>(value >> 0));
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 16));
    }

    /**
     * Writes a 32 bit unsigned integer to the end of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint32Be(uint32_t value) {
        writeByte(static_cast<uint8_t>(value >> 24));
        writeByte(static_cast<uint8_t>(value >> 16));
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 0));
    }

    /**
     * Writes a 32 bit unsigned integer to the end of the stream in little endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint32Le(uint32_t value) {
        writeByte(static_cast<uint8_t>(value >> 0));
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 16));
        writeByte(static_cast<uint8_t>(value >> 24));
    }

    /**
     * Writes a 48 bit unsigned integer to the end of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint48Be(uint64_t value) {
        ASSERT(value >> 48 == 0);
        writeByte(static_cast<uint8_t>(value >> 40));
        writeByte(static_cast<uint8_t>(value >> 32));
        writeByte(static_cast<uint8_t>(value >> 24));
        writeByte(static_cast<uint8_t>(value >> 16));
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 0));
    }

    /**
     * Writes a 48 bit unsigned integer to the end of the stream in little endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint48Le(uint64_t value) {
        ASSERT(value >> 48 == 0);
        writeByte(static_cast<uint8_t>(value >> 0));
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 16));
        writeByte(static_cast<uint8_t>(value >> 24));
        writeByte(static_cast<uint8_t>(value >> 32));
        writeByte(static_cast<uint8_t>(value >> 40));
    }

    /**
     * Writes a 64 bit unsigned integer to the end of the stream in big endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint64Be(uint64_t value) {
        writeByte(static_cast<uint8_t>(value >> 56));
        writeByte(static_cast<uint8_t>(value >> 48));
        writeByte(static_cast<uint8_t>(value >> 40));
        writeByte(static_cast<uint8_t>(value >> 32));
        writeByte(static_cast<uint8_t>(value >> 24));
        writeByte(static_cast<uint8_t>(value >> 16));
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 0));
    }

    /**
     * Writes a 64 bit unsigned integer to the end of the stream in little endian
     * byte order and MSB to LSB bit order.
     */
    void writeUint64Le(uint64_t value) {
        writeByte(static_cast<uint8_t>(value >> 0));
        writeByte(static_cast<uint8_t>(value >> 8));
        writeByte(static_cast<uint8_t>(value >> 16));
        writeByte(static_cast<uint8_t>(value >> 24));
        writeByte(static_cast<uint8_t>(value >> 32));
        writeByte(static_cast<uint8_t>(value >> 40));
        writeByte(static_cast<uint8_t>(value >> 48));
        writeByte(static_cast<uint8_t>(value >> 56));
    }
    //@}

    /** @name INET specific type streaming functions */
    //@{
    /**
     * Writes a MAC address to the end of the stream in big endian byte order
     * and MSB to LSB bit order.
     */
    void writeMacAddress(MacAddress address) {
        writeUint48Be(address.getInt());
    }

    /**
     * Writes an Ipv4 address to the end of the stream in big endian byte order
     * and MSB to LSB bit order.
     */
    void writeIpv4Address(Ipv4Address address) {
        writeUint32Be(address.getInt());
    }

    /**
     * Writes an Ipv6 address to the end of the stream in big endian byte order
     * and MSB to LSB bit order.
     */
    void writeIpv6Address(Ipv6Address address) {
        for (int i = 0; i < 4; i++)
            writeUint32Be(address.words()[i]);
    }
    //@}

    /** @name other useful streaming functions */
    //@{
    /**
     * Writes a zero terminated string in the order of the characters.
     */
    void writeString(std::string s) {
        writeBytes(reinterpret_cast<const uint8_t*>(s.c_str()), B(s.length()));
        writeByte(0);
    }

    /**
     * Writes n bits of a 64 bit unsigned integer to the end of the stream in big
     * endian byte order and MSB to LSB bit order.
     */
    void writeNBitsOfUint64Be(uint64_t value, uint8_t n) {
        if (n > 64)
            throw cRuntimeError("Can not write more than 64 bits.");
        uint64_t mul = (uint64_t)1 << (n - 1);
        for (int i = 0; i < n; ++i) {
            writeBit((value & mul) != 0);
            mul >>= 1;
        }
    }
    //@}
};

} // namespace inet

#endif

