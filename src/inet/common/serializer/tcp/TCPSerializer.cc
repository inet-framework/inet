//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
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

//Define_Module(TCPSerializer);
#include <platdep/sockets.h>

#include "inet/common/serializer/tcp/TCPSerializer.h"

#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"

namespace inet {

namespace serializer {

Register_Serializer(tcp::TCPSegment, IP_PROT, IP_PROT_TCP, TCPSerializer);

// load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"

} // namespace serializer

} // namespace inet

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

using namespace serializer;
using namespace tcp;

void TCPSerializer::serialize(const cPacket *pkt, Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);
    const TCPSegment *tcpseg = check_and_cast<const TCPSegment *>(pkt);
    struct tcphdr *tcp = (struct tcphdr *)(b.accessNBytes(sizeof(struct tcphdr)));
    if (!tcp)
        return;

    int writtenbytes = tcpseg->getByteLength();

    // fill TCP header structure
    tcp->th_sum = 0;
    tcp->th_sport = htons(tcpseg->getSrcPort());
    tcp->th_dport = htons(tcpseg->getDestPort());
    tcp->th_seq = htonl(tcpseg->getSequenceNo());
    tcp->th_ack = htonl(tcpseg->getAckNo());
    tcp->th_offs = TCP_HEADER_OCTETS / 4;

    // set flags
    unsigned char flags = 0;
    if (tcpseg->getFinBit())
        flags |= TH_FIN;
    if (tcpseg->getSynBit())
        flags |= TH_SYN;
    if (tcpseg->getRstBit())
        flags |= TH_RST;
    if (tcpseg->getPshBit())
        flags |= TH_PUSH;
    if (tcpseg->getAckBit())
        flags |= TH_ACK;
    if (tcpseg->getUrgBit())
        flags |= TH_URG;
    tcp->th_flags = (TH_FLAGS & flags);
    tcp->th_win = htons(tcpseg->getWindow());
    tcp->th_urp = htons(tcpseg->getUrgentPointer());

    unsigned short numOptions = tcpseg->getOptionsArraySize();
    unsigned int lengthCounter = 0;
    unsigned char *options = (unsigned char *)tcp->th_options;
    if (numOptions > 0) {    // options present?
        unsigned int maxOptLength = tcpseg->getHeaderLength() - TCP_HEADER_OCTETS;

        for (unsigned short i = 0; i < numOptions; i++) {
            TCPOption option = tcpseg->getOptions(i);
            unsigned short kind = option.getKind();
            unsigned short length = option.getLength();    // length >= 1
            options = ((unsigned char *)tcp->th_options) + lengthCounter;

            lengthCounter += length;

            ASSERT(lengthCounter <= maxOptLength);

            options[0] = kind;
            if (length > 1) {
                options[1] = length;
            }
            unsigned int optlen = option.getValuesArraySize();
            for (unsigned int k = 0; k < optlen; k++) {
                unsigned int value = option.getValues(k);
                unsigned int p0 = 2 + 4 * k;
                for (unsigned int p = std::min((unsigned int)length - 1, 5 + 4 * k); p >= p0; p--) {
                    options[p] = value & 0xFF;
                    value = value >> 8;
                }
            }
        }    // for
             //padding:
        options = (unsigned char *)(tcp->th_options);
        while (lengthCounter % 4)
            options[lengthCounter++] = 0;

        ASSERT(TCP_HEADER_OCTETS + lengthCounter <= TCP_MAX_HEADER_OCTETS);
        tcp->th_offs = (TCP_HEADER_OCTETS + lengthCounter) / 4;    // TCP_HEADER_OCTETS = 20
    }    // if options present
    b.seek(TCP_HEADER_OCTETS + lengthCounter);

    // write data
    if (tcpseg->getByteLength() > tcpseg->getHeaderLength()) {    // data present? FIXME TODO: || tcpseg->getEncapsulatedPacket()!=nullptr
        unsigned int dataLength = tcpseg->getByteLength() - tcpseg->getHeaderLength();
        char *tcpData = (char *)options + lengthCounter;

        if (tcpseg->getByteArray().getDataArraySize() > 0) {
            ASSERT(tcpseg->getByteArray().getDataArraySize() == dataLength);
            tcpseg->getByteArray().copyDataToBuffer(tcpData, dataLength);
        }
        else
            memset(tcpData, 't', dataLength); // fill data part with 't'
    }
    tcp->th_sum = checksum(tcp, writtenbytes, c.l3AddressesPtr, c.l3AddressesLength);
    b.seek(writtenbytes);
}

TCPSegment *TCPSerializer::parse(const unsigned char *buf, unsigned int bufsize, bool withBytes)
{
    Buffer b(const_cast<unsigned char *>(buf), bufsize);
    Context c;
    return check_and_cast_nullable<TCPSegment *>(deserialize(b, c));
}

cPacket* TCPSerializer::deserialize(Buffer &b, Context& context)
{
    struct tcphdr const *const tcp = static_cast<struct tcphdr const *>(b.accessNBytes(TCP_HEADER_OCTETS));
    if (!tcp) {
        return nullptr;
    }

    TCPSegment *tcpseg = new TCPSegment("parsed-tcp");

    // fill TCP header structure
    tcpseg->setSrcPort(ntohs(tcp->th_sport));
    tcpseg->setDestPort(ntohs(tcp->th_dport));
    tcpseg->setSequenceNo(ntohl(tcp->th_seq));
    tcpseg->setAckNo(ntohl(tcp->th_ack));
    unsigned short hdrLength = tcp->th_offs * 4;
    tcpseg->setHeaderLength(hdrLength);

    // set flags
    unsigned char flags = tcp->th_flags;
    tcpseg->setFinBit((flags & TH_FIN) == TH_FIN);
    tcpseg->setSynBit((flags & TH_SYN) == TH_SYN);
    tcpseg->setRstBit((flags & TH_RST) == TH_RST);
    tcpseg->setPshBit((flags & TH_PUSH) == TH_PUSH);
    tcpseg->setAckBit((flags & TH_ACK) == TH_ACK);
    tcpseg->setUrgBit((flags & TH_URG) == TH_URG);

    tcpseg->setWindow(ntohs(tcp->th_win));
    // Checksum (header checksum): modelled by cMessage::hasBitError()
    tcpseg->setUrgentPointer(ntohs(tcp->th_urp));

    if (hdrLength > TCP_HEADER_OCTETS) {    // options present?
        unsigned short optionBytes = hdrLength - TCP_HEADER_OCTETS;    // TCP_HEADER_OCTETS = 20
        if (!b.accessNBytes(optionBytes)) {
            tcpseg->setBitError(true);
            return tcpseg;
        }
        unsigned short optionsCounter = 0;    // index for tcpseg->setOptions[]

        unsigned short length = 0;
        for (unsigned short j = 0; j < optionBytes; j += length) {
            unsigned char *options = ((unsigned char *)tcp->th_options) + j;
            unsigned short kind = options[0];
            length = options[1];

            TCPOption tmpOption;
            switch (kind) {
                case TCPOPTION_END_OF_OPTION_LIST:    // EOL
                case TCPOPTION_NO_OPERATION:    // NOP
                    length = 1;
                    break;

                default:
                    break;
            }    // switch

            // kind
            tmpOption.setKind(kind);
            // length
            tmpOption.setLength(length);
            // value
            int optlen = (length + 1) / 4;
            tmpOption.setValuesArraySize(optlen);
            for (short int k = 0; k < optlen; k++) {
                unsigned int value = 0;
                for (short int l = 2 + 4 * k; l < length && l < 6 + 4 * k; l++) {
                    value = (value << 8) + options[l];
                }
                tmpOption.setValues(k, value);
            }
            // write option to tcp header
            tcpseg->setOptionsArraySize(tcpseg->getOptionsArraySize() + 1);
            tcpseg->setOptions(optionsCounter, tmpOption);
            optionsCounter++;
        }    // for j
    }    // if options present

    tcpseg->setByteLength(b._getBufSize());
    unsigned int payloadLength = b.getRemainder();
    tcpseg->setPayloadLength(payloadLength);
    tcpseg->getByteArray().setDataFromBuffer(b.accessNBytes(payloadLength), payloadLength);
    return tcpseg;
}

uint16_t TCPSerializer::checksum(const void *addr, unsigned int count,
        const void *addr2, unsigned int count2)
{
    uint32_t sum = TCPIPchecksum::_checksum(addr, count) + TCPIPchecksum::_checksum(addr2, count2);

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t) ~sum;
}

} // namespace inet

