//
// Copyright (C) 2005 Christian Dankbar
// Copyright (C) 2009 Thomas Reschka
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

#ifndef __INET_TCPHDR_H
#define __INET_TCPHDR_H

namespace inet {

namespace tcp {

#  define TH_FIN     0x01
#  define TH_SYN     0x02
#  define TH_RST     0x04
#  define TH_PUSH    0x08
#  define TH_ACK     0x10
#  define TH_URG     0x20
#  define TH_ECE     0x40
#  define TH_CWR     0x80
#define TH_FLAGS     0x3F

struct tcphdr
{
    uint16_t th_sport;    /* source port */
    uint16_t th_dport;    /* destination port */
    uint32_t th_seq;    /* sequence number */
    uint32_t th_ack;    /* acknowledgement number */
#  if BYTE_ORDER == LITTLE_ENDIAN
    uint8_t th_x2 : 4;    /* (unused) */
    uint8_t th_offs : 4;    /* data offset */
#  elif BYTE_ORDER == BIG_ENDIAN
    uint8_t th_offs : 4;    /* data offset */
    uint8_t th_x2 : 4;    /* (unused) */
#else // if BYTE_ORDER == LITTLE_ENDIAN
# error "Please check BYTE_ORDER declaration"
#  endif // if BYTE_ORDER == LITTLE_ENDIAN
    uint8_t th_flags;
    uint16_t th_win;    /* window */
    uint16_t th_sum;    /* checksum */
    uint16_t th_urp;    /* urgent pointer */

    uint32_t th_options[0];    /* options (optional) */
    //unsigned char data[0];        XXX MSVC only allows zero-size arrays at the end of a struct
};    // TODO  __attribute__((packed));

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPHDR_H

