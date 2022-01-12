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


#include <assert.h>
#include <cinttypes>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inet/applications/packetdrill/PacketDrill.h"
#include "inet/applications/packetdrill/PacketDrillUtils.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

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

Ptr<Ipv4Header> PacketDrill::makeIpv4Header(IpProtocolId protocol, enum direction_t direction, L3Address localAddr, L3Address remoteAddr)
{
    auto ipv4Header = makeShared<Ipv4Header>();
    ipv4Header->setVersion(4);
    ipv4Header->setHeaderLength(IPv4_MIN_HEADER_LENGTH);

    if (direction == DIRECTION_INBOUND) {
        ipv4Header->setSrcAddress(remoteAddr.toIpv4());
        ipv4Header->setDestAddress(localAddr.toIpv4());
        ipv4Header->setIdentification(pdapp->getIdInbound());
        pdapp->increaseIdInbound();
    } else if (direction == DIRECTION_OUTBOUND) {
        ipv4Header->setSrcAddress(localAddr.toIpv4());
        ipv4Header->setDestAddress(remoteAddr.toIpv4());
        ipv4Header->setIdentification(pdapp->getIdOutbound());
        pdapp->increaseIdOutbound();
    } else
        throw cRuntimeError("Unknown direction type %d", direction);
    ipv4Header->setProtocolId(protocol);
    ipv4Header->setTimeToLive(32);
    ipv4Header->setMoreFragments(0);
    ipv4Header->setDontFragment(0);
    ipv4Header->setFragmentOffset(0);
    ipv4Header->setTypeOfService(0);
    ipv4Header->setChunkLength(IPv4_MIN_HEADER_LENGTH);
    ipv4Header->setTotalLengthField(IPv4_MIN_HEADER_LENGTH);

    return ipv4Header;
}

void PacketDrill::setIpv4HeaderCrc(Ptr<Ipv4Header> &ipv4Header)
{
    ipv4Header->setCrcMode(pdapp->getCrcMode());
    ipv4Header->setCrc(0);
    if (pdapp->getCrcMode() == CRC_COMPUTED) {
        MemoryOutputStream ipv4HeaderStream;
        Chunk::serialize(ipv4HeaderStream, ipv4Header);
        uint16_t crc = TcpIpChecksum::checksum(ipv4HeaderStream.getData());
        ipv4Header->setCrc(crc);
    }
}


Packet* PacketDrill::buildUDPPacket(int address_family, enum direction_t direction,
                                     uint16 udpPayloadBytes, char **error)
{
    PacketDrillApp *app = PacketDrill::pdapp;
    Packet *packet = new Packet("UDPInject");
    if (udpPayloadBytes) {
        auto payload = makeShared<ByteCountChunk>(B(udpPayloadBytes));
        packet->insertAtFront(payload);
    }
    auto udpHeader = makeShared<UdpHeader>();
    if (direction == DIRECTION_INBOUND) {
        udpHeader->setSourcePort(app->getRemotePort());
        udpHeader->setDestinationPort(app->getLocalPort());
        packet->setName("UDPInbound");
    } else if (direction == DIRECTION_OUTBOUND) {
        udpHeader->setSourcePort(app->getLocalPort());
        udpHeader->setDestinationPort(app->getRemotePort());
        packet->setName("UDPOutbound");
    } else
        throw cRuntimeError("Unknown direction");
    udpHeader->setCrcMode(CRC_DISABLED);
    udpHeader->setTotalLengthField(UDP_HEADER_LENGTH + B(udpPayloadBytes));
    packet->insertAtFront(udpHeader);
    auto ipHeader = PacketDrill::makeIpv4Header(IP_PROT_UDP, direction, app->getLocalAddress(), app->getRemoteAddress());
    ipHeader->setTotalLengthField(ipHeader->getTotalLengthField() + packet->getDataLength());
    PacketDrill::setIpv4HeaderCrc(ipHeader);
    packet->insertAtFront(ipHeader);
    return packet;
}


TcpOption *setOptionValues(PacketDrillTcpOption* opt)
{
    unsigned char length = opt->getLength();
    switch (opt->getKind()) {
        case TCPOPT_EOL: // EOL
            return new TcpOptionEnd();
        case TCPOPT_NOP: // NOP
            return new TcpOptionNop();
        case TCPOPT_MAXSEG:
            if (length == 4) {
                auto *option = new TcpOptionMaxSegmentSize();
                option->setLength(length);
                option->setMaxSegmentSize(opt->getMss());
                return option;
            }
            break;
        case TCPOPT_WINDOW:
            if (length == 3) {
                auto *option = new TcpOptionWindowScale();
                option->setLength(length);
                option->setWindowScale(opt->getWindowScale());
                return option;
            }
            break;
        case TCPOPT_SACK_PERMITTED:
            if (length == 2) {
                auto *option = new TcpOptionSackPermitted();
                option->setLength(length);
                return option;
            }
            break;
        case TCPOPT_SACK:
            if (length > 2 && (length % 8) == 2) {
                auto *option = new TcpOptionSack();
                option->setLength(length);
                option->setSackItemArraySize(length / 8);
                unsigned int count = 0;
                for (int i = 0; i < 2 * opt->getBlockList()->getLength(); i += 2) {
                    SackItem si;
                    PacketDrillStruct *pds = check_and_cast<PacketDrillStruct *>(opt->getBlockList()->get(i));
                    si.setStart(pds->getValue1());
                    si.setEnd(pds->getValue2());
                    option->setSackItem(count++, si);
                }
                return option;
            }
            break;
        case TCPOPT_TIMESTAMP:
            if (length == 10) {
                auto *option = new TcpOptionTimestamp();
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
    auto *option = new TcpOptionUnknown();
    option->setKind(static_cast<TcpOptionNumbers>(opt->getKind()));
    option->setLength(length);
    if (length > 2)
        option->setBytesArraySize(length - 2);
    for (unsigned int i = 2; i < length; i++)
        option->setBytes(i-2, length);
    return option;
}

Packet* PacketDrill::buildTCPPacket(int address_family, enum direction_t direction, const char *flags,
                                     uint32 startSequence, uint16 tcpPayloadBytes, uint32 ackSequence,
                                     int32 window, cQueue *tcpOptions, char **error)
{
    Packet *packet = new Packet("TCPInject");
    PacketDrillApp *app = PacketDrill::pdapp;
    if (tcpPayloadBytes) {
        auto payload = makeShared<ByteCountChunk>(B(tcpPayloadBytes));
        packet->insertAtFront(payload);
    }
    auto tcpHeader = makeShared<TcpHeader>();

    // fill TCP header structure
    if (direction == DIRECTION_INBOUND) {
        tcpHeader->setSrcPort(app->getRemotePort());
        tcpHeader->setDestPort(app->getLocalPort());
        packet->setName("TCPInbound");
    } else if (direction == DIRECTION_OUTBOUND) {
        tcpHeader->setSrcPort(app->getLocalPort());
        tcpHeader->setDestPort(app->getRemotePort());
        packet->setName("TCPOutbound");
    }
    tcpHeader->setSequenceNo(startSequence);
    tcpHeader->setAckNo(ackSequence);

    tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH);

    // set flags
    tcpHeader->setFinBit(strchr(flags, 'F'));
    tcpHeader->setSynBit(strchr(flags, 'S'));
    tcpHeader->setRstBit(strchr(flags, 'R'));
    tcpHeader->setPshBit(strchr(flags, 'P'));
    tcpHeader->setAckBit(strchr(flags, '.'));
    tcpHeader->setUrgBit(0);
    if (tcpHeader->getSynBit() && !tcpHeader->getAckBit())
        packet->setName("Inject SYN");
    else if (tcpHeader->getSynBit() && tcpHeader->getAckBit())
        packet->setName("Inject SYN ACK");
    else if (!tcpHeader->getSynBit() && tcpHeader->getAckBit() && !tcpHeader->getPshBit())
        packet->setName("Inject ACK");
    else if (tcpHeader->getPshBit())
        packet->setName("Inject PUSH");
    else if (tcpHeader->getFinBit())
        packet->setName("Inject FIN");

    tcpHeader->setWindow(window);
    tcpHeader->setUrgentPointer(0);
    // Checksum (header checksum): modelled by cMessage::hasBitError()

    if (tcpOptions && tcpOptions->getLength() > 0) { // options present?
        TcpOption *option;

        for (cQueue::Iterator iter(*tcpOptions); !iter.end(); iter++) {
            PacketDrillTcpOption* opt = check_and_cast<PacketDrillTcpOption*>(*iter);
            option = setOptionValues(opt);
            // write option to tcp header
            tcpHeader->appendHeaderOption(option);
        } // for
    } // if options present
    tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + B(tcpHeader->getHeaderOptionArrayLength()));
    tcpHeader->setChunkLength(TCP_MIN_HEADER_LENGTH + B(tcpHeader->getHeaderOptionArrayLength()));
    packet->insertAtFront(tcpHeader);

    auto ipHeader = PacketDrill::makeIpv4Header(IP_PROT_TCP, direction, app->getLocalAddress(),
            app->getRemoteAddress());
    ipHeader->setTotalLengthField(ipHeader->getTotalLengthField() + packet->getDataLength());
    PacketDrill::setIpv4HeaderCrc(ipHeader);
    packet->insertAtFront(ipHeader);
    delete tcpOptions;
    return packet;
}

Packet* PacketDrill::buildSCTPPacket(int address_family, enum direction_t direction, cQueue *chunks)
{
    Packet *packet = new Packet("SCTPInject");
    PacketDrillApp *app = PacketDrill::pdapp;
    auto sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    if (direction == DIRECTION_INBOUND) {
        sctpmsg->setSrcPort(app->getRemotePort());
        sctpmsg->setDestPort(app->getLocalPort());
        auto addressReq = packet->addTag<L3AddressReq>();
        addressReq->setSrcAddress(app->getRemoteAddress());
        addressReq->setDestAddress(app->getLocalAddress());
        sctpmsg->setVTag(app->getPeerVTag());
        packet->setName("SCTPInbound");
        for (cQueue::Iterator iter(*chunks); !iter.end(); iter++) {
            PacketDrillSctpChunk *chunk = check_and_cast<PacketDrillSctpChunk *>(*iter);
            SctpChunk *sctpchunk = chunk->getChunk();
            switch (chunk->getType()) {
                case SCTP_DATA_CHUNK_TYPE:
                    if (sctpchunk->getFlags() & FLAG_CHUNK_FLAGS_NOCHECK) {
                        printf("chunk flags must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_CHUNK_LENGTH_NOCHECK) {
                        printf("chunk length must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_DATA_CHUNK_TSN_NOCHECK) {
                        printf("TSN must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_DATA_CHUNK_SID_NOCHECK) {
                        printf("SID must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_DATA_CHUNK_SSN_NOCHECK) {
                        printf("SSN must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_DATA_CHUNK_PPID_NOCHECK) {
                        printf("PPID must be specified for inbound packets\n");
                        return nullptr;
                    }
                    break;
                case INIT:
                    if (sctpchunk->getFlags() & FLAG_INIT_CHUNK_TAG_NOCHECK) {
                        printf("TAG must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_INIT_CHUNK_A_RWND_NOCHECK) {
                        printf("A_RWND must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_INIT_CHUNK_OS_NOCHECK) {
                        printf("OS must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_INIT_CHUNK_IS_NOCHECK) {
                        printf("IS must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_INIT_CHUNK_OPT_PARAM_NOCHECK) {
                        printf("list of optional parameters must be specified for inbound packets\n");
                        return nullptr;
                    }
                    break;
               case INIT_ACK:
                    if (sctpchunk->getFlags() & FLAG_INIT_ACK_CHUNK_TAG_NOCHECK) {
                        printf("TAG must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_INIT_ACK_CHUNK_A_RWND_NOCHECK) {
                        printf("A_RWND must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_INIT_ACK_CHUNK_OS_NOCHECK) {
                        printf("OS must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_INIT_ACK_CHUNK_IS_NOCHECK) {
                        printf("IS must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_INIT_ACK_CHUNK_OPT_PARAM_NOCHECK) {
                        printf("list of optional parameters must be specified for inbound packets\n");
                        return nullptr;
                    }
                    break;
               case SCTP_SACK_CHUNK_TYPE:
                    if (sctpchunk->getFlags() & FLAG_SACK_CHUNK_CUM_TSN_NOCHECK) {
                        printf("CUM_TSN must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_SACK_CHUNK_A_RWND_NOCHECK) {
                        printf("A_RWND must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_SACK_CHUNK_GAP_BLOCKS_NOCHECK) {
                        printf("GAP_BLOCKS must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_SACK_CHUNK_DUP_TSNS_NOCHECK) {
                        printf("DUP_TSNS must be specified for inbound packets\n");
                        return nullptr;
                    }
                    break;
               case SCTP_ABORT_CHUNK_TYPE:
                    if (sctpchunk->getFlags() & FLAG_CHUNK_LENGTH_NOCHECK) {
                        printf("error causes must be specified for inbound packets\n");
                        return nullptr;
                    }
                    break;
               case SCTP_SHUTDOWN_CHUNK_TYPE:
                    if (sctpchunk->getFlags() & FLAG_SHUTDOWN_CHUNK_CUM_TSN_NOCHECK) {
                        printf("TSN must be specified for inbound packets\n");
                        return nullptr;
                    }
                    break;
               default:
                    if (sctpchunk->getFlags() & FLAG_CHUNK_TYPE_NOCHECK) {
                        printf("chunk type must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_CHUNK_FLAGS_NOCHECK) {
                        printf("chunk flags must be specified for inbound packets\n");
                        return nullptr;
                    }
                    if (sctpchunk->getFlags() & FLAG_CHUNK_LENGTH_NOCHECK) {
                        printf("chunk length must be specified for inbound packets\n");
                        return nullptr;
                    }
                    break;
            }
        }
    } else if (direction == DIRECTION_OUTBOUND) {
        sctpmsg->setSrcPort(app->getLocalPort());
        sctpmsg->setDestPort(app->getRemotePort());
        auto addressReq = packet->addTag<L3AddressReq>();
        addressReq->setSrcAddress(app->getLocalAddress());
        addressReq->setDestAddress(app->getRemoteAddress());
        sctpmsg->setVTag(app->getLocalVTag());
        packet->setName("SCTPOutbound");
        for (cQueue::Iterator iter(*chunks); !iter.end(); iter++) {
            PacketDrillSctpChunk *chunk = check_and_cast<PacketDrillSctpChunk *>(*iter);
            SctpChunk *sctpchunk = chunk->getChunk();
            switch (sctpchunk->getSctpChunkType()) {
                case SCTP_RECONFIG_CHUNK_TYPE:
                    SctpStreamResetChunk* reconfig = check_and_cast<SctpStreamResetChunk*>(sctpchunk);
                    for (unsigned int i = 0; i < reconfig->getParametersArraySize(); i++) {
                        const SctpParameter *parameter = reconfig->getParameters(i);
                        switch (parameter->getParameterType()) {
                            case OUTGOING_RESET_REQUEST_PARAMETER: {
                                printf("OUTGOING_RESET_REQUEST_PARAMETER\n");
                                auto outResetParam = check_and_cast<const SctpOutgoingSsnResetRequestParameter *>(parameter);
                                if (!(pdapp->findSeqNumMap(outResetParam->getSrReqSn()))) {
                                    pdapp->setSeqNumMap(outResetParam->getSrReqSn(), 0);
                                }
                                break;
                            }
                            case INCOMING_RESET_REQUEST_PARAMETER: {
                                printf("INCOMING_RESET_REQUEST_PARAMETER\n");
                                auto inResetParam = check_and_cast<const SctpIncomingSsnResetRequestParameter *>(parameter);
                                if (!(pdapp->findSeqNumMap(inResetParam->getSrReqSn()))) {
                                    pdapp->setSeqNumMap(inResetParam->getSrReqSn(), 0);
                                }
                                break;
                            }
                            case SSN_TSN_RESET_REQUEST_PARAMETER: {
                                printf("SSN_TSN_RESET_REQUEST_PARAMETER\n");
                                auto ssntsnResetParam = check_and_cast<const SctpSsnTsnResetRequestParameter *>(parameter);
                                if (!(pdapp->findSeqNumMap(ssntsnResetParam->getSrReqSn()))) {
                                    pdapp->setSeqNumMap(ssntsnResetParam->getSrReqSn(), 0);
                                }
                                break;
                            }
                            case ADD_OUTGOING_STREAMS_REQUEST_PARAMETER: {
                                printf("ADD_OUTGOING_STREAMS_REQUEST_PARAMETER\n");
                                auto addOutResetParam = check_and_cast<const SctpAddStreamsRequestParameter *>(parameter);
                                if (!(pdapp->findSeqNumMap(addOutResetParam->getSrReqSn()))) {
                                    pdapp->setSeqNumMap(addOutResetParam->getSrReqSn(), 0);
                                }
                                break;
                            }
                            case ADD_INCOMING_STREAMS_REQUEST_PARAMETER: {
                                printf("ADD_INCOMING_STREAMS_REQUEST_PARAMETER\n");
                                auto addInResetParam = check_and_cast<const SctpAddStreamsRequestParameter *>(parameter);
                                if (!(pdapp->findSeqNumMap(addInResetParam->getSrReqSn()))) {
                                    pdapp->setSeqNumMap(addInResetParam->getSrReqSn(), 0);
                                }
                                break;
                            }
                        }
                    }
                    break;
            }
        }
    }
    sctpmsg->setChecksumOk(true);
    sctpmsg->setCrcMode(pdapp->getCrcMode());
    sctpmsg->setCrc(0);

    for (cQueue::Iterator iter(*chunks); !iter.end(); iter++) {
        PacketDrillSctpChunk *chunk = check_and_cast<PacketDrillSctpChunk *>(*iter);
        sctpmsg->appendSctpChunks(chunk->getChunk());
    }

    insertTransportProtocolHeader(packet, Protocol::sctp, sctpmsg);
    auto ipHeader = PacketDrill::makeIpv4Header(IP_PROT_SCTP, direction, app->getLocalAddress(),
            app->getRemoteAddress());
    ipHeader->setTotalLengthField(ipHeader->getTotalLengthField() + packet->getDataLength());
    PacketDrill::setIpv4HeaderCrc(ipHeader);
    insertNetworkProtocolHeader(packet, Protocol::ipv4, ipHeader);
    EV_INFO << "SCTP packet " << packet << endl;
    delete chunks;
    return packet;
}

PacketDrillSctpChunk* PacketDrill::buildDataChunk(int64 flgs, int64 len, int64 tsn, int64 sid, int64 ssn, int64 ppid)
{
    uint32 flags = 0;
    SctpDataChunk *datachunk = new SctpDataChunk();
    datachunk->setSctpChunkType(DATA);
    datachunk->setName("PacketDrillDATA");

    if (flgs != -1) {
        if (flgs & SCTP_DATA_CHUNK_B_BIT) {
            datachunk->setBBit(1);
        }
        if (flgs & SCTP_DATA_CHUNK_E_BIT) {
            datachunk->setEBit(1);
        }
        if (flgs & SCTP_DATA_CHUNK_U_BIT) {
            datachunk->setUBit(1);
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
        SctpSimpleMessage* msg = new SctpSimpleMessage("payload");
        uint32 sendBytes = len - SCTP_DATA_CHUNK_LENGTH;
        msg->setDataArraySize(sendBytes);
        for (uint32 i = 0; i < sendBytes; i++)
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
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(DATA, datachunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildInitChunk(int64 flgs, int64 tag, int64 a_rwnd, int64 os, int64 is, int64 tsn, cQueue *parameters)
{
    uint32 flags = 0;
    uint16 length = 0;
    SctpInitChunk *initchunk = new SctpInitChunk();
    initchunk->setSctpChunkType(INIT);
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
        initchunk->setInitTsn(0);
        flags |= FLAG_INIT_CHUNK_TSN_NOCHECK;
    } else {
        initchunk->setInitTsn(tsn);
    }

    if (parameters != nullptr) {
        PacketDrillSctpParameter *parameter;
        uint16 parLen = 0;
        for (cQueue::Iterator iter(*parameters); !iter.end(); iter++) {
            parameter = check_and_cast<PacketDrillSctpParameter*>(*iter);
            switch (parameter->getType()) {
                case SUPPORTED_EXTENSIONS: {
                    auto ba = parameter->getByteList();
                    parLen = ba->size();
                    initchunk->setSepChunksArraySize(parLen);
                    for (int i = 0; i < parLen; i++) {
                        initchunk->setSepChunks(i, ba->at(i));
                    }
                    if (parLen > 0) {
                        length += ADD_PADDING(SCTP_SUPPORTED_EXTENSIONS_PARAMETER_LENGTH + parLen);
                    }
                    break;
                }
                case SUPPORTED_ADDRESS_TYPES: {
                    flags |= FLAG_INIT_CHUNK_OPT_SUPPORTED_ADDRESS_TYPES_PARAM_NOCHECK;
                    cQueue *list = parameter->getList();
                    if (list != nullptr) {
                        for (cQueue::Iterator iter(*list); !iter.end(); iter++)
                            list->remove((*iter));
                        delete list;
                    }
                    break;
                }
                default: printf("Parameter type not implemented\n");
            }
        }
    } else {
        flags |= FLAG_INIT_CHUNK_OPT_PARAM_NOCHECK;
    }
    initchunk->setAddressesArraySize(0);
    initchunk->setUnrecognizedParametersArraySize(0);
    initchunk->setFlags(flags);
    initchunk->setByteLength(SCTP_INIT_CHUNK_LENGTH + length);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(INIT, initchunk);
    delete parameters;
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildInitAckChunk(int64 flgs, int64 tag, int64 a_rwnd, int64 os, int64 is, int64 tsn, cQueue *parameters)
{
    uint32 flags = 0;
    uint16 length = 0;
    SctpInitAckChunk *initackchunk = new SctpInitAckChunk();
    initackchunk->setSctpChunkType(INIT_ACK);
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
        initackchunk->setInitTsn(0);
        flags |= FLAG_INIT_ACK_CHUNK_TSN_NOCHECK;
    } else {
        initackchunk->setInitTsn((uint32) tsn);
    }
    if (parameters != nullptr) {
        PacketDrillSctpParameter *parameter;
        uint16 parLen = 0;
        for (cQueue::Iterator iter(*parameters); !iter.end(); iter++) {
            parameter = check_and_cast<PacketDrillSctpParameter*>(*iter);
            switch (parameter->getType()) {
                case SUPPORTED_EXTENSIONS: {
                    auto ba = parameter->getByteList();
                    parLen = ba->size();
                    initackchunk->setSepChunksArraySize(parLen);
                    for (int i = 0; i < parLen; i++) {
                        initackchunk->setSepChunks(i, (0x00ff & ba->at(i)));
                    }
                    if (parLen > 0) {
                        length += ADD_PADDING(SCTP_SUPPORTED_EXTENSIONS_PARAMETER_LENGTH + parLen);
                    }
                    break;
                }
                case SUPPORTED_ADDRESS_TYPES: {
                    flags |= FLAG_INIT_CHUNK_OPT_SUPPORTED_ADDRESS_TYPES_PARAM_NOCHECK;
                    cQueue *list = parameter->getList();
                    if (list != nullptr) {
                        for (cQueue::Iterator iter(*list); !iter.end(); iter++)
                            list->remove((*iter));
                        delete list;
                    }
                    break;
                }
                default: printf("Parameter type not implemented\n");
            }
        }
    } else {
        flags |= FLAG_INIT_ACK_CHUNK_OPT_PARAM_NOCHECK;
    }

    initackchunk->setAddressesArraySize(0);
    initackchunk->setUnrecognizedParametersArraySize(0);

    initackchunk->setCookieArraySize(32);
    for (int i = 0; i < 32; i++) {
        initackchunk->setCookie(i, 'A');
    }

    initackchunk->setFlags(flags);
    initackchunk->setByteLength(SCTP_INIT_CHUNK_LENGTH + 36 + length);
    delete parameters;
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(INIT_ACK, initackchunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildSackChunk(int64 flgs, int64 cum_tsn, int64 a_rwnd, cQueue *gaps, cQueue *dups)
{
    auto* sackchunk = new SctpSackChunk();
    sackchunk->setSctpChunkType(SACK);
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
            gap = check_and_cast<PacketDrillStruct*>(*iter);
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
            tsn = check_and_cast<PacketDrillStruct*>(*iter);
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
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(SACK, sackchunk);
    return sctpchunk;

}

PacketDrillSctpChunk* PacketDrill::buildCookieEchoChunk(int64 flgs, int64 len, PacketDrillBytes *cookie)
{
    SctpCookieEchoChunk *cookieechochunk = new SctpCookieEchoChunk();
    cookieechochunk->setSctpChunkType(COOKIE_ECHO);
    cookieechochunk->setName("COOKIE_ECHO");
    uint32 flags = 0;

    if (cookie) {
        printf("cookie present\n");
    }
    else
    {
        flags |= FLAG_CHUNK_VALUE_NOCHECK;
        cookieechochunk->setCookieArraySize(0);
    }
    cookieechochunk->setUnrecognizedParametersArraySize(0);
    cookieechochunk->setByteLength(SCTP_COOKIE_ACK_LENGTH);
    cookieechochunk->setFlags(flags);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(COOKIE_ECHO, cookieechochunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildCookieAckChunk(int64 flgs)
{
    SctpCookieAckChunk *cookieAckChunk = new SctpCookieAckChunk("Cookie_Ack");
    cookieAckChunk->setSctpChunkType(COOKIE_ACK);
    cookieAckChunk->setByteLength(SCTP_COOKIE_ACK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(COOKIE_ACK, cookieAckChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildShutdownChunk(int64 flgs, int64 cum_tsn)
{
    auto *shutdownchunk = new SctpShutdownChunk();
    shutdownchunk->setSctpChunkType(SHUTDOWN);
    shutdownchunk->setName("SHUTDOWN");
    uint32 flags = 0;

    if (cum_tsn == -1) {
        flags |= FLAG_SHUTDOWN_CHUNK_CUM_TSN_NOCHECK;
        shutdownchunk->setCumTsnAck(0);
    } else {
        shutdownchunk->setCumTsnAck(cum_tsn);
    }
    shutdownchunk->setFlags(flags);
    shutdownchunk->setByteLength(SCTP_SHUTDOWN_CHUNK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(SHUTDOWN, shutdownchunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildShutdownAckChunk(int64 flgs)
{
    auto *shutdownAckChunk = new SctpShutdownAckChunk("Shutdown_Ack");
    shutdownAckChunk->setSctpChunkType(SHUTDOWN_ACK);
    shutdownAckChunk->setByteLength(SCTP_COOKIE_ACK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(SHUTDOWN_ACK, shutdownAckChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildShutdownCompleteChunk(int64 flgs)
{
    auto *shutdowncompletechunk = new SctpShutdownCompleteChunk();
    shutdowncompletechunk->setSctpChunkType(SHUTDOWN_COMPLETE);
    shutdowncompletechunk->setName("SHUTDOWN_COMPLETE");

    if (flgs != -1) {
        shutdowncompletechunk->setTBit(flgs);
    } else {
        flgs |= FLAG_CHUNK_FLAGS_NOCHECK;
    }
    shutdowncompletechunk->setByteLength(SCTP_SHUTDOWN_ACK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(SHUTDOWN_COMPLETE, shutdowncompletechunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildAbortChunk(int64 flgs)
{
    auto *abortChunk = new SctpAbortChunk("Abort");
    abortChunk->setSctpChunkType(ABORT);

    if (flgs != -1) {
        abortChunk->setT_Bit(flgs);
    } else {
        flgs |= FLAG_CHUNK_FLAGS_NOCHECK;
    }
    abortChunk->setByteLength(SCTP_ABORT_CHUNK_LENGTH);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(ABORT, abortChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildErrorChunk(int64 flgs, cQueue *causes)
{
    PacketDrillStruct *errorcause;
    auto *errorChunk = new SctpErrorChunk("Error");
    errorChunk->setSctpChunkType(ERRORTYPE);
    errorChunk->setByteLength(SCTP_ERROR_CHUNK_LENGTH);
    for (cQueue::Iterator iter(*causes); !iter.end(); iter++) {
        errorcause = check_and_cast<PacketDrillStruct*>(*iter);
        auto *cause = new SctpSimpleErrorCauseParameter("Cause");
        cause->setParameterType(INVALID_STREAM_IDENTIFIER);
        cause->setByteLength(8);
        cause->setValue(errorcause->getValue2());
        errorChunk->addParameters(cause);
        causes->remove((*iter));
    }
    delete causes;
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(ERRORTYPE, errorChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildHeartbeatChunk(int64 flgs, PacketDrillSctpParameter *info)
{
    auto *heartbeatChunk = new SctpHeartbeatChunk();
    heartbeatChunk->setSctpChunkType(HEARTBEAT);
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
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(HEARTBEAT, heartbeatChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildHeartbeatAckChunk(int64 flgs, PacketDrillSctpParameter *info)
{
    auto *heartbeatAckChunk = new SctpHeartbeatAckChunk();
    heartbeatAckChunk->setSctpChunkType(HEARTBEAT_ACK);
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
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(HEARTBEAT_ACK, heartbeatAckChunk);
    return sctpchunk;
}

PacketDrillSctpChunk* PacketDrill::buildReconfigChunk(int64 flgs, cQueue *parameters)
{
    uint32 flags = 0;
    uint16 len = 0;
    auto *resetChunk = new SctpStreamResetChunk("RE_CONFIG");
    resetChunk->setSctpChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    if (parameters != nullptr) {
        PacketDrillSctpParameter *parameter;
        for (cQueue::Iterator iter(*parameters); !iter.end(); iter++) {
            parameter = check_and_cast<PacketDrillSctpParameter*>(*iter);
            switch (parameter->getType()) {
                case OUTGOING_RESET_REQUEST_PARAMETER: {
                    auto *outResetParam = new SctpOutgoingSsnResetRequestParameter("Outgoing_Request_Param");
                    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
                    PacketDrillStruct *content = (PacketDrillStruct *)parameter->getContent();
                    if (content->getValue1() == -1) {
                        outResetParam->setSrReqSn(0);
                        flags |= FLAG_RECONFIG_REQ_SN_NOCHECK;
                    } else {
                        outResetParam->setSrReqSn(content->getValue1());
                    }
                    if (content->getValue2() == -1) {
                        outResetParam->setSrResSn(0);
                        flags |= FLAG_RECONFIG_RESP_SN_NOCHECK;
                    } else {
                        outResetParam->setSrResSn(content->getValue2());
                    }
                    if (content->getValue3() == -1) {
                        outResetParam->setLastTsn(0);
                        flags |= FLAG_RECONFIG_LAST_TSN_NOCHECK;
                    } else {
                        outResetParam->setLastTsn(content->getValue3());
                    }
                    if (content->getStreams() != nullptr) {
                        outResetParam->setStreamNumbersArraySize(content->getStreams()->getLength());
                        unsigned int i = 0;
                        for (cQueue::Iterator it(*content->getStreams()); !it.end(); it++) {
                            if (check_and_cast<PacketDrillExpression *>(*it)->getNum() != -1)
                                outResetParam->setStreamNumbers(i++, check_and_cast<PacketDrillExpression *>(*it)->getNum());
                            else
                                outResetParam->setStreamNumbersArraySize(outResetParam->getStreamNumbersArraySize() - 1);
                        }
                        len = outResetParam->getStreamNumbersArraySize();
                        for (cQueue::Iterator iter(*content->getStreams()); !iter.end(); iter++)
                            content->getStreams()->remove((*iter));
                        delete content->getStreams();
                    }
                    outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + len * 2);
                    resetChunk->addParameter(outResetParam);
                    break;
                }
                case INCOMING_RESET_REQUEST_PARAMETER: {
                    auto *inResetParam = new SctpIncomingSsnResetRequestParameter("Incoming_Request_Param");
                    PacketDrillStruct *content = (PacketDrillStruct *)parameter->getContent();
                    inResetParam->setParameterType(INCOMING_RESET_REQUEST_PARAMETER);
                    inResetParam->setSrReqSn(content->getValue1());
                    if (content->getStreams() != nullptr) {
                        inResetParam->setStreamNumbersArraySize(content->getStreams()->getLength());
                        unsigned int i = 0;
                        for (cQueue::Iterator it(*content->getStreams()); !it.end(); it++) {
                            if (check_and_cast<PacketDrillExpression *>(*it)->getNum() != -1)
                                inResetParam->setStreamNumbers(i++, check_and_cast<PacketDrillExpression *>(*it)->getNum());
                            else
                                inResetParam->setStreamNumbersArraySize(inResetParam->getStreamNumbersArraySize() - 1);
                        }
                        for (cQueue::Iterator iter(*content->getStreams()); !iter.end(); iter++)
                            content->getStreams()->remove((*iter));
                        delete content->getStreams();
                    }
                    inResetParam->setByteLength(SCTP_INCOMING_RESET_REQUEST_PARAMETER_LENGTH + inResetParam->getStreamNumbersArraySize() * 2);
                    resetChunk->addParameter(inResetParam);
                    break;
                }
                case SSN_TSN_RESET_REQUEST_PARAMETER: {
                    auto *resetParam = new SctpSsnTsnResetRequestParameter("SSN_TSN_Request_Param");
                    PacketDrillStruct *content = (PacketDrillStruct *)parameter->getContent();
                    resetParam->setParameterType(SSN_TSN_RESET_REQUEST_PARAMETER);
                    resetParam->setSrReqSn(content->getValue1());
                    resetParam->setByteLength(SCTP_SSN_TSN_RESET_REQUEST_PARAMETER_LENGTH);
                    resetChunk->addParameter(resetParam);
                    break;
                }
                case STREAM_RESET_RESPONSE_PARAMETER: {
                    auto *responseParam = new SctpStreamResetResponseParameter("Response_Param");
                    PacketDrillStruct *content = (PacketDrillStruct *)parameter->getContent();
                    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
                    responseParam->setSrResSn(content->getValue1());
                    if (content->getValue2() == -1) {
                        responseParam->setResult(0);
                        flags |= FLAG_RECONFIG_RESULT_NOCHECK;
                    } else {
                        responseParam->setResult(content->getValue2());
                    }
                    uint32 len = SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH;
                    if (content->getValue3() != -2) {
                        if (content->getValue3() == -1) {
                            responseParam->setSendersNextTsn(0);
                            flags |= FLAG_RECONFIG_SENDER_NEXT_TSN_NOCHECK;
                        } else {
                            responseParam->setSendersNextTsn(content->getValue3());
                        }
                        if (content->getValue4() == -1) {
                            responseParam->setReceiversNextTsn(0);
                            flags |= FLAG_RECONFIG_RECEIVER_NEXT_TSN_NOCHECK;
                        } else {
                            responseParam->setReceiversNextTsn(content->getValue4());
                        }
                        len += 8;
                    }
                    responseParam->setByteLength(len);
                    resetChunk->addParameter(responseParam);
                    break;
                }
                case ADD_INCOMING_STREAMS_REQUEST_PARAMETER: {
                    auto *addInParam = new SctpAddStreamsRequestParameter("ADD_INCOMING_STREAMS_REQUEST_PARAMETER");
                    PacketDrillStruct *content = (PacketDrillStruct *)parameter->getContent();
                    addInParam->setParameterType(ADD_INCOMING_STREAMS_REQUEST_PARAMETER);
                    addInParam->setSrReqSn(content->getValue1());
                    addInParam->setNumberOfStreams(content->getValue2());
                    addInParam->setReserved(0);
                    addInParam->setByteLength(SCTP_ADD_STREAMS_REQUEST_PARAMETER_LENGTH);
                    resetChunk->addParameter(addInParam);
                    break;
                }
                case ADD_OUTGOING_STREAMS_REQUEST_PARAMETER: {
                    auto *addOutParam = new SctpAddStreamsRequestParameter("ADD_OUTGOING_STREAMS_REQUEST_PARAMETER");
                    PacketDrillStruct *content = (PacketDrillStruct *)parameter->getContent();
                    addOutParam->setParameterType(ADD_OUTGOING_STREAMS_REQUEST_PARAMETER);
                    addOutParam->setSrReqSn(content->getValue1());
                    addOutParam->setNumberOfStreams(content->getValue2());
                    addOutParam->setReserved(0);
                    addOutParam->setByteLength(SCTP_ADD_STREAMS_REQUEST_PARAMETER_LENGTH);
                    resetChunk->addParameter(addOutParam);
                    break;
                }
                default: printf("Parameter type not implemented\n");
            }
        }
        for (cQueue::Iterator iter(*parameters); !iter.end(); iter++)
            parameters->remove(*iter);
        delete parameters;
    }
    resetChunk->setFlags(flags);
    PacketDrillSctpChunk *sctpchunk = new PacketDrillSctpChunk(RE_CONFIG, resetChunk);
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

    case EXPR_SCTP_INITMSG: {
        assert(in->getType() == EXPR_SCTP_INITMSG);
        assert(out->getType() == EXPR_SCTP_INITMSG);
        struct sctp_initmsg_expr *expr = (struct sctp_initmsg_expr *)malloc(sizeof(struct sctp_initmsg_expr));
        expr->sinit_num_ostreams = new PacketDrillExpression(in->getInitmsg()->sinit_num_ostreams->getType());
        if (evaluate(in->getInitmsg()->sinit_num_ostreams, expr->sinit_num_ostreams, error)) {
            delete (expr->sinit_num_ostreams);
            free (expr);
            return STATUS_ERR;
        }
        expr->sinit_max_instreams = new PacketDrillExpression(in->getInitmsg()->sinit_max_instreams->getType());
        if (evaluate(in->getInitmsg()->sinit_max_instreams, expr->sinit_max_instreams, error)) {
            delete (expr->sinit_num_ostreams);
            delete (expr->sinit_max_instreams);
            free (expr);
            return STATUS_ERR;
        }
        expr->sinit_max_attempts = new PacketDrillExpression(in->getInitmsg()->sinit_max_attempts->getType());
        if (evaluate(in->getInitmsg()->sinit_max_attempts, expr->sinit_max_attempts, error)) {
            delete (expr->sinit_num_ostreams);
            delete (expr->sinit_max_instreams);
            delete (expr->sinit_max_attempts);
            free (expr);
            return STATUS_ERR;
        }
        expr->sinit_max_init_timeo = new PacketDrillExpression(in->getInitmsg()->sinit_max_init_timeo->getType());
        if (evaluate(in->getInitmsg()->sinit_max_init_timeo, expr->sinit_max_init_timeo, error)) {
            delete (expr->sinit_num_ostreams);
            delete (expr->sinit_max_instreams);
            delete (expr->sinit_max_attempts);
            delete (expr->sinit_max_init_timeo);
            free (expr);
            return STATUS_ERR;
        }
        out->setInitmsg(expr);
        break;
    }

    case EXPR_SCTP_ASSOCPARAMS: {
        assert(in->getType() == EXPR_SCTP_ASSOCPARAMS);
        assert(out->getType() == EXPR_SCTP_ASSOCPARAMS);
        struct sctp_assocparams_expr *ap = (struct sctp_assocparams_expr *)malloc(sizeof(struct sctp_assocparams_expr));
        ap->sasoc_assoc_id = new PacketDrillExpression(in->getAssocParams()->sasoc_assoc_id->getType());
        if (evaluate(in->getAssocParams()->sasoc_assoc_id, ap->sasoc_assoc_id, error)) {
            delete (ap->sasoc_assoc_id);
            free (ap);
            return STATUS_ERR;
        }
        ap->sasoc_asocmaxrxt = new PacketDrillExpression(in->getAssocParams()->sasoc_asocmaxrxt->getType());
        if (evaluate(in->getAssocParams()->sasoc_asocmaxrxt, ap->sasoc_asocmaxrxt, error)) {
            delete (ap->sasoc_assoc_id);
            delete (ap->sasoc_asocmaxrxt);
            free (ap);
            return STATUS_ERR;
        }
        ap->sasoc_number_peer_destinations = new PacketDrillExpression(in->getAssocParams()->sasoc_number_peer_destinations->getType());
        if (evaluate(in->getAssocParams()->sasoc_number_peer_destinations, ap->sasoc_number_peer_destinations, error)) {
            delete (ap->sasoc_assoc_id);
            delete (ap->sasoc_asocmaxrxt);
            delete (ap->sasoc_number_peer_destinations);
            free (ap);
            return STATUS_ERR;
        }
        ap->sasoc_peer_rwnd = new PacketDrillExpression(in->getAssocParams()->sasoc_peer_rwnd->getType());
        if (evaluate(in->getAssocParams()->sasoc_peer_rwnd, ap->sasoc_peer_rwnd, error)) {
            delete (ap->sasoc_assoc_id);
            delete (ap->sasoc_asocmaxrxt);
            delete (ap->sasoc_number_peer_destinations);
            delete (ap->sasoc_peer_rwnd);
            free (ap);
            return STATUS_ERR;
        }
        ap->sasoc_local_rwnd = new PacketDrillExpression(in->getAssocParams()->sasoc_local_rwnd->getType());
        if (evaluate(in->getAssocParams()->sasoc_local_rwnd, ap->sasoc_local_rwnd, error)) {
            delete (ap->sasoc_assoc_id);
            delete (ap->sasoc_asocmaxrxt);
            delete (ap->sasoc_number_peer_destinations);
            delete (ap->sasoc_peer_rwnd);
            delete (ap->sasoc_local_rwnd);
            free (ap);
            return STATUS_ERR;
        }
        ap->sasoc_cookie_life = new PacketDrillExpression(in->getAssocParams()->sasoc_cookie_life->getType());
        if (evaluate(in->getAssocParams()->sasoc_cookie_life, ap->sasoc_cookie_life, error)) {
            delete (ap->sasoc_assoc_id);
            delete (ap->sasoc_asocmaxrxt);
            delete (ap->sasoc_number_peer_destinations);
            delete (ap->sasoc_peer_rwnd);
            delete (ap->sasoc_local_rwnd);
            delete (ap->sasoc_cookie_life);
            free (ap);
            return STATUS_ERR;
        }
        out->setAssocParams(ap);
        break;
    }

    case EXPR_SCTP_RTOINFO: {
        assert(in->getType() == EXPR_SCTP_RTOINFO);
        assert(out->getType() == EXPR_SCTP_RTOINFO);
        struct sctp_rtoinfo_expr *info = (struct sctp_rtoinfo_expr *)malloc(sizeof(struct sctp_rtoinfo_expr));
        info->srto_assoc_id = new PacketDrillExpression(in->getRtoinfo()->srto_assoc_id->getType());
        if (evaluate(in->getRtoinfo()->srto_assoc_id, info->srto_assoc_id, error)) {
            delete (info->srto_assoc_id);
            free (info);
            return STATUS_ERR;
        }
        info->srto_initial = new PacketDrillExpression(in->getRtoinfo()->srto_initial->getType());
        if (evaluate(in->getRtoinfo()->srto_initial, info->srto_initial, error)) {
            delete (info->srto_assoc_id);
            delete (info->srto_initial);
            free (info);
            return STATUS_ERR;
        }
        info->srto_max = new PacketDrillExpression(in->getRtoinfo()->srto_max->getType());
        if (evaluate(in->getRtoinfo()->srto_max, info->srto_max, error)) {
            delete (info->srto_assoc_id);
            delete (info->srto_initial);
            delete (info->srto_max);
            free (info);
            return STATUS_ERR;
        }
        info->srto_min = new PacketDrillExpression(in->getRtoinfo()->srto_min->getType());
        if (evaluate(in->getRtoinfo()->srto_min, info->srto_min, error)) {
            delete (info->srto_assoc_id);
            delete (info->srto_initial);
            delete (info->srto_max);
            delete (info->srto_min);
            free (info);
            return STATUS_ERR;
        }
        out->setRtoinfo(info);
        break;
    }

    case EXPR_SCTP_SACKINFO: {
        assert(in->getType() == EXPR_SCTP_SACKINFO);
        assert(out->getType() == EXPR_SCTP_SACKINFO);
        struct sctp_sack_info_expr *si = (struct sctp_sack_info_expr *)malloc(sizeof(struct sctp_sack_info_expr));
        si->sack_assoc_id = new PacketDrillExpression(in->getSackinfo()->sack_assoc_id->getType());
        if (evaluate(in->getSackinfo()->sack_assoc_id, si->sack_assoc_id, error)) {
            delete (si->sack_assoc_id);
            free (si);
            return STATUS_ERR;
        }
        si->sack_delay = new PacketDrillExpression(in->getSackinfo()->sack_delay->getType());
        if (evaluate(in->getSackinfo()->sack_delay, si->sack_delay, error)) {
            delete (si->sack_assoc_id);
            delete (si->sack_delay);
            free (si);
            return STATUS_ERR;
        }
        si->sack_freq = new PacketDrillExpression(in->getSackinfo()->sack_freq->getType());
        if (evaluate(in->getSackinfo()->sack_freq, si->sack_freq, error)) {
            delete (si->sack_assoc_id);
            delete (si->sack_delay);
            delete (si->sack_freq);
            free (si);
            return STATUS_ERR;
        }
        out->setSackinfo(si);
        break;
    }

    case EXPR_SCTP_STATUS: {
        assert(in->getType() == EXPR_SCTP_STATUS);
        assert(out->getType() == EXPR_SCTP_STATUS);
        struct sctp_status_expr *st = (struct sctp_status_expr *)malloc(sizeof(struct sctp_status_expr));
        st->sstat_assoc_id = new PacketDrillExpression(in->getStatus()->sstat_assoc_id->getType());
        if (evaluate(in->getStatus()->sstat_assoc_id, st->sstat_assoc_id, error)) {
            delete (st->sstat_assoc_id);
            free (st);
            return STATUS_ERR;
        }
        st->sstat_state = new PacketDrillExpression(in->getStatus()->sstat_state->getType());
        if (evaluate(in->getStatus()->sstat_state, st->sstat_state, error)) {
            delete (st->sstat_assoc_id);
            delete (st->sstat_state);
            free (st);
            return STATUS_ERR;
        }
        st->sstat_rwnd = new PacketDrillExpression(in->getStatus()->sstat_rwnd->getType());
        if (evaluate(in->getStatus()->sstat_rwnd, st->sstat_rwnd, error)) {
            delete (st->sstat_assoc_id);
            delete (st->sstat_state);
            delete (st->sstat_rwnd);
            free (st);
            return STATUS_ERR;
        }
        st->sstat_unackdata = new PacketDrillExpression(in->getStatus()->sstat_unackdata->getType());
        if (evaluate(in->getStatus()->sstat_unackdata, st->sstat_unackdata, error)) {
            delete (st->sstat_assoc_id);
            delete (st->sstat_state);
            delete (st->sstat_rwnd);
            delete (st->sstat_unackdata);
            free (st);
            return STATUS_ERR;
        }
        st->sstat_penddata = new PacketDrillExpression(in->getStatus()->sstat_penddata->getType());
        if (evaluate(in->getStatus()->sstat_penddata, st->sstat_penddata, error)) {
            delete (st->sstat_assoc_id);
            delete (st->sstat_state);
            delete (st->sstat_rwnd);
            delete (st->sstat_unackdata);
            delete (st->sstat_penddata);
            free (st);
            return STATUS_ERR;
        }
        st->sstat_instrms = new PacketDrillExpression(in->getStatus()->sstat_instrms->getType());
        if (evaluate(in->getStatus()->sstat_instrms, st->sstat_instrms, error)) {
            delete (st->sstat_assoc_id);
            delete (st->sstat_state);
            delete (st->sstat_rwnd);
            delete (st->sstat_unackdata);
            delete (st->sstat_penddata);
            delete (st->sstat_instrms);
            free (st);
            return STATUS_ERR;
        }
        st->sstat_outstrms = new PacketDrillExpression(in->getStatus()->sstat_outstrms->getType());
        if (evaluate(in->getStatus()->sstat_outstrms, st->sstat_outstrms, error)) {
            delete (st->sstat_assoc_id);
            delete (st->sstat_state);
            delete (st->sstat_rwnd);
            delete (st->sstat_unackdata);
            delete (st->sstat_penddata);
            delete (st->sstat_instrms);
            delete (st->sstat_outstrms);
            free (st);
            return STATUS_ERR;
        }
        st->sstat_fragmentation_point = new PacketDrillExpression(in->getStatus()->sstat_fragmentation_point->getType());
        if (evaluate(in->getStatus()->sstat_fragmentation_point, st->sstat_fragmentation_point, error)) {
            delete (st->sstat_assoc_id);
            delete (st->sstat_state);
            delete (st->sstat_rwnd);
            delete (st->sstat_unackdata);
            delete (st->sstat_penddata);
            delete (st->sstat_instrms);
            delete (st->sstat_outstrms);
            delete (st->sstat_fragmentation_point);
            free (st);
            return STATUS_ERR;
        }
        st->sstat_primary = new PacketDrillExpression(in->getStatus()->sstat_primary->getType());
        if (evaluate(in->getStatus()->sstat_primary, st->sstat_primary, error)) {
            delete (st->sstat_assoc_id);
            delete (st->sstat_state);
            delete (st->sstat_rwnd);
            delete (st->sstat_unackdata);
            delete (st->sstat_penddata);
            delete (st->sstat_instrms);
            delete (st->sstat_outstrms);
            delete (st->sstat_fragmentation_point);
            delete (st->sstat_primary);
            free (st);
            return STATUS_ERR;
        }
        out->setStatus(st);
        break;
    }

    case EXPR_SCTP_ASSOCVAL: {
        assert(in->getType() == EXPR_SCTP_ASSOCVAL);
        assert(out->getType() == EXPR_SCTP_ASSOCVAL);
        struct sctp_assoc_value_expr *ass = (struct sctp_assoc_value_expr *)malloc(sizeof(struct sctp_assoc_value_expr));
        ass->assoc_id = new PacketDrillExpression(in->getAssocval()->assoc_id->getType());
        if (evaluate(in->getAssocval()->assoc_id, ass->assoc_id, error)) {
            delete (ass->assoc_id);
            free (ass);
            return STATUS_ERR;
        }
        ass->assoc_value = new PacketDrillExpression(in->getAssocval()->assoc_value->getType());
        if (evaluate(in->getAssocval()->assoc_value, ass->assoc_value, error)) {
            delete (ass->assoc_id);
            delete (ass->assoc_value);
            free (ass);
            return STATUS_ERR;
        }
        out->setAssocval(ass);
        break;
    }

    case EXPR_SCTP_SNDRCVINFO: {
        assert(in->getType() == EXPR_SCTP_SNDRCVINFO);
        assert(out->getType() == EXPR_SCTP_SNDRCVINFO);
        struct sctp_sndrcvinfo_expr *sri = (struct sctp_sndrcvinfo_expr *)malloc(sizeof(struct sctp_sndrcvinfo_expr));
        sri->sinfo_stream = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_stream->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_stream, sri->sinfo_stream, error)) {
            delete (sri->sinfo_stream);
            free (sri);
            return STATUS_ERR;
        }
        sri->sinfo_ssn = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_ssn->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_ssn, sri->sinfo_ssn, error)) {
            delete (sri->sinfo_stream);
            delete (sri->sinfo_ssn);
            free (sri);
            return STATUS_ERR;
        }
        sri->sinfo_flags = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_flags->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_flags, sri->sinfo_flags, error)) {
            delete (sri->sinfo_stream);
            delete (sri->sinfo_ssn);
            delete (sri->sinfo_flags);
            free (sri);
            return STATUS_ERR;
        }
        sri->sinfo_ppid = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_ppid->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_ppid, sri->sinfo_ppid, error)) {
            delete (sri->sinfo_stream);
            delete (sri->sinfo_ssn);
            delete (sri->sinfo_flags);
            delete (sri->sinfo_ppid);
            free (sri);
            return STATUS_ERR;
        }
        sri->sinfo_context = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_context->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_context, sri->sinfo_context, error)) {
            delete (sri->sinfo_stream);
            delete (sri->sinfo_ssn);
            delete (sri->sinfo_flags);
            delete (sri->sinfo_ppid);
            delete (sri->sinfo_context);
            free (sri);
            return STATUS_ERR;
        }
        sri->sinfo_timetolive = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_timetolive->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_timetolive, sri->sinfo_timetolive, error)) {
            delete (sri->sinfo_stream);
            delete (sri->sinfo_ssn);
            delete (sri->sinfo_flags);
            delete (sri->sinfo_ppid);
            delete (sri->sinfo_context);
            delete (sri->sinfo_timetolive);
            free (sri);
            return STATUS_ERR;
        }
        sri->sinfo_tsn = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_tsn->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_tsn, sri->sinfo_tsn, error)) {
            delete (sri->sinfo_stream);
            delete (sri->sinfo_ssn);
            delete (sri->sinfo_flags);
            delete (sri->sinfo_ppid);
            delete (sri->sinfo_context);
            delete (sri->sinfo_timetolive);
            delete (sri->sinfo_tsn);
            free (sri);
            return STATUS_ERR;
        }
        sri->sinfo_cumtsn = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_cumtsn->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_cumtsn, sri->sinfo_cumtsn, error)) {
            delete (sri->sinfo_stream);
            delete (sri->sinfo_ssn);
            delete (sri->sinfo_flags);
            delete (sri->sinfo_ppid);
            delete (sri->sinfo_context);
            delete (sri->sinfo_timetolive);
            delete (sri->sinfo_tsn);
            delete (sri->sinfo_cumtsn);
            free (sri);
            return STATUS_ERR;
        }
        sri->sinfo_assoc_id = new PacketDrillExpression(in->getSndRcvInfo()->sinfo_assoc_id->getType());
        if (evaluate(in->getSndRcvInfo()->sinfo_assoc_id, sri->sinfo_assoc_id, error)) {
            delete (sri->sinfo_stream);
            delete (sri->sinfo_ssn);
            delete (sri->sinfo_flags);
            delete (sri->sinfo_ppid);
            delete (sri->sinfo_context);
            delete (sri->sinfo_timetolive);
            delete (sri->sinfo_tsn);
            delete (sri->sinfo_cumtsn);
            delete (sri->sinfo_assoc_id);
            free (sri);
            return STATUS_ERR;
        }
        out->setSndRcvInfo(sri);
        break;
    }

    case EXPR_SCTP_RESET_STREAMS: {
        assert(in->getType() == EXPR_SCTP_RESET_STREAMS);
        assert(out->getType() == EXPR_SCTP_RESET_STREAMS);
        struct sctp_reset_streams_expr *rs = (struct sctp_reset_streams_expr *) malloc(sizeof(struct sctp_reset_streams_expr));
        rs->srs_assoc_id = new PacketDrillExpression(in->getResetStreams()->srs_assoc_id->getType());
        if (evaluate(in->getResetStreams()->srs_assoc_id, rs->srs_assoc_id, error)) {
            delete (rs->srs_assoc_id);
            free (rs);
            return STATUS_ERR;
        }
        rs->srs_flags = new PacketDrillExpression(in->getResetStreams()->srs_flags->getType());
        if (evaluate(in->getResetStreams()->srs_flags, rs->srs_flags, error)) {
            delete (rs->srs_assoc_id);
            delete (rs->srs_flags);
            free (rs);
            return STATUS_ERR;
        }
        rs->srs_number_streams = new PacketDrillExpression(in->getResetStreams()->srs_number_streams->getType());
        if (evaluate(in->getResetStreams()->srs_number_streams, rs->srs_number_streams, error)) {
            delete (rs->srs_assoc_id);
            delete (rs->srs_flags);
            delete (rs->srs_number_streams);
            free (rs);
            return STATUS_ERR;
        }
        if (rs->srs_number_streams->getNum() > 0) {
            rs->srs_stream_list = new PacketDrillExpression(in->getResetStreams()->srs_stream_list->getType());
            if (evaluate(in->getResetStreams()->srs_stream_list, rs->srs_stream_list, error)) {
                EV_WARN << "Error in evaluate\n";
                delete (rs->srs_assoc_id);
                delete (rs->srs_flags);
                delete (rs->srs_number_streams);
                delete (rs->srs_stream_list);
                free (rs);
                return STATUS_ERR;
            }
            delete in->getResetStreams()->srs_stream_list->getList();
        } else {
            rs->srs_stream_list = nullptr;
        }
        out->setResetStreams(rs);
        break;
    }

    case EXPR_SCTP_ADD_STREAMS: {
        assert(in->getType() == EXPR_SCTP_ADD_STREAMS);
        assert(out->getType() == EXPR_SCTP_ADD_STREAMS);
        printf("evaluate EXPR_SCTP_ADD_STREAMS\n");
        struct sctp_add_streams_expr *as = (struct sctp_add_streams_expr *) malloc(sizeof(struct sctp_add_streams_expr));
        as->sas_assoc_id = new PacketDrillExpression(in->getAddStreams()->sas_assoc_id->getType());
        if (evaluate(in->getAddStreams()->sas_assoc_id, as->sas_assoc_id, error)) {
        printf("add streams assoc id=%" PRId64 "\n", as->sas_assoc_id->getNum());
            delete (as->sas_assoc_id);
            free (as);
            return STATUS_ERR;
        }
        as->sas_instrms = new PacketDrillExpression(in->getAddStreams()->sas_instrms->getType());
        if (evaluate(in->getAddStreams()->sas_instrms, as->sas_instrms, error)) {
            delete (as->sas_assoc_id);
            delete (as->sas_instrms);
            free (as);
            return STATUS_ERR;
        }
        as->sas_outstrms = new PacketDrillExpression(in->getAddStreams()->sas_outstrms->getType());
        if (evaluate(in->getAddStreams()->sas_outstrms, as->sas_outstrms, error)) {
            delete (as->sas_assoc_id);
            delete (as->sas_instrms);
            delete (as->sas_outstrms);
            free (as);
            return STATUS_ERR;
        }
        out->setAddStreams(as);
        break;
    }

    case EXPR_SCTP_PEER_ADDR_PARAMS: {
        assert(in->getType() == EXPR_SCTP_PEER_ADDR_PARAMS);
        assert(out->getType() == EXPR_SCTP_PEER_ADDR_PARAMS);
        struct sctp_paddrparams_expr *spp = (struct sctp_paddrparams_expr *) malloc(sizeof(struct sctp_paddrparams_expr));
        spp->spp_assoc_id = new PacketDrillExpression(in->getPaddrParams()->spp_assoc_id->getType());
        if (evaluate(in->getPaddrParams()->spp_assoc_id, spp->spp_assoc_id, error)) {
            delete (spp->spp_assoc_id);
            free (spp);
            return STATUS_ERR;
        }
        spp->spp_address = new PacketDrillExpression(in->getPaddrParams()->spp_address->getType());
        if (evaluate(in->getPaddrParams()->spp_address, spp->spp_address, error)) {
            delete (spp->spp_assoc_id);
            delete (spp->spp_address);
            free (spp);
            return STATUS_ERR;
        }
        spp->spp_hbinterval = new PacketDrillExpression(in->getPaddrParams()->spp_hbinterval->getType());
        if (evaluate(in->getPaddrParams()->spp_hbinterval, spp->spp_hbinterval, error)) {
            delete (spp->spp_assoc_id);
            delete (spp->spp_address);
            delete (spp->spp_hbinterval);
            free (spp);
            return STATUS_ERR;
        }
        spp->spp_pathmaxrxt = new PacketDrillExpression(in->getPaddrParams()->spp_pathmaxrxt->getType());
        if (evaluate(in->getPaddrParams()->spp_pathmaxrxt, spp->spp_pathmaxrxt, error)) {
            delete (spp->spp_assoc_id);
            delete (spp->spp_address);
            delete (spp->spp_hbinterval);
            delete (spp->spp_pathmaxrxt);
            free (spp);
            return STATUS_ERR;
        }
        spp->spp_pathmtu = new PacketDrillExpression(in->getPaddrParams()->spp_pathmtu->getType());
        if (evaluate(in->getPaddrParams()->spp_pathmtu, spp->spp_pathmtu, error)) {
            delete (spp->spp_assoc_id);
            delete (spp->spp_address);
            delete (spp->spp_hbinterval);
            delete (spp->spp_pathmaxrxt);
            delete (spp->spp_pathmtu);
            free (spp);
            return STATUS_ERR;
        }
        spp->spp_flags = new PacketDrillExpression(in->getPaddrParams()->spp_flags->getType());
        if (evaluate(in->getPaddrParams()->spp_flags, spp->spp_flags, error)) {
            delete (spp->spp_assoc_id);
            delete (spp->spp_address);
            delete (spp->spp_hbinterval);
            delete (spp->spp_pathmaxrxt);
            delete (spp->spp_pathmtu);
            delete (spp->spp_flags);
            free (spp);
            return STATUS_ERR;
        }
        spp->spp_ipv6_flowlabel = new PacketDrillExpression(in->getPaddrParams()->spp_ipv6_flowlabel->getType());
        if (evaluate(in->getPaddrParams()->spp_ipv6_flowlabel, spp->spp_ipv6_flowlabel, error)) {
            delete (spp->spp_assoc_id);
            delete (spp->spp_address);
            delete (spp->spp_hbinterval);
            delete (spp->spp_pathmaxrxt);
            delete (spp->spp_pathmtu);
            delete (spp->spp_flags);
            delete (spp->spp_ipv6_flowlabel);
            free (spp);
            return STATUS_ERR;
        }
        spp->spp_dscp = new PacketDrillExpression(in->getPaddrParams()->spp_dscp->getType());
        if (evaluate(in->getPaddrParams()->spp_dscp, spp->spp_dscp, error)) {
            delete (spp->spp_assoc_id);
            delete (spp->spp_address);
            delete (spp->spp_hbinterval);
            delete (spp->spp_pathmaxrxt);
            delete (spp->spp_pathmtu);
            delete (spp->spp_flags);
            delete (spp->spp_ipv6_flowlabel);
            delete (spp->spp_dscp);
            free (spp);
            return STATUS_ERR;
        }
        out->setPaddrParams(spp);
        break;
    }

    case EXPR_STRING:
        if (out->unescapeCstringExpression(in->getString(), error))
            return STATUS_ERR;
        break;

    case EXPR_BINARY:
        if (evaluate_binary_expression(in, out, error)) {
            printf("Error in EXPR_BINARY\n");
        }
        break;

    case EXPR_LIST:
        if (evaluateListExpression(in, out, error)) {
            printf("Error in EXPR_LIST\n");
        }
        break;

    case EXPR_SOCKET_ADDRESS_IPV4:
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
        PacketDrillExpression *outExpr = new PacketDrillExpression(check_and_cast<PacketDrillExpression *>(*it)->getType());
        if (evaluate(check_and_cast<PacketDrillExpression *>(*it), outExpr, error)) {
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

    PacketDrillExpression *lhs = new PacketDrillExpression(in->getBinary()->lhs->getType());
    PacketDrillExpression *rhs = new PacketDrillExpression(in->getBinary()->rhs->getType());
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

} // namespace inet

