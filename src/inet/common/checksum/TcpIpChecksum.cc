//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
//               2009 Zoltan Bojthe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/checksum/TcpIpChecksum.h"

namespace inet {

uint16_t TcpIpChecksum::_checksum(const void *_addr, unsigned int count)
{
    const uint8_t *addr = static_cast<const uint8_t *>(_addr);
    uint32_t sum = 0;

    while (count > 1) {
        sum += (addr[0] << 8) | addr[1];
        addr += 2;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        count -= 2;
    }

    if (count)
        sum += addr[0] << 8;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t)sum;
}

uint16_t TcpIpChecksum::_checksum(const std::vector<uint8_t>& vec)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < vec.size(); i++) {
        sum += (i & 1) ? vec[i] : (vec[i] << 8);
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t)sum;
}

} // namespace inet

