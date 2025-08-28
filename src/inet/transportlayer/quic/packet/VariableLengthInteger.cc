//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "VariableLengthInteger.h"

namespace inet {
namespace quic {

size_t getVariableLengthIntegerSize(VariableLengthInteger i) {
    if (i < (1ull << 6)) {
        return 1;
    }
    if (i < (1ull << 14)) {
        return 2;
    }
    if (i < (1ull << 30)) {
        return 4;
    }
    return 8;
}

size_t getEncodedVariableLengthIntegerSize(uint8_t firstByte)
{
    return (1 << (firstByte >> 6));
}

void serializeVariableLengthInteger(MemoryOutputStream& stream, VariableLengthInteger value)
{
    switch (getVariableLengthIntegerSize(value)) {
        case 1:
            // 0-6 bit length, "00" bits identify length
            stream.writeByte(value);
            break;
        case 2:
            // 7-14 bit length, "01" bits identify length
            stream.writeByte(0x40 | (value >> 8));
            stream.writeByte(value & 0xFF);
            break;
        case 4:
            // 15-30 bit length, "10" bits identify length
            stream.writeByte(0x80 | (value >> 24));
            stream.writeByte((value >> 16) & 0xFF);
            stream.writeByte((value >> 8) & 0xFF);
            stream.writeByte(value & 0xFF);
            break;
        case 8:
            // 31-62 bit length, "11" bits identify length
            stream.writeByte(0xC0 | (value >> 56));
            stream.writeByte((value >> 48) & 0xFF);
            stream.writeByte((value >> 40) & 0xFF);
            stream.writeByte((value >> 32) & 0xFF);
            stream.writeByte((value >> 24) & 0xFF);
            stream.writeByte((value >> 16) & 0xFF);
            stream.writeByte((value >> 8) & 0xFF);
            stream.writeByte(value & 0xFF);
            break;
    }
}

VariableLengthInteger deserializeVariableLengthInteger(MemoryInputStream& stream)
{
    uint8_t firstByte = stream.readByte();
    uint8_t prefix = firstByte >> 6;
    uint64_t value = 0;

    switch (prefix) {
        case 0: // "00" bits identify length of 1 bytes
            value = firstByte;
            break;
        case 1: // "01" bits identify length of 2 bytes
            value = ((uint64_t)(firstByte & 0x3F) << 8);
            value |= stream.readByte();
            break;
        case 2: // "10" bits identify length of 4 bytes
            value = ((uint64_t)(firstByte & 0x3F) << 24);
            value |= ((uint64_t)stream.readByte() << 16);
            value |= ((uint64_t)stream.readByte() << 8);
            value |= stream.readByte();
            break;
        case 3: // "11" bits identify length of 8 bytes
            value = ((uint64_t)(firstByte & 0x3F) << 56);
            value |= ((uint64_t)stream.readByte() << 48);
            value |= ((uint64_t)stream.readByte() << 40);
            value |= ((uint64_t)stream.readByte() << 32);
            value |= ((uint64_t)stream.readByte() << 24);
            value |= ((uint64_t)stream.readByte() << 16);
            value |= ((uint64_t)stream.readByte() << 8);
            value |= stream.readByte();
            break;
    }

    return value;
}

VariableLengthInteger decodeVariableLengthInteger(const uint8_t *src, const uint8_t *src_end, size_t *consumedBytes)
{
    if (src >= src_end)
        throw cRuntimeError("Not enough data to decode VariableLengthInteger");

    uint8_t firstByte = *src;
    uint8_t prefix = firstByte >> 6;
    size_t length = (1 << prefix); // 1, 2, 4, or 8 bytes
    if (src + length > src_end)
        throw cRuntimeError("Not enough data to decode VariableLengthInteger");

    VariableLengthInteger value = 0;
    switch (length) {
        case 1:
            value = firstByte;
            break;
        case 2:
            value = ((uint64_t)(firstByte & 0x3F) << 8);
            value |= *(++src);
            break;
        case 4:
            value = ((uint64_t)(firstByte & 0x3F) << 24);
            value |= ((uint64_t)(*(++src)) << 16);
            value |= ((uint64_t)(*(++src)) << 8);
            value |= *(++src);
            break;
        case 8:
            value = ((uint64_t)(firstByte & 0x3F) << 56);
            value |= ((uint64_t)(*(++src)) << 48);
            value |= ((uint64_t)(*(++src)) << 40);
            value |= ((uint64_t)(*(++src)) << 32);
            value |= ((uint64_t)(*(++src)) << 24);
            value |= ((uint64_t)(*(++src)) << 16);
            value |= ((uint64_t)(*(++src)) << 8);
            value |= *(++src);
            break;
    }

    if (consumedBytes)
        *consumedBytes = length;
    return value;
}

}  // namespace quic
}  // namespace inet

