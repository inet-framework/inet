//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
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

//#include "headers/defs.h"
//namespace INETFw // load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
/*{
#include "headers/bsdint.h"
#include "headers/in.h"
#include "headers/in_systm.h"
//#include "headers/ip.h"
#include "headers/tcp.h"
};

#include "TCPSerializer.h"
//#include "TCPConnection.h"

#ifndef _MSC_VER
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace INETFw;*/

//Define_Module(SCTPSerializer);
#include "TCPSerializer.h"

int TCPSerializer::serialize(TCPSegment *msg, unsigned char *buf, unsigned int bufsize,  pseudoheader *pseudo)
{
	//cMessage* cmsg;
	struct tcphdr *tcp = (struct tcphdr*) (buf);
	//int writtenbytes = sizeof(struct tcphdr)+msg->payloadLength();
	int writtenbytes = msg->getByteLength();

	// fill TCP header structure
	tcp->th_sum = 0;
	tcp->th_sport = htons(msg->getSrcPort());
	tcp->th_dport = htons(msg->getDestPort());
	tcp->th_seq = htonl(msg->getSequenceNo());
	tcp->th_ack = htonl(msg->getAckNo());
	tcp->th_off = (sizeof(struct tcphdr)/4)<<4;

	unsigned char flags = 0;
	if(msg->getFinBit())
		flags |= TH_FIN;
	if(msg->getSynBit())
		flags |= TH_SYN;
	if(msg->getRstBit())
		flags |= TH_RST;
	if(msg->getPshBit())
		flags |= TH_PUSH;
	if(msg->getAckBit())
		flags |= TH_ACK;
	if(msg->getUrgBit())
		flags |= TH_URG;
	tcp->th_flags = (TH_FLAGS & flags);
	tcp->th_win = htons(msg->getWindow());
	tcp->th_urp = htons(msg->getUrgentPointer());


	// for computation of the checksum we need the pseudo header in front of the tcp header
	int wholeheadersize = sizeof(pseudoheader) + writtenbytes;
	char *wholeheader = (char*) malloc(wholeheadersize);

	memcpy(wholeheader, pseudo, sizeof(pseudoheader));
	memcpy(wholeheader + sizeof(pseudoheader), buf, writtenbytes);

	tcp->th_sum = checksum((unsigned char*)wholeheader, wholeheadersize);
	free(wholeheader);
	//tcp->th_sum =checksum((unsigned char*)tcp, writtenbytes);
	return writtenbytes;
}

unsigned short TCPSerializer::checksum(unsigned char *addr, unsigned int count)
{
    long sum = 0;

    while (count > 1)  {
        sum += *((unsigned short *&)addr)++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        count -= 2;
    }

    if (count)
        sum += *(unsigned char *)addr;

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}



