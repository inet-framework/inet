//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <string.h>

#include "inet/common/checksum/ChecksumType_m.h"

namespace inet {

ChecksumType parseChecksumType(const char *checksumTypeString)
{
    if (!strcmp(checksumTypeString, "internet"))
        return CHECKSUM_INTERNET;
    else if (!strcmp(checksumTypeString, "crc8"))
        return CHECKSUM_CRC8;
    else if (!strcmp(checksumTypeString, "crc16-ibm"))
        return CHECKSUM_CRC16_IBM;
    else if (!strcmp(checksumTypeString, "crc16-ccitt"))
        return CHECKSUM_CRC16_CCITT;
    else if (!strcmp(checksumTypeString, "crc32"))
        return CHECKSUM_CRC32;
    else if (!strcmp(checksumTypeString, "crc64"))
        return CHECKSUM_CRC64;
    else
        throw cRuntimeError("Unknown checksum type '%s', allowed ones are: internet, crc8, crc16-ibm, crc16-ccitt, crc32, crc64", checksumTypeString);
}

int getChecksumSizeInBytes(ChecksumType type)
{
    switch (type) {
        case CHECKSUM_INTERNET: return 2;
        case CHECKSUM_CRC8: return 1;
        case CHECKSUM_CRC16_IBM: return 2;
        case CHECKSUM_CRC16_CCITT: return 2;
        case CHECKSUM_CRC32: return 4;
        case CHECKSUM_CRC64: return 8;
        default: throw cRuntimeError("Unknown checksum type: %d", type);
    }
}

} // namespace inet
