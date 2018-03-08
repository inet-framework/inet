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

#include "inet/common/serializer/SctpChecksum.h"
#include "inet/common/serializer/sctp/headers/sctphdr.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {
namespace serializer {

uint32_t SctpChecksum::checksum(const void *addr, unsigned int len)
{
    uint32 h;
    const uint8_t *buf = static_cast<const uint8_t *>(addr);
    unsigned char byte0, byte1, byte2, byte3;
    uint32 crc32c;
    uint32 i;
    uint32 res = (~0L);
    for (i = 0; i < len; i++) {
        CRC32C(res, buf[i]);
    }
    h = ~res;
    byte0 = h & 0xff;
    byte1 = (h >> 8) & 0xff;
    byte2 = (h >> 16) & 0xff;
    byte3 = (h >> 24) & 0xff;
    crc32c = ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3);
    return htonl(crc32c);
}

} // namespace serializer
} // namespace inet
