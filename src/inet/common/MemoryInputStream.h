//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MEMORYINPUTSTREAM_H
#define __INET_MEMORYINPUTSTREAM_H

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
// TODO allow arbitrary mixed bit/byte reads
// TODO add parameter checks
// TODO review efficiency
class INET_API MemoryInputStream
{
  protected:
    /**
     * This vector contains the bits that are read from this stream. The first
     * bit of the bit stream is stored in the most significant bit of the first
     * byte. For the longest possible bit stream given the same number of bytes,
     * the last bit of the bit stream is stored in the least significant bit of
     * the last byte. In other cases some of the lower bits of the last byte are
     * not used.
     */
    const std::vector<uint8_t> data;
    /**
     * The length of the bit stream measured in bits.
     */
    b length;
    /**
     * The position of the next bit that will be read measured in bits.
     * When this value larger than `length`, the stream has been read beyond the end of data.
     */
    b position;

  protected:
    bool isByteAligned() const {
        return (position.get<b>() & 7) == 0;
    }

  public:
    MemoryInputStream(const std::vector<uint8_t>& data, b length = b(-1), b position = b(0)) :
        data(data),
        length(length == b(-1) ? b(data.size() * 8) : length),
        position(position)
    {
        ASSERT(b(0) <= this->length);
        ASSERT(b(0) <= position && position <= this->length);
    }

    MemoryInputStream(const uint8_t *buffer, b length, b position = b(0)) :
        data(buffer, buffer + length.get<B>()),
        length(length),
        position(position)
    {
        ASSERT(b(0) <= this->length);
        ASSERT(b(0) <= position && position <= this->length);
    }

    /** @name Stream querying functions */
    //@{
    /**
     * Returns true if a read operation ever read beyond the end of the stream.
     */
    bool isReadBeyondEnd() const { return position > length; }

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

    /** @name Stream updating functions */
    //@{
    /**
     * Updates the read position of the stream.
     */
    void seek(b newPosition) {
        ASSERT(b(0) <= newPosition && newPosition <= length);
        position = newPosition;
    }
    //@}

    /** @name Bit streaming functions */
    //@{
    /**
     * Reads a bit at the current position of the stream.
     */
    bool readBit() {
        bool value = false;
        if (position < length) {
            size_t i = position.get<b>();
            size_t byteIndex = i >> 3;
            uint8_t bitOffset = i & 7;
            uint8_t byte = data.at(byteIndex);
            uint8_t mask = 1 << (7 - bitOffset);
            value = (byte & mask) != 0;
        }
        position += b(1);
        return value;
    }

    /**
     * Reads the same bit repeatedly at the current position of the stream.
     */
    bool readBitRepeatedly(bool value, size_t count) {
        bool success = position <= length;
        for (size_t i = 0; i < count; i++)
            success &= (value == readBit());
        return success;
    }

    /**
     * Reads a sequence of bits at the current position of the stream keeping
     * the original bit order.
     */
    b readBits(std::vector<bool>& bits, b length) {
        b len = (position + length <= this->length) ? length : (position < this->length) ? this->length - position : b(0);
        for (b i = b(0); i < len; i++)
            bits.push_back(readBit());
        if (len < length)
            position += length - len;
        return b(len);
    }
    //@}

    /** @name Byte streaming functions */
    //@{
    /**
     * Reads a byte at the current position of the stream in MSB to LSB bit order.
     */
    uint8_t readByte() {
        uint8_t value = 0;
        if (position < this->length) {
            size_t i = position.get<b>();
            size_t byteIndex = i >> 3;
            uint8_t bitOffset = i & 7;
            value = data.at(byteIndex++) << bitOffset;
            if (bitOffset != 0 && byteIndex < data.size())
                value |= (data.at(byteIndex) >> (8 - bitOffset));
        }
        position += B(1);
        return value;
    }

    /**
     * Reads the same byte repeatedly at the current position of the stream in
     * MSB to LSB bit order.
     */
    bool readByteRepeatedly(uint8_t value, size_t count) {
        b endPos = position + B(count);
        bool success = true;
        for (size_t i = 0; i < count && position < this->length; i++)
            success &= (value == readByte());
        position = endPos;
        return success && position < this->length;
    }

    /**
     * Reads a sequence of bytes at the current position of the stream keeping
     * the original byte order in MSB to LSB bit order.
     */
    B readBytes(std::vector<uint8_t>& bytes, B length) {
        B readLen(0);
        if (position < this->length && length > B(0)) {
            size_t i = position.get<b>();
            size_t byteIndex = (i+7) >> 3;
            uint8_t bitOffset = i & 7;
            size_t endByteIndex = std::min(byteIndex + length.get<B>(), data.size());
            readLen = B(endByteIndex - byteIndex);
            if (bitOffset == 0) {
                bytes.insert(bytes.end(), data.begin() + byteIndex, data.begin() + endByteIndex);
            }
            else {
                bytes.reserve(bytes.size() + B(readLen).get() + 1);
                for (size_t i = byteIndex; i < endByteIndex; i++)
                    bytes.push_back(data.at(i-1) << bitOffset | data.at(i) >> (8 - bitOffset));
                if (readLen < length && position + readLen < this->length) {
                    bytes.push_back(data.at(endByteIndex-1) << bitOffset);
                    readLen += B(1);
                }
            }
        }
        position += length;
        return readLen;
    }

    /**
     * Reads a sequence of bytes at the current position of the stream keeping
     * the original byte order in MSB to LSB bit order.
     */
    B readBytes(uint8_t *buffer, B length) {
        B readLen(0);
        if (position < this->length && length > B(0)) {
            size_t i = position.get<b>();
            size_t byteIndex = (i+7) >> 3;
            uint8_t bitOffset = i & 7;
            size_t endByteIndex = std::min(byteIndex + length.get<B>(), data.size());
            readLen = B(endByteIndex - byteIndex);
            if (bitOffset == 0) {
                std::copy(data.begin() + byteIndex, data.begin() + endByteIndex, buffer);
            }
            else {
                for (size_t i = byteIndex; i < endByteIndex; i++)
                    *buffer++ = data.at(i-1) << bitOffset | data.at(i) >> (8 - bitOffset);
                if (readLen < length && position + readLen < this->length) {
                    *buffer++ = data.at(endByteIndex-1) << bitOffset;
                    readLen += B(1);
                }
            }
        }
        position += length;
        return readLen;
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
     * Reads a 48 bit unsigned integer at the current position of the stream in
     * big endian byte order and MSB to LSB bit order.
     */
    uint64_t readUint48Be() {
        uint64_t value = 0;
        value |= (static_cast<uint64_t>(readByte()) << 40);
        value |= (static_cast<uint64_t>(readByte()) << 32);
        value |= (static_cast<uint64_t>(readByte()) << 24);
        value |= (static_cast<uint64_t>(readByte()) << 16);
        value |= (static_cast<uint64_t>(readByte()) << 8);
        value |= (static_cast<uint64_t>(readByte()) << 0);
        return value;
    }

    /**
     * Reads a 48 bit unsigned integer at the current position of the stream in
     * little endian byte order and MSB to LSB bit order.
     */
    uint64_t readUint48Le() {
        uint64_t value = 0;
        value |= (static_cast<uint64_t>(readByte()) << 0);
        value |= (static_cast<uint64_t>(readByte()) << 8);
        value |= (static_cast<uint64_t>(readByte()) << 16);
        value |= (static_cast<uint64_t>(readByte()) << 24);
        value |= (static_cast<uint64_t>(readByte()) << 32);
        value |= (static_cast<uint64_t>(readByte()) << 40);
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
        return MacAddress(readUint48Be());
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
        for (auto& element : d)
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
        std::string str;
        while (uint8_t b = readByte())
            str.push_back(b);
        return str;
    }

    /**
     * Reads n bits of a 64 bit unsigned integer at the current position of the
     * stream in big endian byte order and MSB to LSB bit order.
     */
    uint64_t readNBitsToUint64Be(uint8_t n) {
        if (n == 0 || n > 64)
            throw cRuntimeError("Can not read 0 bit or more than 64 bits.");
        uint64_t mul = (uint64_t)1 << (n - 1);
        uint64_t num = 0;
        for (int i = 0; i < n; ++i) {
            if (readBit())
                num |= mul;
            mul >>= 1;
        }
        return num;
    }
    //@}
};

} // namespace inet

#endif

