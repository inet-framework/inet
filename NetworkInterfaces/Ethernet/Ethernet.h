/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _ETHERNETDEFS_H
#define _ETHERNETDEFS_H


// Constants from the 802.3 spec
#define MAX_PACKETBURST          13
#define GIGABIT_MAX_BURST_BYTES  8192  /* don't start new frame after 8192 or more bytes already transmitted */
#define MAX_ETHERNET_DATA        1500  /* including LLC, SNAP etc headers */
#define MAX_ETHERNET_FRAME       1518  /* excludes preamble and SFD */
#define MIN_ETHERNET_FRAME       64    /* excludes preamble and SFD */
#define GIGABIT_MIN_FRAME_WITH_EXT 512 /* excludes preamble and SFD, but includes 448 byte extension */
#define INTERFRAME_GAP_BITS      96
#define ETHERNET_TXRATE          10000000L   /* 10 Mbit/sec (in bit/s) */
#define FAST_ETHERNET_TXRATE     100000000L  /* 100 Mbit/sec (in bit/s) */
#define GIGABIT_ETHERNET_TXRATE  1000000000L /* 1000 Mbit/sec (in bit/s) */
#define SLOT_TIME                (512.0/ETHERNET_TXRATE)  /* for Ethernet & Fast Ethernet, in seconds */
#define GIGABIT_SLOT_TIME        (4096.0/GIGABIT_ETHERNET_TXRATE) /* seconds */
#define MAX_ATTEMPTS             16
#define BACKOFF_RANGE_LIMIT      10
#define JAM_SIGNAL_BYTES         4
#define PREAMBLE_BYTES           7
#define SFD_BYTES                1
#define PAUSE_BITTIME            512 /* pause is in 512-bit-time units */

#define ETHER_MAC_FRAME_BYTES    (6+6+2+4) /* src(6)+dest(6)+length/type(2)+FCS(4) */
#define ETHER_LLC_HEADER_LENGTH  (3) /* ssap(1)+dsap(1)+control(1) */
#define ETHER_SNAP_HEADER_LENGTH (5) /* org(3)+local(2) */
#define ETHER_PAUSE_COMMAND_BYTES (6) /* FIXME verify */

#endif

