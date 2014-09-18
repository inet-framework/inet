//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
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

#ifndef __INET_TCPIPCHECKSUM_H
#define __INET_TCPIPCHECKSUM_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Calculates checksum.
 */
class TCPIPchecksum
{
  public:
    TCPIPchecksum() {}

    /*
     * calculate the 16 bit one's complement of the one's
     * complement sum of all 16 bit words in the header and text.  If a
     * segment contains an odd number of header and text octets to be
     * checksummed, the last octet is padded on the right with zeros to
     * form a 16 bit word for checksum purposes
     */
    static uint16_t checksum(const void *addr, unsigned int count)
    {
        return ~_checksum(addr, count);
    }

    static uint16_t _checksum(const void *addr, unsigned int count);
};

} // namespace inet

#endif // ifndef __INET_TCPIPCHECKSUM_H

