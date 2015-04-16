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

#include "inet/common/serializer/SerializerUtil.h"

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

void TCPSerializer::serializeOption(const TCPOption *option, Buffer &b, Context& c)
{
    unsigned short kind = option->getKind();
    unsigned short length = option->getLength();    // length >= 1

    b.writeByte(kind);
    if (length > 1)
        b.writeByte(length);

    auto *opt = dynamic_cast<const TCPOptionUnknown *>(option);
    if (opt) {
        unsigned int datalen = opt->getBytesArraySize();
        ASSERT(length == 2 + datalen);
        for (unsigned int i = 0; i < datalen; i++)
            b.writeByte(opt->getBytes(i));
        return;
    }

    switch (kind) {
        case TCPOPTION_END_OF_OPTION_LIST:    // EOL
            check_and_cast<const TCPOptionEnd *>(option);
            ASSERT(length == 1);
            break;

        case TCPOPTION_NO_OPERATION:    // NOP
            check_and_cast<const TCPOptionNop *>(option);
            ASSERT(length == 1);
            break;

        case TCPOPTION_MAXIMUM_SEGMENT_SIZE: {
            auto *opt = check_and_cast<const TCPOptionMaxSegmentSize *>(option);
            ASSERT(length == 4);
            b.writeUint16(opt->getMaxSegmentSize());
            break;
        }

        case TCPOPTION_WINDOW_SCALE: {
            auto *opt = check_and_cast<const TCPOptionWindowScale *>(option);
            ASSERT(length == 3);
            b.writeByte(opt->getWindowScale());
            break;
        }

        case TCPOPTION_SACK_PERMITTED: {
            auto *opt = check_and_cast<const TCPOptionSackPermitted *>(option);
            ASSERT(length == 2);
            break;
        }

        case TCPOPTION_SACK: {
            auto *opt = check_and_cast<const TCPOptionSack *>(option);
            ASSERT(length == 2 + opt->getSackItemArraySize() * 8);
            for (unsigned int i = 0; i < opt->getSackItemArraySize(); i++) {
                SackItem si = opt->getSackItem(i);
                b.writeUint32(si.getStart());
                b.writeUint32(si.getEnd());
            }
            break;
        }

        case TCPOPTION_TIMESTAMP: {
            auto *opt = check_and_cast<const TCPOptionTimestamp *>(option);
            ASSERT(length == 10);
            b.writeUint32(opt->getSenderTimestamp());
            b.writeUint32(opt->getEchoedTimestamp());
            break;
        }

        default: {
            throw cRuntimeError("Unknown TCPOption kind=%d (not in a TCPOptionUnknown option)", kind);
            break;
        }
    }    // switch
}

void TCPSerializer::serialize(const cPacket *pkt, Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);
    const TCPSegment *tcpseg = check_and_cast<const TCPSegment *>(pkt);
    ASSERT(tcpseg->getHeaderLength() <= TCP_MAX_HEADER_OCTETS);
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
    tcp->th_x2 = 0;     // unused

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

    unsigned short numOptions = tcpseg->getHeaderOptionArraySize();
    unsigned char *options = (unsigned char *)tcp->th_options;
    unsigned int optionsLength = tcpseg->getHeaderLength() - TCP_HEADER_OCTETS;
    if (numOptions > 0) {    // options present?
        Buffer sb(b, 0, optionsLength);

        for (unsigned short i = 0; i < numOptions; i++) {
            const TCPOption *option = tcpseg->getHeaderOption(i);
            serializeOption(option, sb, c);
        }    // for
        //padding:
        sb.fillNBytes(sb.getRemainder(), 0);
        if (sb.hasError())
            b.setError();

        tcp->th_offs = (TCP_HEADER_OCTETS + sb.getPos() + 3) / 4;    // TCP_HEADER_OCTETS = 20
    }    // if options present
    b.seek(tcpseg->getHeaderLength());

    // write data
    if (tcpseg->getByteLength() > tcpseg->getHeaderLength()) {    // data present? FIXME TODO: || tcpseg->getEncapsulatedPacket()!=nullptr
        unsigned int dataLength = tcpseg->getByteLength() - tcpseg->getHeaderLength();
        char *tcpData = (char *)options + optionsLength;

        if (tcpseg->getByteArray().getDataArraySize() > 0) {
            ASSERT(tcpseg->getByteArray().getDataArraySize() == dataLength);
            tcpseg->getByteArray().copyDataToBuffer(tcpData, dataLength);
        }
        else
            b.fillNBytes(dataLength, 't'); // fill data part with 't'
    }
    tcp->th_sum = htons(TCPIPchecksum::checksum(IP_PROT_TCP, tcp, writtenbytes, c.l3AddressesPtr, c.l3AddressesLength));
    b.seek(writtenbytes);
}

TCPOption *TCPSerializer::deserializeOption(Buffer &b, Context& c)
{
    unsigned char kind = b.readByte();
    unsigned char length = 0;

    switch (kind) {
        case TCPOPTION_END_OF_OPTION_LIST:    // EOL
            return new TCPOptionEnd();

        case TCPOPTION_NO_OPERATION:    // NOP
            return new TCPOptionNop();

        case TCPOPTION_MAXIMUM_SEGMENT_SIZE:
            length = b.readByte();
            if (length == 4) {
                auto *option = new TCPOptionMaxSegmentSize();
                option->setLength(length);
                option->setMaxSegmentSize(b.readUint16());
                return option;
            }
            break;

        case TCPOPTION_WINDOW_SCALE:
            length = b.readByte();
            if (length == 3) {
                auto *option = new TCPOptionWindowScale();
                option->setLength(length);
                option->setWindowScale(b.readByte());
                return option;
            }
            break;

        case TCPOPTION_SACK_PERMITTED:
            length = b.readByte();
            if (length == 2) {
                auto *option = new TCPOptionSackPermitted();
                option->setLength(length);
                return option;
            }
            break;

        case TCPOPTION_SACK:
            length = b.readByte();
            if (length > 2 && (length & 8) == 2) {
                auto *option = new TCPOptionSack();
                option->setLength(length);
                option->setSackItemArraySize(length / 8);
                unsigned int count = 0;
                for (unsigned int i = 2; i < length; i += 8) {
                    SackItem si;
                    si.setStart(b.readUint32());
                    si.setEnd(b.readUint32());
                    option->setSackItem(count++, si);
                }
                return option;
            }
            break;

        case TCPOPTION_TIMESTAMP:
            length = b.readByte();
            if (length == 10) {
                auto *option = new TCPOptionTimestamp();
                option->setLength(length);
                option->setSenderTimestamp(b.readUint32());
                option->setEchoedTimestamp(b.readUint32());
                return option;
            }
            break;

        default:
            length = b.readByte();
            break;
    }    // switch

    auto *option = new TCPOptionUnknown();
    option->setKind(kind);
    option->setLength(length);
    if (length > 2)
        option->setBytesArraySize(length - 2);
    for (unsigned int i = 2; i < length; i++)
        option->setBytes(i-2, b.readByte());
    return option;
}

TCPSegment *TCPSerializer::deserialize(const unsigned char *buf, unsigned int bufsize, bool withBytes)
{
    Buffer b(const_cast<unsigned char *>(buf), bufsize);
    Context c;
    return check_and_cast_nullable<TCPSegment *>(deserialize(b, c));
}

cPacket* TCPSerializer::deserialize(Buffer &b, Context& c)
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

    tcpseg->setUrgentPointer(ntohs(tcp->th_urp));

    if (hdrLength > TCP_HEADER_OCTETS) {    // options present?
        unsigned short optionBytes = hdrLength - TCP_HEADER_OCTETS;    // TCP_HEADER_OCTETS = 20
        Buffer sb(b, 0, optionBytes);

        while (sb.getRemainder()) {
            TCPOption *option = deserializeOption(sb, c);
            tcpseg->addHeaderOption(option);
        }
        if (sb.hasError())
            b.setError();
    }    // if options present
    b.seek(hdrLength);
    tcpseg->setByteLength(b._getBufSize());
    unsigned int payloadLength = b.getRemainder();
    tcpseg->setPayloadLength(payloadLength);
    tcpseg->getByteArray().setDataFromBuffer(b.accessNBytes(payloadLength), payloadLength);

    if (b.hasError())
        tcpseg->setBitError(true);
    // Checksum: modeled by cPacket::hasBitError()
    if (tcp->th_sum != 0 && c.l3AddressesPtr && c.l3AddressesLength && TCPIPchecksum::checksum(IP_PROT_TCP, tcp, b._getBufSize(), c.l3AddressesPtr, c.l3AddressesLength))
        tcpseg->setBitError(true);
    return tcpseg;
}

} // namespace inet

