//
// Copyright (C) 2005 Christian Dankbar
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

#ifndef OPPSIM_NETINET_TCP_H
#define OPPSIM_NETINET_TCP_H	

#  define TH_FIN	0x01
#  define TH_SYN	0x02
#  define TH_RST	0x04
#  define TH_PUSH	0x08
#  define TH_ACK	0x10
#  define TH_URG	0x20
#define TH_FLAGS	0x3F

struct tcphdr
  {
    unsigned short th_sport;		/* source port */
    unsigned short th_dport;		/* destination port */
    unsigned int th_seq;		/* sequence number */
    unsigned int th_ack;		/* acknowledgement number */
    unsigned char th_off;
    unsigned char th_flags;
    unsigned short th_win;		/* window */
    unsigned short th_sum;		/* checksum */
    unsigned short th_urp;		/* urgent pointer */
};


#ifndef _PSEUDOHEADER_  //cd
#define _PSEUDOHEADER_
typedef struct {
	unsigned long srcaddr;
	unsigned long dstaddr;
	unsigned char zero;
	unsigned char ptcl;
	unsigned short len;
} pseudoheader;
#endif

#endif /* netinet/tcp.h */
