//
// Copyright (C) 2015 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "PacketDrill.h"
#include "PacketDrillUtils.h"
#include "inet/transportlayer/udp/UDPPacket_m.h"
#include "inet/transportlayer/tcp_common/TCPSegment_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram_m.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"

using namespace inet;
using namespace tcp;
using namespace sctp;

PacketDrillApp *PacketDrill::pdapp = nullptr;

PacketDrill::PacketDrill(PacketDrillApp *mod)
{
    pdapp = mod;
}

PacketDrill::~PacketDrill()
{

}

IPv4Datagram* PacketDrill::makeIPPacket(int protocol, enum direction_t direction, L3Address localAddr, L3Address remoteAddr)
{
    IPv4Datagram *datagram = new IPv4Datagram("IPInject");
    datagram->setVersion(4);
    datagram->setHeaderLength(20);

    if (direction == DIRECTION_INBOUND) {
        datagram->setSrcAddress(remoteAddr.toIPv4());
        datagram->setDestAddress(localAddr.toIPv4());
        datagram->setIdentification(pdapp->getIdInbound());
        pdapp->increaseIdInbound();
    } else if (direction == DIRECTION_OUTBOUND) {
        datagram->setSrcAddress(localAddr.toIPv4());
        datagram->setDestAddress(remoteAddr.toIPv4());
        datagram->setIdentification(pdapp->getIdOutbound());
        pdapp->increaseIdOutbound();
    } else
        throw cRuntimeError("Unknown direction type %d", direction);
    datagram->setTransportProtocol(protocol);
    datagram->setTimeToLive(31);
    datagram->setMoreFragments(0);
    datagram->setDontFragment(0);
    datagram->setFragmentOffset(0);
    datagram->setTypeOfService(0);
    datagram->setByteLength(20);
    return datagram;
}


cPacket* PacketDrill::buildUDPPacket(int address_family, enum direction_t direction,
                                     uint16 udp_payload_bytes, char **error)
{
    PacketDrillApp *app = PacketDrill::pdapp;
    UDPPacket *udpPacket = new UDPPacket("UDPInject");
    udpPacket->setByteLength(8);
    cPacket *payload = new cPacket("payload");
    payload->setByteLength(udp_payload_bytes);
    udpPacket->encapsulate(payload);
    IPv4Datagram *ipDatagram = PacketDrill::makeIPPacket(IP_PROT_UDP, direction, app->getLocalAddress(), app->getRemoteAddress());
    if (direction == DIRECTION_INBOUND) {
        udpPacket->setSourcePort(app->getRemotePort());
        udpPacket->setDestinationPort(app->getLocalPort());
        udpPacket->setName("UDPInbound");
    } else if (direction == DIRECTION_OUTBOUND) {
        udpPacket->setSourcePort(app->getLocalPort());
        udpPacket->setDestinationPort(app->getRemotePort());
        udpPacket->setName("UDPOutbound");
    } else
        throw cRuntimeError("Unknown direction");

    ipDatagram->encapsulate(udpPacket);
    cPacket* pkt = ipDatagram->dup();
    delete ipDatagram;
    return pkt;
}


TCPOption *setOptionValues(PacketDrillTcpOption* opt)
{
    unsigned char length = opt->getLength();
    switch (opt->getKind()) {
        case TCPOPT_EOL: // EOL
            return new TCPOptionEnd();
        case TCPOPT_NOP: // NOP
            return new TCPOptionNop();
        case TCPOPT_MAXSEG:
            if (length == 4) {
                auto *option = new TCPOptionMaxSegmentSize();
                option->setLength(length);
                option->setMaxSegmentSize(opt->getMss());
                return option;
            }
            break;
        case TCPOPT_WINDOW:
            if (length == 3) {
                auto *option = new TCPOptionWindowScale();
                option->setLength(length);
                option->setWindowScale(opt->getWindowScale());
                return option;
            }
            break;
        case TCPOPT_SACK_PERMITTED:
            if (length == 2) {
                auto *option = new TCPOptionSackPermitted();
                option->setLength(length);
                return option;
            }
            break;
        case TCPOPT_SACK:
            if (length > 2 && (length % 8) == 2) {
                auto *option = new TCPOptionSack();
                option->setLength(length);
                option->setSackItemArraySize(length / 8);
                unsigned int count = 0;
                for (int i = 0; i < 2 * opt->getBlockList()->getLength(); i += 2) {
                    SackItem si;
                    si.setStart(((PacketDrillStruct *) opt->getBlockList()->get(i))->getValue1());
                    si.setEnd(((PacketDrillStruct *) opt->getBlockList()->get(i))->getValue2());
                    option->setSackItem(count++, si);
                }
                return option;
            }
            break;
        case TCPOPT_TIMESTAMP:
            if (length == 10) {
                auto *option = new TCPOptionTimestamp();
                option->setLength(length);
                option->setSenderTimestamp(opt->getVal());
                option->setEchoedTimestamp(opt->getEcr());
                return option;
            }
            break;
        default:
            EV_INFO << "TCP option is not supported (yet).";
            break;
    } // switch
    auto *option = new TCPOptionUnknown();
    option->setKind(opt->getKind());
    option->setLength(length);
    if (length > 2)
        option->setBytesArraySize(length - 2);
    for (unsigned int i = 2; i < length; i++)
        option->setBytes(i-2, length);
    return option;
}

cPacket* PacketDrill::buildTCPPacket(int address_family, enum direction_t direction, const char *flags,
                                     uint32 startSequence, uint16 tcpPayloadBytes, uint32 ackSequence,
                                     int32 window, cQueue *tcpOptions, char **error)
{
    PacketDrillApp *app = PacketDrill::pdapp;
    TCPSegment *tcpseg = new TCPSegment("TCPInject");

    // fill TCP header structure
    if (direction == DIRECTION_INBOUND) {
        tcpseg->setSrcPort(app->getRemotePort());
        tcpseg->setDestPort(app->getLocalPort());
        tcpseg->setName("TCPInbound");
    } else if (direction == DIRECTION_OUTBOUND) {
        tcpseg->setSrcPort(app->getLocalPort());
        tcpseg->setDestPort(app->getRemotePort());
        tcpseg->setName("TCPOutbound");
    }
    tcpseg->setSequenceNo(startSequence);
    tcpseg->setAckNo(ackSequence);

    tcpseg->setHeaderLength(TCP_HEADER_OCTETS);

    // set flags
    tcpseg->setFinBit(strchr(flags, 'F'));
    tcpseg->setSynBit(strchr(flags, 'S'));
    tcpseg->setRstBit(strchr(flags, 'R'));
    tcpseg->setPshBit(strchr(flags, 'P'));
    tcpseg->setAckBit(strchr(flags, '.'));
    tcpseg->setUrgBit(0);
    if (tcpseg->getSynBit() && !tcpseg->getAckBit())
        tcpseg->setName("Inject SYN");
    else if (tcpseg->getSynBit() && tcpseg->getAckBit())
        tcpseg->setName("Inject SYN ACK");
    else if (!tcpseg->getSynBit() && tcpseg->getAckBit() && !tcpseg->getPshBit())
        tcpseg->setName("Inject ACK");
    else if (tcpseg->getPshBit())
        tcpseg->setName("Inject PUSH");
    else if (tcpseg->getFinBit())
        tcpseg->setName("Inject FIN");

    tcpseg->setWindow(window);
    tcpseg->setUrgentPointer(0);
    // Checksum (header checksum): modelled by cMessage::hasBitError()

    uint16 tcpOptionsLength = 0;

    if (tcpOptions && tcpOptions->getLength() > 0) { // options present?
        TCPOption *option;
        uint16 optionsCounter = 0;

        for (cQueue::Iterator iter(*tcpOptions); !iter.end(); iter++) {
            PacketDrillTcpOption* opt = (PacketDrillTcpOption*)(*iter);

            option = setOptionValues(opt);
            tcpOptionsLength += opt->getLength();
            // write option to tcp header
            tcpseg->addHeaderOption(option);
            optionsCounter++;
        } // for
    } // if options present
    tcpseg->setHeaderLength(tcpseg->getHeaderLength() + tcpOptionsLength);
    tcpseg->setByteLength(tcpseg->getHeaderLength() + tcpPayloadBytes);
    tcpseg->setPayloadLength(tcpPayloadBytes);

    IPv4Datagram *ipDatagram = PacketDrill::makeIPPacket(IP_PROT_TCP, direction, app->getLocalAddress(),
            app->getRemoteAddress());
    ipDatagram->encapsulate(tcpseg);
    cPacket* pkt = ipDatagram->dup();
    delete tcpOptions;
    delete ipDatagram;
    return pkt;
}

cPacket* PacketDrill::buildSCTPPacket(int address_family, enum direction_t direction, cQueue *chunks)
{
    PacketDrillApp *app = PacketDrill::pdapp;
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setByteLength(SCTP_COMMON_HEADER);
    if (direction == DIRECTION_INBOUND)
    {
        sctpmsg->setSrcPort(app->getRemotePort());
        sctpmsg->setDestPort(app->getLocalPort());
        sctpmsg->setTag(app->getPeerVTag());
        sctpmsg->setName("SCTPInbound");
    }
    else if (direction == DIRECTION_OUTBOUND)
    {
        sctpmsg->setSrcPort(app->getLocalPort());
        sctpmsg->setDestPort(app->getRemotePort());
        sctpmsg->setTag(app->getLocalVTag());
        sctpmsg->setName("SCTPOutbound");
    }
    sctpmsg->setChecksumOk(true);

    for (cQueue::Iterator iter(*chunks); !iter.end(); iter++) {
        PacketDrillSctpChunk *chunk = (PacketDrillSctpChunk *)(*iter);
        sctpmsg->addChunk(chunk->getChunk());
    }

    IPv4Datagram *ipDatagram = PacketDrill::makeIPPacket(IP_PROT_SCTP, direction, app->getLocalAddress(),
            app->getRemoteAddress());
    ipDatagram->encapsulate(sctpmsg);
    cPacket* pkt = ipDatagram->dup();
    delete ipDatagram;
    delete chunks;
    return pkt;
}

PacketDrillSctpChunk* PacketDrill::buildDataChunk(int64 flgs, int64 len, int64 tsn, int64 sid, int64 ssn, int64 ppid)
{
    uint32 flags = 0;
    SCTPDataChunk *datachunk = new SCTPDataChunk();
    datachunk->setChunkType(DATA);
    datachunk->setName("PacketDrillDATA");

    if (flgs != -1) {
        if (flgs & SCTP_DATA_CHUNK_B_BIT) {
            datachunk->setBBit(1);
        }
        if (flgs & SCTP_DATA_CHUNK_E_BIT) {
            datachunk->setEBit(1);
        }
    } else {
        flags |= FLAG_CHUNK_FLAGS_NOCHECK;
    }

    if (tsn == -1) {
        datachunk->setTsn(0);
        flags |= FLAG_DATA_CHUNK_TSN_NOCHECK;
    } else {
        datachunk->setTsn((uint32) tsn);
    }

    if (sid == -1) {
        datachunk->setSid(0);
        flags |= FLAG_DATA_CHUNK_SID_NOCHECK;
    } else {
        datachunk->setSid((uint16) sid);
    }

    if (ssn == -1) {
        datachunk->setSsn(0);
        flags |= FLAG_DATA_CHUNK_SSN_NOCHECK;
    } else {
        datachunk->setSsn((uint16) ssn);
    }

    if (ppid == -1) {
        datachunk->setPpid(0);
        flags |= FLAG_DATA_CHUNK_PPID_NOCHECK;
    } else {
        datachunk->setPpid((uint32) ppid);
    }
// ToDo: Padding
    if (len != -1) {
        datachunk->setByteLength(SCTP_DATA_CHUNK_LENGTH);
        SCTPSimpleMessage* msg = new SCTPSimpleMessage("payload");
        uint32 sendBytes = len - SCTP_DATA_CHUNK_LENGTH;
        msg->setDataArraySize(sendBytes);
        for (int i = 0; i < sendBytes; i++)
            msg->setData(i, 'a');
        msg->setDataLen(sendBytes);
        msg->setEncaps(false);
        msg->setByteLength(sendBytes);
        datachunk->encapsulate(msg);
    } else {
        flags |= FLAG_CHUNK_LENGTH_NOCHECK;
        flags |= FLAG_CHUNK_VALUE_NOCHECK;
        datachunk->setByteLength(SCTP_DATA_CHUNK_LENGTH);
    }
    datachunk->setFlags(flags);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(DATA, (SCTPChunk *)datachunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildInitChunk(int64 flgs, int64 tag, int64 a_rwnd, int64 os, int64 is, int64 tsn, cQueue *parameters)
{
    uint32 flags = 0;
    uint16 length = 0;
    SCTPInitChunk *initchunk = new SCTPInitChunk();
    initchunk->setChunkType(INIT);
    initchunk->setName("INIT");

    if (tag == -1) {
        initchunk->setInitTag(0);
        flags |= FLAG_INIT_CHUNK_TAG_NOCHECK;
    } else {
        initchunk->setInitTag((uint32) tag);
    }

    if (a_rwnd == -1) {
        initchunk->setA_rwnd(0);
        flags |= FLAG_INIT_CHUNK_A_RWND_NOCHECK;
    } else {
        initchunk->setA_rwnd((uint32) a_rwnd);
    }

    if (is == -1) {
        initchunk->setNoInStreams(0);
        flags |= FLAG_INIT_CHUNK_IS_NOCHECK;
    } else {
        initchunk->setNoInStreams((uint16) is);
    }

    if (os == -1) {
        initchunk->setNoOutStreams(0);
        flags |= FLAG_INIT_CHUNK_OS_NOCHECK;
    } else {
        initchunk->setNoOutStreams((uint16) os);
    }

    if (tsn == -1) {
        initchunk->setInitTSN(0);
        flags |= FLAG_INIT_CHUNK_TSN_NOCHECK;
    } else {
        initchunk->setInitTSN((uint32) tsn);
    }

    if (parameters != nullptr) {
        PacketDrillSctpParameter *parameter;
        uint16 parLen = 0;
        for (cQueue::Iterator iter(*parameters); !iter.end(); iter++) {
            parameter = (PacketDrillSctpParameter*) (*iter);
            printf("parameter type=%d\n", parameter->getType());
            switch (parameter->getType()) {
                case SUPPORTED_EXTENSIONS: {
                    printf("SUPPORTED_EXTENSIONS\n");
                    ByteArray *ba = (ByteArray *)(parameter->getByteList());
                    parLen = ba->getDataArraySize();
                    initchunk->setSepChunksArraySize(parLen);
                    for (int i = 0; i < parLen; i++) {
                        initchunk->setSepChunks(i, ba->getData(i));
                    }
                    if (parLen > 0) {
                        length += ADD_PADDING(SCTP_SUPPORTED_EXTENSIONS_PARAMETER_LENGTH + parLen);
                    }
                    break;
                }
                default: printf("Parameter type not implemented\n");
            }
        }
    }
    initchunk->setAddressesArraySize(0);
    initchunk->setUnrecognizedParametersArraySize(0);
    initchunk->setFlags(flags);
    initchunk->setByteLength(SCTP_INIT_CHUNK_LENGTH + length);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(INIT, (SCTPChunk *)initchunk);
    delete parameters;
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildInitAckChunk(int64 flgs, int64 tag, int64 a_rwnd, int64 os, int64 is, int64 tsn, cQueue *parameters)
{
    uint32 flags = 0;
    SCTPInitAckChunk *initackchunk = new SCTPInitAckChunk();
    initackchunk->setChunkType(INIT_ACK);
    initackchunk->setName("INIT_ACK");

    if (tag == -1) {
        initackchunk->setInitTag(0);
        flags |= FLAG_INIT_ACK_CHUNK_TAG_NOCHECK;
    } else {
        initackchunk->setInitTag((uint32) tag);
    }

    if (a_rwnd == -1) {
        initackchunk->setA_rwnd(0);
        flags |= FLAG_INIT_ACK_CHUNK_A_RWND_NOCHECK;
    } else {
        initackchunk->setA_rwnd((uint32) a_rwnd);
    }

    if (is == -1) {
        initackchunk->setNoInStreams(0);
        flags |= FLAG_INIT_ACK_CHUNK_IS_NOCHECK;
    } else {
        initackchunk->setNoInStreams((uint16) is);
    }

    if (os == -1) {
        initackchunk->setNoOutStreams(0);
        flags |= FLAG_INIT_ACK_CHUNK_OS_NOCHECK;
    } else {
        initackchunk->setNoOutStreams((uint16) os);
    }

    if (tsn == -1) {
        initackchunk->setInitTSN(0);
        flags |= FLAG_INIT_ACK_CHUNK_TSN_NOCHECK;
    } else {
        initackchunk->setInitTSN((uint32) tsn);
    }

    initackchunk->setAddressesArraySize(0);
    initackchunk->setUnrecognizedParametersArraySize(0);

    initackchunk->setCookieArraySize(32);
    for (int i = 0; i < 32; i++) {
        initackchunk->setCookie(i, 'A');
    }

    initackchunk->setFlags(flags);
    initackchunk->setByteLength(SCTP_INIT_CHUNK_LENGTH + 36);
    delete parameters;
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(INIT_ACK, (SCTPChunk *)initackchunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildSackChunk(int64 flgs, int64 cum_tsn, int64 a_rwnd, cQueue *gaps, cQueue *dups)
{
    SCTPSackChunk* sackchunk = new SCTPSackChunk();
    sackchunk->setChunkType(SACK);
    sackchunk->setName("SACK");
    uint32 flags = 0;

    if (cum_tsn == -1) {
        sackchunk->setCumTsnAck(0);
        flags |= FLAG_SACK_CHUNK_CUM_TSN_NOCHECK;
    } else {
        sackchunk->setCumTsnAck((uint32) cum_tsn);
    }

    if (a_rwnd == -1) {
        sackchunk->setA_rwnd(0);
        flags |= FLAG_SACK_CHUNK_A_RWND_NOCHECK;
    } else {
        sackchunk->setA_rwnd((uint32) a_rwnd);
    }

    if (gaps == NULL) {
        sackchunk->setNumGaps(0);
        flags |= FLAG_CHUNK_LENGTH_NOCHECK;
        flags |= FLAG_SACK_CHUNK_GAP_BLOCKS_NOCHECK;
    } else if (gaps->getLength() != 0) {
        gaps->setName("gapList");
        int num = 0;
        PacketDrillStruct* gap;
        sackchunk->setNumGaps(gaps->getLength());
        sackchunk->setGapStartArraySize(gaps->getLength());
        sackchunk->setGapStopArraySize(gaps->getLength());
        for (cQueue::Iterator iter(*gaps); !iter.end(); iter++) {
            gap = (PacketDrillStruct*)(*iter);
            sackchunk->setGapStart(num, gap->getValue1());
            sackchunk->setGapStop(num, gap->getValue2());
            num++;
        }
        delete gaps;
    } else {
        sackchunk->setNumGaps(0);
        delete gaps;
    }

    if (dups == NULL) {
        sackchunk->setNumDupTsns(0);
        flags |= FLAG_CHUNK_LENGTH_NOCHECK;
        flags |= FLAG_SACK_CHUNK_DUP_TSNS_NOCHECK;
    } else if (dups->getLength() != 0) {
        int num = 0;
        PacketDrillStruct* tsn;
        sackchunk->setNumDupTsns(dups->getLength());
        sackchunk->setDupTsnsArraySize(dups->getLength());

        for (cQueue::Iterator iter(*dups); !iter.end(); iter++) {
            tsn = (PacketDrillStruct*)(*iter);
            sackchunk->setDupTsns(num, tsn->getValue1());
            num++;
        }
        delete dups;
    } else {
        sackchunk->setNumDupTsns(0);
        delete dups;
    }
    sackchunk->setByteLength(SCTP_SACK_CHUNK_LENGTH + (sackchunk->getNumGaps() + sackchunk->getNumDupTsns()) * 4);
    sackchunk->setFlags(flags);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(SACK, (SCTPChunk *)sackchunk);
    return sctpchunk;

}

PacketDrillSctpChunk* PacketDrill::buildCookieEchoChunk(int64 flgs, int64 len, PacketDrillBytes *cookie)
{
    SCTPCookieEchoChunk *cookieechochunk = new SCTPCookieEchoChunk();
    cookieechochunk->setChunkType(COOKIE_ECHO);
    cookieechochunk->setName("COOKIE_ECHO");
    uint32 flags = 0;

    if (cookie) {

    }
    else
    {
        flags |= FLAG_CHUNK_VALUE_NOCHECK;
        cookieechochunk->setCookieArraySize(0);
    }
    cookieechochunk->setUnrecognizedParametersArraySize(0);
    cookieechochunk->setByteLength(SCTP_COOKIE_ACK_LENGTH);
    cookieechochunk->setFlags(flags);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(COOKIE_ECHO, (SCTPChunk *)cookieechochunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildCookieAckChunk(int64 flgs)
{
    SCTPCookieAckChunk *cookieAckChunk = new SCTPCookieAckChunk("Cookie_Ack");
    cookieAckChunk->setChunkType(COOKIE_ACK);
    cookieAckChunk->setByteLength(SCTP_COOKIE_ACK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(COOKIE_ACK, (SCTPChunk *)cookieAckChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildShutdownChunk(int64 flgs, int64 cum_tsn)
{
    SCTPShutdownChunk *shutdownchunk = new SCTPShutdownChunk();
    shutdownchunk->setChunkType(SHUTDOWN);
    shutdownchunk->setName("SHUTDOWN");
    uint32 flags = 0;

    if (cum_tsn == -1) {
        flags |= FLAG_SHUTDOWN_CHUNK_CUM_TSN_NOCHECK;
        shutdownchunk->setCumTsnAck(0);
    } else {
        shutdownchunk->setCumTsnAck((uint32) cum_tsn);
    }
    shutdownchunk->setFlags(flags);
    shutdownchunk->setByteLength(SCTP_SHUTDOWN_CHUNK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(SHUTDOWN, (SCTPChunk *)shutdownchunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildShutdownAckChunk(int64 flgs)
{
    SCTPShutdownAckChunk *shutdownAckChunk = new SCTPShutdownAckChunk("Shutdown_Ack");
    shutdownAckChunk->setChunkType(SHUTDOWN_ACK);
    shutdownAckChunk->setByteLength(SCTP_COOKIE_ACK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(SHUTDOWN_ACK, (SCTPChunk *)shutdownAckChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildShutdownCompleteChunk(int64 flgs)
{
    SCTPShutdownCompleteChunk *shutdowncompletechunk = new SCTPShutdownCompleteChunk();
    shutdowncompletechunk->setChunkType(SHUTDOWN_COMPLETE);
    shutdowncompletechunk->setName("SHUTDOWN_COMPLETE");

    if (flgs != -1) {
        printf("T-Bit\n");
        shutdowncompletechunk->setTBit(flgs);
    } else {
        flgs |= FLAG_CHUNK_FLAGS_NOCHECK;
    }
    shutdowncompletechunk->setByteLength(SCTP_SHUTDOWN_ACK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(SHUTDOWN_COMPLETE, (SCTPChunk *)shutdowncompletechunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildAbortChunk(int64 flgs)
{
    SCTPAbortChunk *abortChunk = new SCTPAbortChunk("Abort");
    abortChunk->setChunkType(ABORT);

    if (flgs != -1) {
        abortChunk->setT_Bit(flgs);
    } else {
        flgs |= FLAG_CHUNK_FLAGS_NOCHECK;
    }
    abortChunk->setByteLength(SCTP_ABORT_CHUNK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(ABORT, (SCTPChunk *)abortChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildHeartbeatChunk(int64 flgs, PacketDrillSctpParameter *info)
{
    SCTPHeartbeatChunk *heartbeatChunk = new SCTPHeartbeatChunk();
    heartbeatChunk->setChunkType(HEARTBEAT);
    assert(info == NULL ||
       info->getLength() + SCTP_HEARTBEAT_CHUNK_LENGTH <= MAX_SCTP_CHUNK_BYTES);
    if (info && info->getLength() > 0)
    {
        uint32 flgs = info->getFlags();
        uint16 length =  0;
        if (flgs & FLAG_CHUNK_VALUE_NOCHECK) {
            if (flgs & FLAG_CHUNK_LENGTH_NOCHECK) {
                length = 12;
            } else {
                length = info->getLength();
            }
            if (length >= 12) {
                heartbeatChunk->setInfoArraySize(length - 4);
                heartbeatChunk->setRemoteAddr(pdapp->getRemoteAddress());
                heartbeatChunk->setTimeField(simTime());
                for (int i = 0; i < length - 12; i++) {
                    heartbeatChunk->setInfo(i, 'x');
                }
                heartbeatChunk->setByteLength(SCTP_HEARTBEAT_CHUNK_LENGTH + length);
            }
        } else {
            printf("Take Info Parameter not yet implemented\n");
        }
    } else {
        heartbeatChunk->setRemoteAddr(pdapp->getRemoteAddress());
        heartbeatChunk->setTimeField(simTime());
        heartbeatChunk->setByteLength(SCTP_HEARTBEAT_CHUNK_LENGTH + 12);
    }
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(HEARTBEAT, (SCTPChunk *)heartbeatChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildHeartbeatAckChunk(int64 flgs, PacketDrillSctpParameter *info)
{
    SCTPHeartbeatAckChunk *heartbeatAckChunk = new SCTPHeartbeatAckChunk();
    heartbeatAckChunk->setChunkType(HEARTBEAT_ACK);
    assert(info == NULL ||
       info->getLength() + SCTP_HEARTBEAT_CHUNK_LENGTH <= MAX_SCTP_CHUNK_BYTES);
    if (info && info->getLength() > 0)
    {
        printf("not implemented yet\n");
    } else {
        heartbeatAckChunk->setRemoteAddr(pdapp->getRemoteAddress());
        heartbeatAckChunk->setTimeField(pdapp->getPeerHeartbeatTime());
        heartbeatAckChunk->setByteLength(SCTP_HEARTBEAT_CHUNK_LENGTH + 12);
    }
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(HEARTBEAT_ACK, (SCTPChunk *)heartbeatAckChunk);
    return sctpchunk;
}

int PacketDrill::evaluate(PacketDrillExpression *in, PacketDrillExpression *out, char **error)
{
    int result = STATUS_OK;
    int64 number;
    out->setType(in->getType());

    if ((in->getType() <= EXPR_NONE) || (in->getType() >= NUM_EXPR_TYPES)) {
        EV_ERROR << "bad expression type: " << in->getType() << endl;
        return STATUS_ERR;
    }
    switch (in->getType()) {
    case EXPR_ELLIPSIS:
        break;

    case EXPR_INTEGER:
        out->setNum(in->getNum());
        break;

    case EXPR_WORD:
        out->setType(EXPR_INTEGER);
        if (in->symbolToInt(in->getString(), &number, error))
            return STATUS_ERR;
        out->setNum(number);
        break;

    case EXPR_SCTP_INITMSG:
        assert(in->getType() == EXPR_SCTP_INITMSG);
        assert(out->getType() == EXPR_SCTP_INITMSG);
        out->setInitmsg(in->getInitmsg());
        break;

    case EXPR_SCTP_RTOINFO:
        assert(in->getType() == EXPR_SCTP_RTOINFO);
        assert(out->getType() == EXPR_SCTP_RTOINFO);
        out->setRtoinfo(in->getRtoinfo());
        break;

    case EXPR_SCTP_SACKINFO:
        assert(in->getType() == EXPR_SCTP_SACKINFO);
        assert(out->getType() == EXPR_SCTP_SACKINFO);
        out->setSackinfo(in->getSackinfo());
        break;

    case EXPR_SCTP_ASSOCVAL:
        assert(in->getType() == EXPR_SCTP_ASSOCVAL);
        assert(out->getType() == EXPR_SCTP_ASSOCVAL);
        out->setAssocval(in->getAssocval());
        break;

    case EXPR_STRING:
        if (out->unescapeCstringExpression(in->getString(), error))
            return STATUS_ERR;
        break;

    case EXPR_BINARY:
        result = evaluate_binary_expression(in, out, error);
        break;

    case EXPR_LIST:
        result = evaluateListExpression(in, out, error);
        break;

    case EXPR_NONE:
        break;

    default:
        EV_INFO << "type " << in->getType() << " not implemented\n";
    }
    return result;
}

/* Return a copy of the given expression list with each expression
 * evaluated (e.g. symbols resolved to ints). On failure, return NULL
 * and fill in *error.
 */
int PacketDrill::evaluateExpressionList(cQueue *in_list, cQueue *out_list, char **error)
{
    cQueue *node_ptr = out_list;
    for (cQueue::Iterator it(*in_list); !it.end(); it++) {
        PacketDrillExpression *outExpr = new PacketDrillExpression(((PacketDrillExpression *)(*it))->getType());
        if (evaluate((PacketDrillExpression *)(*it), outExpr, error)) {
            delete(outExpr);
            return STATUS_ERR;
        }
        node_ptr->insert(outExpr);
    }
    return STATUS_OK;
}

int PacketDrill::evaluate_binary_expression(PacketDrillExpression *in, PacketDrillExpression *out, char **error)
{
    int result = STATUS_ERR;
    assert(in->getType() == EXPR_BINARY);
    assert(in->getBinary());
    out->setType(EXPR_INTEGER);

    PacketDrillExpression *lhs = nullptr;
    PacketDrillExpression *rhs = nullptr;
    if ((evaluate(in->getBinary()->lhs, lhs, error)) ||
        (evaluate(in->getBinary()->rhs, rhs, error))) {
        delete(rhs);
        delete(lhs);
        return result;
    }
    if (strcmp("|", in->getBinary()->op) == 0) {
        if (lhs->getType() != EXPR_INTEGER) {
            EV_ERROR << "left hand side of | not an integer\n";
        } else if (rhs->getType() != EXPR_INTEGER) {
            EV_ERROR << "right hand side of | not an integer\n";
        } else {
            out->setNum(lhs->getNum() | rhs->getNum());
            result = STATUS_OK;
        }
    } else {
        EV_ERROR << "bad binary operator '" << in->getBinary()->op << "'\n";
    }
    delete(rhs);
    delete(lhs);
    return result;
}

int PacketDrill::evaluateListExpression(PacketDrillExpression *in, PacketDrillExpression *out, char **error)
{
    assert(in->getType() == EXPR_LIST);
    assert(out->getType() == EXPR_LIST);

    out->setList(new cQueue("listExpression"));
    return evaluateExpressionList(in->getList(), out->getList(), error);
}
