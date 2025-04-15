//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <string.h>

#include "inet/common/checksum/ChecksumType_m.h"

#include "inet/common/checksum/Checksum.h"

namespace inet {

ChecksumType parseChecksumType(const char *checksumTypeString)
{
    if (!strcmp(checksumTypeString, "internet"))
        return CHECKSUM_INTERNET;
    else if (!strcmp(checksumTypeString, "crc32-eth"))
        return CHECKSUM_CRC32_ETH;
    else if (!strcmp(checksumTypeString, "crc32c"))
        return CHECKSUM_CRC32C;
    else if (!strcmp(checksumTypeString, "crc16-ibm"))
        return CHECKSUM_CRC16_IBM;
    else if (!strcmp(checksumTypeString, "crc16-ccitt"))
        return CHECKSUM_CRC16_CCITT;
    else
        throw cRuntimeError("Unknown checksum type '%s', allowed ones are: internet, crc16-ibm, crc16-ccitt, crc32-eth, crc32c", checksumTypeString);
}

int getChecksumSizeInBytes(ChecksumType type)
{
    switch (type) {
        case CHECKSUM_INTERNET:
        case CHECKSUM_CRC16_IBM:
        case CHECKSUM_CRC16_CCITT:
            return 2;
        case CHECKSUM_CRC32_ETH:
        case CHECKSUM_CRC32C:
            return 4;
        default:
            throw cRuntimeError("Unknown checksum type: %d", type);
    }
}

uint64_t computeChecksum(const unsigned char *buf, size_t bufsize, ChecksumType checksumType)
{
    switch (checksumType) {
        case CHECKSUM_TYPE_UNDEFINED:
            throw cRuntimeError("Checksum type is undefined");
        case CHECKSUM_INTERNET:
            return internetChecksum(buf, bufsize);
        case CHECKSUM_CRC32_ETH:
            return ethernetCRC(buf, bufsize);
        case CHECKSUM_CRC32C:
            return crc32c(buf, bufsize);
        case CHECKSUM_CRC16_IBM:
            return crc16_ibm(buf, bufsize);
        case CHECKSUM_CRC16_CCITT:
            return crc16_ccitt(buf, bufsize);
        default:
            throw cRuntimeError("Unknown checksum type: %d", (int)checksumType);
    }
}


} // namespace inet
