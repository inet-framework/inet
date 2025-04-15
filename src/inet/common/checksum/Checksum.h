//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHECKSUM_H
#define __INET_CHECKSUM_H

#include "inet/common/INETDefs.h"

namespace inet {

//
// Resources:
// - CRC RevEng catalog: https://reveng.sourceforge.net/crc-catalogue/
// - https://crccalc.com/
// - https://www.sunshine2k.de/coding/javascript/crc/crc_js.html
// - https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/
//

/**
 * Computes the Internet checksum used in TCP and other internet protocols,
 * as the one's complement 16-bit sum, with the final result inverted (bit-flipped).
 */
uint16_t internetChecksum(const uint8_t *addr, size_t count, uint32_t sum = 0);

/**
 * Computes the FCS value used in 802.3 Ethernet and 802.11 Wifi as CRC32,
 * with the order of bytes reversed.
 */
uint32_t ethernetCRC(const uint8_t *buf, size_t bufsize, uint32_t crc = 0);

/**
 * Computes CRC32, a.k.a. CRC-32/ISO-HDLC.
 *
 * CRC RevEng categorization:
 * width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926 residue=0xdebb20e3 name="CRC-32/ISO-HDLC"
 */
uint32_t crc32_iso_hdlc(const uint8_t *buf, size_t bufsize, uint32_t crc = 0);

/**
 * Computes CRC32C (Castagnoli), a.k.a. CRC-32/ISCSI.
 *
 * CRC RevEng categorization:
 * width=32 poly=0x1edc6f41 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xe3069283 residue=0xb798b438 name="CRC-32/ISCSI"
 */
uint32_t crc32c(const uint8_t *buf, size_t bufsize, uint32_t crc = 0);

/**
 * Computes CRC16-IBM, a.k.a. CRC-16/ARC.
 *
 * CRC RevEng categorization:
 * width=16 poly=0x8005 init=0x0000 refin=true refout=true xorout=0x0000 check=0xbb3d residue=0x0000 name="CRC-16/ARC"
 */
uint16_t crc16_ibm(const uint8_t *buf, size_t bufsize, uint16_t crc = 0);

/**
 * Computes CRC16-CCITT, a.k.a. CRC-16/KERMIT.
 *
 * CRC RevEng categorization:
 * width=16 poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000 check=0x2189 residue=0x0000 name="CRC-16/KERMIT"
 */
uint16_t crc16_ccitt(const uint8_t *buf, size_t bufsize, uint16_t crc = 0);


/**
 * Computes the CRC-32 for an arbitrary configuration. The parameters are:
 *    poly:       The polynomial (shown non-reflected as on crccalc.com).
 *    init:       The initial CRC value.
 *    reflectIn:  If true, the algorithm works in the "reflected" (LSB-first) domain.
 *    reflectOut: If true, and not in reflected mode, the final CRC is bit-reversed.
 *    xorOut:     The final XOR value.
 *
 * These are the parameters also used by the CRC RevEng catalog (https://reveng.sourceforge.net/crc-catalogue/),
 * https://crccalc.com/, and other CRC-related sites and resources.
 */
uint32_t generic_crc32(const uint8_t *buf, size_t bufsize,
        uint32_t poly, uint32_t init, bool reflectIn, bool reflectOut, uint32_t xorOut);

/**
 * Computes a 16-bit CRC for arbitrary configurations. See generic_crc32() for documentation.
 */
uint16_t generic_crc16(const uint8_t *buf, size_t bufsize,
        uint16_t poly, uint16_t init, bool reflectIn, bool reflectOut, uint16_t xorOut);

/**
 * Computes an 8-bit CRC for arbitrary configurations. See generic_crc32() for documentation.
 */
uint8_t generic_crc8(const uint8_t *buf, size_t bufsize,
        uint8_t poly, uint8_t init, bool reflectIn, bool reflectOut, uint8_t xorOut);


inline uint32_t crc32_iso_hdlc_bitwise(const uint8_t *buf, size_t bufsize)
{
    // width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926 residue=0xdebb20e3 name="CRC-32/ISO-HDLC"
    return generic_crc32(buf, bufsize, 0x04c11db7, 0xffffffff, true, true, 0xffffffff);
}

inline uint32_t crc32c_bitwise(const uint8_t *buf, size_t bufsize)
{
    // width=32 poly=0x1edc6f41 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xe3069283 residue=0xb798b438 name="CRC-32/ISCSI"
    return generic_crc32(buf, bufsize, 0x1edc6f41, 0xffffffff, true, true, 0xffffffff);
}

inline uint16_t crc16_ibm_bitwise(const uint8_t *buf, size_t bufsize)
{
    // width=16 poly=0x8005 init=0x0000 refin=true refout=true xorout=0x0000 check=0xbb3d residue=0x0000 name="CRC-16/ARC"
    return generic_crc16(buf, bufsize, 0x8005, 0x0000, true, true, 0x0000);
}

inline uint16_t crc16_ccitt_bitwise(const uint8_t *buf, size_t bufsize)
{
    // width=16 poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000 check=0x2189 residue=0x0000 name="CRC-16/KERMIT"
    return generic_crc16(buf, bufsize, 0x1021, 0x0000, true, true, 0x0000);
}

// Variants taking byte arrays
inline uint16_t internetChecksum(const std::vector<uint8_t>& vec, uint32_t sum = 0) { return internetChecksum(vec.data(), vec.size(), sum); }
inline uint32_t ethernetCRC(const std::vector<uint8_t>& vec, uint32_t crc = 0) { return ethernetCRC(vec.data(), vec.size(), crc); }
inline uint32_t crc32_iso_hdlc(const std::vector<uint8_t>& vec, uint32_t crc = 0) { return crc32_iso_hdlc(vec.data(), vec.size(), crc); }
inline uint32_t crc32c(const std::vector<uint8_t>& vec, uint32_t crc = 0) { return crc32c(vec.data(), vec.size(), crc); }
inline uint16_t crc16_ibm(const std::vector<uint8_t>& vec, uint16_t crc = 0) { return crc16_ibm(vec.data(), vec.size(), crc); }
inline uint16_t crc16_ccitt(const std::vector<uint8_t>& vec, uint16_t crc = 0) { return crc16_ccitt(vec.data(), vec.size(), crc); }

inline uint32_t crc32_iso_hdlc_bitwise(const std::vector<uint8_t>& vec) { return crc32_iso_hdlc_bitwise(vec.data(), vec.size()); }
inline uint32_t crc32c_bitwise(const std::vector<uint8_t>& vec) { return crc32c_bitwise(vec.data(), vec.size()); }
inline uint16_t crc16_ibm_bitwise(const std::vector<uint8_t>& vec) { return crc16_ibm_bitwise(vec.data(), vec.size()); }
inline uint16_t crc16_ccitt_bitwise(const std::vector<uint8_t>& vec) { return crc16_ccitt_bitwise(vec.data(), vec.size()); }

} // namespace inet

#endif
