//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHECKSUM_H
#define __INET_CHECKSUM_H

#include "inet/common/INETDefs.h"

namespace inet {

uint16_t internetChecksum(const void *_addr, unsigned int count, uint32_t sum = 0);

inline uint16_t internetChecksum(const std::vector<uint8_t>& vec, uint32_t sum = 0) {
    return internetChecksum(vec.data(), vec.size(), sum);
}

uint32_t ethernetCRC(const unsigned char *buf, unsigned int bufsize, uint32_t crc = 0);

inline uint32_t ethernetCRC(const std::vector<uint8_t>& vec, uint32_t crc = 0) {
    return ethernetCRC(vec.data(), vec.size(), crc);
}

} // namespace inet

#endif
