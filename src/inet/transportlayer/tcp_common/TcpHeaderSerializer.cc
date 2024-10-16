//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp_common/TcpHeaderSerializer.h"

#include "inet/common/Endian.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/tcp_common/headers/tcphdr.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h> // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace tcp {

Register_Serializer(TcpHeader, TcpHeaderSerializer);

void TcpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& tcpHeader = staticPtrCast<const TcpHeader>(chunk);
    struct tcphdr tcp;

    // fill Tcp header structure
    if (tcpHeader->getCrcMode() != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize Tcp header without a properly computed CRC");
    tcp.th_sum = htons(tcpHeader->getCrc());
    tcp.th_sport = htons(tcpHeader->getSrcPort());
    tcp.th_dport = htons(tcpHeader->getDestPort());
    tcp.th_seq = htonl(tcpHeader->getSequenceNo());
    tcp.th_ack = htonl(tcpHeader->getAckNo());
    tcp.th_x2 = 0; // unused

    // set flags
    uint8_t flags = 0;
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
    if (tcpHeader->getEceBit())
        flags |= TH_ECE;
    if (tcpHeader->getCwrBit())
        flags |= TH_CWR;

    tcp.th_flags = flags;
    tcp.th_win = htons(tcpHeader->getWindow());
    tcp.th_urp = htons(tcpHeader->getUrgentPointer());
    if (B(tcpHeader->getHeaderLength()).get() % 4 != 0)
        throw cRuntimeError("invalid Tcp header length=%s: must be dividable by 4 bytes", tcpHeader->getHeaderLength().str().c_str());
    tcp.th_offs = B(tcpHeader->getHeaderLength()).get() / 4;

    stream.writeBytes((uint8_t *)&tcp, TCP_MIN_HEADER_LENGTH);

    unsigned short numOptions = tcpHeader->getHeaderOptionArraySize();
    unsigned int optionsLength = 0;
    if (numOptions > 0) {
        for (unsigned short i = 0; i < numOptions; i++) {
            const TcpOption *option = tcpHeader->getHeaderOption(i);
            serializeOption(stream, option);
            optionsLength += option->getLength();
        }
        if (optionsLength % 4 != 0)
            stream.writeByteRepeatedly(0, 4 - optionsLength % 4);
    }
    ASSERT(tcpHeader->getHeaderLength() == TCP_MIN_HEADER_LENGTH + B(optionsLength));
}

void TcpHeaderSerializer::serializeOption(MemoryOutputStream& stream, const TcpOption *option) const
{
    TcpOptionNumbers kind = option->getKind();
    unsigned short length = option->getLength(); // length >= 1

    stream.writeByte(kind);
    if (length > 1)
        stream.writeByte(length);

    auto *opt = dynamic_cast<const TcpOptionUnknown *>(option);
    if (opt) {
        unsigned int datalen = opt->getBytesArraySize();
        ASSERT(length == 2 + datalen);
        for (unsigned int i = 0; i < datalen; i++)
            stream.writeByte(opt->getBytes(i));
        return;
    }

    switch (kind) {
        case TCPOPTION_END_OF_OPTION_LIST: // EOL
            check_and_cast<const TcpOptionEnd *>(option);
            ASSERT(length == 1);
            break;

        case TCPOPTION_NO_OPERATION: // NOP
            check_and_cast<const TcpOptionNop *>(option);
            ASSERT(length == 1);
            break;

        case TCPOPTION_MAXIMUM_SEGMENT_SIZE: {
            auto *opt = check_and_cast<const TcpOptionMaxSegmentSize *>(option);
            ASSERT(length == 4);
            stream.writeUint16Be(opt->getMaxSegmentSize());
            break;
        }

        case TCPOPTION_WINDOW_SCALE: {
            auto *opt = check_and_cast<const TcpOptionWindowScale *>(option);
            ASSERT(length == 3);
            stream.writeByte(opt->getWindowScale());
            break;
        }

        case TCPOPTION_SACK_PERMITTED: {
            auto *opt = check_and_cast<const TcpOptionSackPermitted *>(option);
            (void)opt; // UNUSED
            ASSERT(length == 2);
            break;
        }

        case TCPOPTION_SACK: {
            auto *opt = check_and_cast<const TcpOptionSack *>(option);
            ASSERT(length == 2 + opt->getSackItemArraySize() * 8);
            for (unsigned int i = 0; i < opt->getSackItemArraySize(); i++) {
                SackItem si = opt->getSackItem(i);
                stream.writeUint32Be(si.getStart());
                stream.writeUint32Be(si.getEnd());
            }
            break;
        }

        case TCPOPTION_TIMESTAMP: {
            auto *opt = check_and_cast<const TcpOptionTimestamp *>(option);
            ASSERT(length == 10);
            stream.writeUint32Be(opt->getSenderTimestamp());
            stream.writeUint32Be(opt->getEchoedTimestamp());
            break;
        }

        default: {
            throw cRuntimeError("Unknown TCPOption kind=%d (not in a TCPOptionUnknown option)", kind);
            break;
        }
    } // switch
}

const Ptr<Chunk> TcpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto position = stream.getPosition();
    uint8_t *buffer = new uint8_t[B(TCP_MIN_HEADER_LENGTH).get()];
    stream.readBytes(buffer, TCP_MIN_HEADER_LENGTH);
    auto tcpHeader = makeShared<TcpHeader>();
    const struct tcphdr& tcp = *static_cast<const struct tcphdr *>((void *)buffer);
    ASSERT(B(sizeof(tcp)) == TCP_MIN_HEADER_LENGTH);

    // fill Tcp header structure
    tcpHeader->setSrcPort(ntohs(tcp.th_sport));
    tcpHeader->setDestPort(ntohs(tcp.th_dport));
    tcpHeader->setSequenceNo(ntohl(tcp.th_seq));
    tcpHeader->setAckNo(ntohl(tcp.th_ack));
    B headerLength = B(tcp.th_offs * 4);

    // set flags
    unsigned char flags = tcp.th_flags;
    tcpHeader->setFinBit((flags & TH_FIN) == TH_FIN);
    tcpHeader->setSynBit((flags & TH_SYN) == TH_SYN);
    tcpHeader->setRstBit((flags & TH_RST) == TH_RST);
    tcpHeader->setPshBit((flags & TH_PUSH) == TH_PUSH);
    tcpHeader->setAckBit((flags & TH_ACK) == TH_ACK);
    tcpHeader->setUrgBit((flags & TH_URG) == TH_URG);
    tcpHeader->setEceBit((flags & TH_ECE) == TH_ECE);
    tcpHeader->setCwrBit((flags & TH_CWR) == TH_CWR);

    tcpHeader->setWindow(ntohs(tcp.th_win));

    tcpHeader->setUrgentPointer(ntohs(tcp.th_urp));

    if (headerLength > TCP_MIN_HEADER_LENGTH) {
        while (stream.getPosition() - position < headerLength) {
            TcpOption *option = deserializeOption(stream);
            tcpHeader->appendHeaderOption(option);
        }
    }
    tcpHeader->setHeaderLength(headerLength);
    tcpHeader->setCrc(ntohs(tcp.th_sum));
    tcpHeader->setCrcMode(CRC_COMPUTED);
    delete [] buffer;
    return tcpHeader;
}

TcpOption *TcpHeaderSerializer::deserializeOption(MemoryInputStream& stream) const
{
    TcpOptionNumbers kind = static_cast<TcpOptionNumbers>(stream.readByte());
    unsigned char length = 0;

    switch (kind) {
        case TCPOPTION_END_OF_OPTION_LIST: // EOL
            return new TcpOptionEnd();

        case TCPOPTION_NO_OPERATION: // NOP
            return new TcpOptionNop();

        case TCPOPTION_MAXIMUM_SEGMENT_SIZE:
            length = stream.readByte();
            if (length == 4) {
                auto *option = new TcpOptionMaxSegmentSize();
                option->setLength(length);
                option->setMaxSegmentSize(stream.readUint16Be());
                return option;
            }
            break;

        case TCPOPTION_WINDOW_SCALE:
            length = stream.readByte();
            if (length == 3) {
                auto *option = new TcpOptionWindowScale();
                option->setLength(length);
                option->setWindowScale(stream.readByte());
                return option;
            }
            break;

        case TCPOPTION_SACK_PERMITTED:
            length = stream.readByte();
            if (length == 2) {
                auto *option = new TcpOptionSackPermitted();
                option->setLength(length);
                return option;
            }
            break;

        case TCPOPTION_SACK:
            length = stream.readByte();
            if (length > 2 && (length % 8) == 2) {
                auto *option = new TcpOptionSack();
                option->setLength(length);
                option->setSackItemArraySize(length / 8);
                unsigned int count = 0;
                for (unsigned int i = 2; i < length; i += 8) {
                    SackItem si;
                    si.setStart(stream.readUint32Be());
                    si.setEnd(stream.readUint32Be());
                    option->setSackItem(count++, si);
                }
                return option;
            }
            break;

        case TCPOPTION_TIMESTAMP:
            length = stream.readByte();
            if (length == 10) {
                auto *option = new TcpOptionTimestamp();
                option->setLength(length);
                option->setSenderTimestamp(stream.readUint32Be());
                option->setEchoedTimestamp(stream.readUint32Be());
                return option;
            }
            break;

        default:
            length = stream.readByte();
            break;
    } // switch

    auto *option = new TcpOptionUnknown();
    option->setKind(kind);
    option->setLength(length);
    if (length > 2)
        option->setBytesArraySize(length - 2);
    for (unsigned int i = 2; i < length; i++)
        option->setBytes(i - 2, stream.readByte());
    return option;
}

} // namespace tcp

} // namespace inet

