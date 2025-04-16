//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

//
// Common checksum types.
//
enum ChecksumType
{
    CHECKSUM_TYPE_UNDEFINED = -1;
    CHECKSUM_INTERNET = 1;    // "Internet checksum" used in TCP/IP
    CHECKSUM_CRC16_IBM = 3;   // CRC-16-IBM, also called CRC-16-ANSI or CRC-16-ISO
    CHECKSUM_CRC16_CCITT = 4; // CRC-16-CCITT
    CHECKSUM_ETHERNET_FCS = 5;   // Ethernet and WiFi FCS (byte-swapped version of CRC32 a.k.a. CRC-32/ISO-HDLC)
    CHECKSUM_CRC32C = 6;      // CRC-32C (Castagnoli)
}

cplusplus {{
INET_API ChecksumType parseChecksumType(const char *checksumType);
INET_API int getChecksumSizeInBytes(ChecksumType type);
INET_API uint64_t computeChecksum(const unsigned char *buf, size_t bufsize, ChecksumType checksumType);
inline uint64_t computeChecksum(const std::vector<uint8_t>& vec, ChecksumType checksumType) { return computeChecksum(vec.data(), vec.size(), checksumType); }
}}
