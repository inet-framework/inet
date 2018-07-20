/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INET_ETHERNET_H
#define __INET_ETHERNET_H

#include "inet/common/INETDefs.h"

namespace inet {

// Constants from the 802.3 spec
#define MAX_PACKETBURST                     13

const B GIGABIT_MAX_BURST_BYTES           = B(8192);  /* don't start new frame after 8192 or more bytes already transmitted */
const B MAX_ETHERNET_DATA_BYTES           = B(1500);  /* including LLC, SNAP etc headers */
const B MAX_ETHERNET_FRAME_BYTES          = B(1526);  /* excludes preamble and SFD */
const B MIN_ETHERNET_FRAME_BYTES          = B(64);  /* excludes preamble and SFD */
const B GIGABIT_MIN_FRAME_BYTES_WITH_EXT  = B(512);  /* excludes preamble and SFD, but includes 448 byte extension */
const b INTERFRAME_GAP_BITS               = b(96);

#define ETHERNET_TXRATE                     10000000.0   /* 10 Mbit/sec (in bit/s) */
#define FAST_ETHERNET_TXRATE                100000000.0   /* 100 Mbit/sec (in bit/s) */
#define GIGABIT_ETHERNET_TXRATE             1000000000.0   /* 1 Gbit/sec (in bit/s) */
#define FAST_GIGABIT_ETHERNET_TXRATE        10000000000.0   /* 10 Gbit/sec (in bit/s) */
#define FOURTY_GIGABIT_ETHERNET_TXRATE      40000000000.0   /* 40 Gbit/sec (in bit/s) */
#define HUNDRED_GIGABIT_ETHERNET_TXRATE     100000000000.0   /* 100 Gbit/sec (in bit/s) */
#define TWOHUNDRED_GIGABIT_ETHERNET_TXRATE  200000000000.0   /* 200 Gbit/sec (in bit/s) */
#define FOURHUNDRED_GIGABIT_ETHERNET_TXRATE 400000000000.0   /* 400 Gbit/sec (in bit/s) */

#define MAX_ATTEMPTS                        16
#define BACKOFF_RANGE_LIMIT                 10
const B JAM_SIGNAL_BYTES                    = B(4);
const B PREAMBLE_BYTES                      = B(7);
const B SFD_BYTES                           = B(1);

#define PAUSE_UNIT_BITS                     512 /* one pause unit is 512 bit times */

/*
 * The number of bytes in an ethernet (MAC) address.
 */
const B ETHER_ADDR_LEN  = B(6);

/*
 * The number of bytes in the type field.
 */
const B ETHER_TYPE_LEN  = B(2);

/*
 * The number of bytes in the trailing CRC field.
 */
const B ETHER_FCS_BYTES                  = B(4);
const B ETHER_MAC_HEADER_BYTES           = ETHER_ADDR_LEN + ETHER_ADDR_LEN + ETHER_TYPE_LEN; /* src(6)+dest(6)+length/type(2) */
const B ETHER_MAC_FRAME_BYTES            = ETHER_MAC_HEADER_BYTES + ETHER_FCS_BYTES; /* src(6)+dest(6)+length/type(2)+FCS(4) */
const B ETHER_LLC_HEADER_LENGTH          = B(3); /* ssap(1)+dsap(1)+control(1) */
const B ETHER_SNAP_HEADER_LENGTH         = B(5); /* org(3)+local(2) */
const B ETHER_PAUSE_COMMAND_BYTES        = B(2 + 2); /* opcode(2)+parameters(2) */
const B ETHER_PAUSE_COMMAND_PADDED_BYTES = std::max(MIN_ETHERNET_FRAME_BYTES, ETHER_MAC_FRAME_BYTES + ETHER_PAUSE_COMMAND_BYTES);

/*
 * A macro to validate a length with
 */
#define ETHER_IS_VALID_LEN(foo)  ((foo) >= MIN_ETHERNET_FRAME_BYTES && (foo) <= ETHER_MAX_LEN)


} // namespace inet

#endif // ifndef __INET_ETHERNET_H

