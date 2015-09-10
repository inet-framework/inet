//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
//               2009 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/serializer/TCPIPchecksum.h"

namespace inet {
namespace serializer {

uint16_t TCPIPchecksum::_checksum(const void *_addr, unsigned int count)
{
    const unsigned char *addr = static_cast<const unsigned char *>(_addr);
    uint32_t sum = 0;

    while (count > 1) {
        sum += (addr[0] << 8) | addr[1];
        addr += 2;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        count -= 2;
    }

    if (count)
        sum += *(const uint8_t *)addr;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t)sum;
}

uint16_t TCPIPchecksum::checksum(unsigned int protocolId, const void *packet, unsigned int packetLength,
        const void *addr, unsigned int addrLength)
{
    uint32_t sum = TCPIPchecksum::_checksum(packet, packetLength)
            + TCPIPchecksum::_checksum(addr, addrLength) + packetLength + protocolId;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t) ~sum;
}


} // namespace serializer
} // namespace inet

