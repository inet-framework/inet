//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_ETHERNETCRC_H
#define __INET_ETHERNETCRC_H

#include "inet/common/INETDefs.h"

namespace inet {

extern const uint32_t crc32_tab[];

uint32_t ethernetCRC(const unsigned char *buf, unsigned int bufsize);

} // namespace inet

/*
 * Ethernet CRC32 polynomials (big- and little-endian verions).
 */
#define ETHER_CRC_POLY_LE    0xedb88320
#define ETHER_CRC_POLY_BE    0x04c11db6

#endif /* ETHERNETCRC_H_ */

