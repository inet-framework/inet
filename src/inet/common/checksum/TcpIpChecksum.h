//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//               2009 Zoltan Bojthe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPIPCHECKSUM_H
#define __INET_TCPIPCHECKSUM_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Calculates checksum.
 */
class INET_API TcpIpChecksum
{
  public:
    TcpIpChecksum() {}

    /*
     * calculate the 16 bit one's complement of the one's
     * complement sum of all 16 bit words in the header and text.  If a
     * segment contains an odd number of header and text octets to be
     * checksummed, the last octet is padded on the right with zeros to
     * form a 16 bit word for checksum purposes
     *
     * calculated checksum in host byte order
     */
    static uint16_t checksum(const void *addr, unsigned int count)
    {
        return ~_checksum(addr, count);
    }

    static uint16_t _checksum(const void *addr, unsigned int count);

    static uint16_t _checksum(const std::vector<uint8_t>& vec);
    static uint16_t checksum(const std::vector<uint8_t>& vec) { return ~_checksum(vec); }
};

} // namespace inet

#endif

