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

#include "inet/common/packet/SerializerRegistry.h"
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/tcp/headers/tcphdr.h"
#include "inet/common/serializer/tcp/TcpHeaderSerializer.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {

using namespace inet::tcp;

Register_Serializer(TcpHeader, TcpHeaderSerializer);

void TcpHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& tcpHeader = std::static_pointer_cast<const TcpHeader>(chunk);
    struct tcphdr tcp;

    // fill TCP header structure
    tcp.th_sum = 0;
    tcp.th_sport = htons(tcpHeader->getSrcPort());
    tcp.th_dport = htons(tcpHeader->getDestPort());
    tcp.th_seq = htonl(tcpHeader->getSequenceNo());
    tcp.th_ack = htonl(tcpHeader->getAckNo());
    tcp.th_offs = TCP_HEADER_OCTETS / 4;
    tcp.th_x2 = 0;     // unused

    // set flags
    unsigned char flags = 0;
    if (tcpHeader->getFinBit())
        flags |= TH_FIN;
    if (tcpHeader->getSynBit())
        flags |= TH_SYN;
    if (tcpHeader->getRstBit())
        flags |= TH_RST;
    if (tcpHeader->getPshBit())
        flags |= TH_PUSH;
    if (tcpHeader->getAckBit())
        flags |= TH_ACK;
    if (tcpHeader->getUrgBit())
        flags |= TH_URG;
    tcp.th_flags = (TH_FLAGS & flags);
    tcp.th_win = htons(tcpHeader->getWindow());
    tcp.th_urp = htons(tcpHeader->getUrgentPointer());
    if (tcpHeader->getHeaderLength() % 4 != 0)
        throw cRuntimeError("invalid TCP header length=%u: must be dividable by 4", tcpHeader->getHeaderLength());
    tcp.th_offs = tcpHeader->getHeaderLength() / 4;

    for (int i = 0; i < TCP_HEADER_OCTETS; i++)
        stream.writeByte(((uint8_t *)&tcp)[i]);
//    unsigned short numOptions = tcpHeader->getHeaderOptionArraySize();
//    unsigned int optionsLength = 0;
//    if (numOptions > 0) {    // options present?
//        for (unsigned short i = 0; i < numOptions; i++) {
//            const TCPOption *option = tcpHeader->getHeaderOption(i);
//            serializeOption(option, b, c);
//            optionsLength += option->getLength();
//        }    // for
//        //padding:
//        optionsLength %= 4;
//        if (optionsLength)
//            stream.fillNBytes(4 - optionsLength, 0);
//    }    // if options present

    // write data
//    if (tcpHeader->getChunkByteLength() > tcpHeader->getHeaderLength()) {    // data present? FIXME TODO: || tcpHeader->getEncapsulatedPacket()!=nullptr
//        unsigned int dataLength = tcpHeader->getChunkByteLength() - tcpHeader->getHeaderLength();
//
//        if (tcpHeader->getByteArray().getDataArraySize() > 0) {
//            ASSERT(tcpHeader->getByteArray().getDataArraySize() == dataLength);
//            tcpHeader->getByteArray().copyDataToBuffer(stream.accessNBytes(0), stream.getRemainingSize());
//            stream.accessNBytes(dataLength);
//        }
//        else
//            stream.fillNBytes(dataLength, 't'); // fill data part with 't'
//    }
}

std::shared_ptr<Chunk> TcpHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    uint8_t buffer[TCP_HEADER_OCTETS];
    for (int i = 0; i < TCP_HEADER_OCTETS; i++)
        buffer[i] = stream.readByte();
    auto tcpHeader = std::make_shared<TcpHeader>();
    const struct tcphdr& tcp = *static_cast<const struct tcphdr *>((void *)&buffer);
    ASSERT(sizeof(tcp) == TCP_HEADER_OCTETS);

    // fill TCP header structure
    tcpHeader->setSrcPort(ntohs(tcp.th_sport));
    tcpHeader->setDestPort(ntohs(tcp.th_dport));
    tcpHeader->setSequenceNo(ntohl(tcp.th_seq));
    tcpHeader->setAckNo(ntohl(tcp.th_ack));
    unsigned short hdrLength = TCP_HEADER_OCTETS;

    // set flags
    unsigned char flags = tcp.th_flags;
    tcpHeader->setFinBit((flags & TH_FIN) == TH_FIN);
    tcpHeader->setSynBit((flags & TH_SYN) == TH_SYN);
    tcpHeader->setRstBit((flags & TH_RST) == TH_RST);
    tcpHeader->setPshBit((flags & TH_PUSH) == TH_PUSH);
    tcpHeader->setAckBit((flags & TH_ACK) == TH_ACK);
    tcpHeader->setUrgBit((flags & TH_URG) == TH_URG);

    tcpHeader->setWindow(ntohs(tcp.th_win));

    tcpHeader->setUrgentPointer(ntohs(tcp.th_urp));

//    if (hdrLength > TCP_HEADER_OCTETS) {    // options present?
//        unsigned short optionBytes = hdrLength - TCP_HEADER_OCTETS;    // TCP_HEADER_OCTETS = 20
//        Buffer sstream(b, optionBytes);
//
//        while (sstream.getRemainingSize()) {
//            TCPOption *option = deserializeOption(sb, c);
//            tcpHeader->addHeaderOption(option);
//        }
//        if (sstream.hasError())
//            stream.setError();
//    }    // if options present
    tcpHeader->setHeaderLength(hdrLength);
    stream.seek(hdrLength);
//    tcpHeader->setChunkByteLength(stream._getBufSize());
//    unsigned int payloadLength = stream.getRemainingSize();
//    tcpHeader->setPayloadLength(payloadLength);
//    tcpHeader->getByteArray().setDataFromBuffer(stream.accessNBytes(payloadLength), payloadLength);
//
//    pkt->pushHeader(tcpHeader);

//    if (stream.hasError())
//        pkt->setBitError(true);
    // Checksum: modeled by cPacket::hasBitError()
//    if (tcp.th_sum != 0 && c.l3AddressesPtr && c.l3AddressesLength && TCPIPchecksum::checksum(IP_PROT_TCP, stream._getBuf(), stream._getBufSize(), c.l3AddressesPtr, c.l3AddressesLength))
//        pkt->setBitError(true);
    return tcpHeader;
}

} // namespace serializer

} // namespace inet

