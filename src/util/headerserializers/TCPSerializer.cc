//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
//               2009 Thomas Reschka
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

//Define_Module(TCPSerializer);
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

    // set flags
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

    unsigned short numOptions = msg->getOptionsArraySize();
    if (numOptions > 0) // options present?
    {
        unsigned short lengthCounter = 0;

        while (lengthCounter < (msg->getHeaderLength()-TCP_HEADER_OCTETS)) // TCP_HEADER_OCTETS = 20
        {
            unsigned short j = 0; // index for tcp->th_options[]
            for (unsigned short i=0; i<numOptions; i++)
            {
                TCPOption option = msg->getOptions(i);
                unsigned short kind = option.getKind();
                unsigned short length = option.getLength();

                lengthCounter = lengthCounter + length;

                switch(kind)
                {
                    case TCPOPTION_END_OF_OPTION_LIST: // EOL
                    {
                        // kind
                        if (tcp->th_options[j] == 0)
                            {tcp->th_options[j] = kind;}
                        else
                            {tcp->th_options[j] = tcp->th_options[j] + (kind<<8);}  // 1 padding byte prefixed
                        break;
                    }
                    case TCPOPTION_NO_OPERATION: // NOP
                    {
                        // kind
                        if (tcp->th_options[j] == 0) // first NOP
                            {tcp->th_options[j] = kind;}
                        else
                            {tcp->th_options[j] = tcp->th_options[j] + (kind<<8);} // 1 padding byte prefixed
                        break;
                    }
                    case TCPOPTION_MAXIMUM_SEGMENT_SIZE: // MSS
                    {
                        // kind
                        tcp->th_options[j] = kind;
                        // length
                        tcp->th_options[j] = tcp->th_options[j] + (length<<8);
                        // value
                        if (option.getValuesArraySize()!=0)
                            {tcp->th_options[j] = tcp->th_options[j] + htonl(option.getValues(0));}
                        j++;
                        break;
                    }
                    case TCPOPTION_SACK_PERMITTED: // SACK_PERMITTED
                    {
                        // kind
                        tcp->th_options[j] = tcp->th_options[j] + (kind<<16);   // 2 padding bytes prefixed
                        // length
                        tcp->th_options[j] = tcp->th_options[j] + (length<<24); // 2 padding bytes prefixed
                        j++;
                        break;
                    }
                    case TCPOPTION_SACK: // SACK
                    {
                        // kind
                        tcp->th_options[j] = tcp->th_options[j] + (kind<<16); // 2 padding bytes prefixed
                        // length
                        tcp->th_options[j] = tcp->th_options[j] + (length<<24); // 2 padding bytes prefixed
                        j++;
                        // values
                        if (option.getValuesArraySize()!=0)
                        {
                            for (unsigned short k=0; k<option.getValuesArraySize(); k++)
                            {
                                tcp->th_options[j] = tcp->th_options[j] + htonl(option.getValues(k));
                                j++;
                            }
                        }
                        break;
                    }

                    // TODO add new TCPOptions here once they are implemented
                    default:
                    {
                        EV << "ERROR: Received option of kind " << kind << " with length " << length << " which is currently not supported\n";
                        break;
                    }
                } // switch
            } // for
        } // while
        tcp->th_off = ((TCP_HEADER_OCTETS+lengthCounter)/4)<<4; // TCP_HEADER_OCTETS = 20
    } // if options present

    // write data
    if (msg->getByteLength() > msg->getHeaderLength()) // data present? FIXME TODO: || msg->getEncapsulatedMsg()!=NULL
    {
        unsigned int dataLength = msg->getByteLength() - msg->getHeaderLength();
//      TCPPayloadMessage *tcpP = check_and_cast<TCPPayloadMessage* >(msg->getEncapsulatedMsg()); // FIXME
        char *tcpData = (char *)tcp->th_options; //FIXME + length(options)!
        for (unsigned int i=0; i < dataLength; i++)
        {
            if (i < msg->getPayloadArraySize())
            {
//              tcpData[i] = (unsigned char) tcpP.msg; // FIXME
                tcpData[i] = 't'; // FIXME - write 't' as dummy data
            }
            else
                {tcpData[i] = 't';} // write 't' as dummy data
        }
    }

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

void TCPSerializer::parse(unsigned char *buf, unsigned int bufsize, TCPSegment *tcpseg)
{
    struct tcphdr *tcp = (struct tcphdr*) (buf);

    // fill TCP header structure
    tcpseg->setSrcPort(ntohs(tcp->th_sport));
    tcpseg->setDestPort(ntohs(tcp->th_dport));
    tcpseg->setSequenceNo(ntohl(tcp->th_seq));
    tcpseg->setAckNo(ntohl(tcp->th_ack));
    tcpseg->setHeaderLength(((unsigned short) tcp->th_off)/4);
    ASSERT(tcpseg->getHeaderLength()>=TCP_HEADER_OCTETS || tcpseg->getHeaderLength()<=TCP_MAX_HEADER_OCTETS); // TCP_HEADER_OCTETS = 20, TCP_MAX_HEADER_OCTETS = 60

    // set flags
    unsigned char flags = tcp->th_flags;
    if ((flags & TH_FIN) == TH_FIN)
        tcpseg->setFinBit(true);
    if ((flags & TH_SYN) == TH_SYN)
        tcpseg->setSynBit(true);
    if ((flags & TH_RST) == TH_RST)
        tcpseg->setRstBit(true);
    if ((flags & TH_PUSH) == TH_PUSH)
        tcpseg->setPshBit(true);
    if ((flags & TH_ACK) == TH_ACK)
        tcpseg->setAckBit(true);
    if ((flags & TH_URG) == TH_URG)
        tcpseg->setUrgBit(true);

    tcpseg->setWindow(ntohs(tcp->th_win));
    // Checksum (header checksum): modelled by cMessage::hasBitError()
    tcpseg->setUrgentPointer(ntohs(tcp->th_urp));

    if (tcpseg->getHeaderLength() > TCP_HEADER_OCTETS) // options present?
    {
        unsigned short optionBytes = tcpseg->getHeaderLength() - TCP_HEADER_OCTETS; // TCP_HEADER_OCTETS = 20
        unsigned short numOptions = optionBytes/4;
        unsigned short optionsCounter = 0; // index for tcpseg->setOptions[]

        for (unsigned short j=0; j<numOptions; j++)
        {
            unsigned int tmp = ntohl(tcp->th_options[j]);
            unsigned short tmpArray[4];
            tmpArray[0] = (tmp>>24) & 0xff; // byte0
            tmpArray[1] = (tmp>>16) & 0xff; // byte1
            tmpArray[2] = (tmp>>8)  & 0xff; // byte2
            tmpArray[3] =  tmp      & 0xff; // byte3

            unsigned short kind   = tmpArray[0];
            unsigned short length = tmpArray[1];
            unsigned short value  = tmp; // byte 2 and 3 (16 bit)

            bool lengthMatched = false;
            unsigned short i=0;

            while(!lengthMatched && i<3)
            {
                kind = tmpArray[i];
                length = tmpArray[(i+1)];

                TCPOption tmpOption;
                switch(kind)
                {
                    case TCPOPTION_END_OF_OPTION_LIST: // EOL
                    {
                        // kind
                        tmpOption.setKind(kind);
                        // length
                        tmpOption.setLength(1);
                        // no value
                        tmpOption.setValuesArraySize(0);
                        break;
                    }
                    case TCPOPTION_NO_OPERATION: // NOP
                    {
                        // kind
                        tmpOption.setKind(kind);
                        // length
                        tmpOption.setLength(1);
                        // no value
                        tmpOption.setValuesArraySize(0);
                        break;
                    }
                    case TCPOPTION_MAXIMUM_SEGMENT_SIZE: // MSS
                    {
                        // kind
                        tmpOption.setKind(kind);
                        // length
                        tmpOption.setLength(length);
                        // value
                        tmpOption.setValuesArraySize(1);
                        value = (unsigned short) tmp; // byte 2 and 3 (16 bit)
                        tmpOption.setValues(0,value);
                        if (length == 4)
                            {lengthMatched = true;}
                        break;
                    }
                    case TCPOPTION_SACK_PERMITTED: // SACK_PERMITTED
                    {
                        // kind
                        tmpOption.setKind(kind);
                        // length
                        tmpOption.setLength(length);
                        // no value
                        tmpOption.setValuesArraySize(0);
                        lengthMatched = true;
                        if ((i+1) == 2 && length == 2)
                            {lengthMatched = true;}
                        break;
                    }
                    case TCPOPTION_SACK: // SACK
                    {
                        // kind
                        tmpOption.setKind(kind);
                        // length
                        tmpOption.setLength(length);
                        // values
                        tmpOption.setValuesArraySize((length-2)/4);
                        for (unsigned short k=0; k<tmpOption.getValuesArraySize(); k++)
                        {
                            j++;
                            tmpOption.setValues(k,ntohl(tcp->th_options[j]));
                        }
                        if ((2+length)%4==0)
                            {lengthMatched = true;}
                        break;
                    }

                    // TODO add new TCPOptions here once they are implemented
                    default:
                    {
                        EV << "ERROR: Received option of kind " << kind << " with length " << length << " which is currently not supported\n";
                        tmpOption.setKind(1); // NOP
                        tmpOption.setLength(1);
                        tmpOption.setValuesArraySize(0);
                        break;
                    }
                } // switch

                // write option to tcp header
                tcpseg->setOptionsArraySize(tcpseg->getOptionsArraySize()+1);
                tcpseg->setOptions(optionsCounter,tmpOption);
                optionsCounter++;
                i++;
            } // while i
        } // for j
    } // if options present

    tcpseg->setByteLength(bufsize);
    tcpseg->setPayloadLength(tcpseg->getByteLength() - tcpseg->getHeaderLength());
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
