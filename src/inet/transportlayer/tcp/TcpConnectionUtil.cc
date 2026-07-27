//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2011 Thomas Reschka
// Copyright (C) 2011 OpenSim Ltd.
// Copyright (C) 2015 Martin Becke
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <string.h>

#include <algorithm> // min,max

#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/IcmpType_m.h"
#include "inet/networklayer/common/Icmpv4ErrorTag_m.h"
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/networklayer/common/Icmpv6Type_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

//
// helper functions
//

const char *TcpConnection::stateName(int state)
{
#define CASE(x)    case x: \
        s = (char *)#x + 6; break
    const char *s = "unknown";
    switch (state) {
        CASE(TCP_S_INIT);
        CASE(TCP_S_CLOSED);
        CASE(TCP_S_LISTEN);
        CASE(TCP_S_SYN_SENT);
        CASE(TCP_S_SYN_RCVD);
        CASE(TCP_S_ESTABLISHED);
        CASE(TCP_S_CLOSE_WAIT);
        CASE(TCP_S_LAST_ACK);
        CASE(TCP_S_FIN_WAIT_1);
        CASE(TCP_S_FIN_WAIT_2);
        CASE(TCP_S_CLOSING);
        CASE(TCP_S_TIME_WAIT);
    }
    return s;
#undef CASE
}

const char *TcpConnection::eventName(int event)
{
#define CASE(x)    case x: \
        s = (char *)#x + 6; break
    const char *s = "unknown";
    switch (event) {
        CASE(TCP_E_IGNORE);
        CASE(TCP_E_OPEN_ACTIVE);
        CASE(TCP_E_OPEN_PASSIVE);
        CASE(TCP_E_ACCEPT);
        CASE(TCP_E_SEND);
        CASE(TCP_E_CLOSE);
        CASE(TCP_E_ABORT);
        CASE(TCP_E_DESTROY);
        CASE(TCP_E_STATUS);
        CASE(TCP_E_QUEUE_BYTES_LIMIT);
        CASE(TCP_E_READ);
        CASE(TCP_E_SETOPTION);
        CASE(TCP_E_RCV_DATA);
        CASE(TCP_E_RCV_ACK);
        CASE(TCP_E_RCV_SYN);
        CASE(TCP_E_RCV_SYN_ACK);
        CASE(TCP_E_RCV_FIN);
        CASE(TCP_E_RCV_FIN_ACK);
        CASE(TCP_E_RCV_RST);
        CASE(TCP_E_RCV_UNEXP_SYN);
        CASE(TCP_E_TIMEOUT_2MSL);
        CASE(TCP_E_TIMEOUT_CONN_ESTAB);
        CASE(TCP_E_TIMEOUT_FIN_WAIT_2);
    }
    return s;
#undef CASE
}

const char *TcpConnection::indicationName(int code)
{
#define CASE(x)    case x: \
        s = (char *)#x + 6; break
    const char *s = "unknown";
    switch (code) {
        CASE(TCP_I_DATA);
        CASE(TCP_I_URGENT_DATA);
        CASE(TCP_I_AVAILABLE);
        CASE(TCP_I_ESTABLISHED);
        CASE(TCP_I_PEER_CLOSED);
        CASE(TCP_I_CLOSED);
        CASE(TCP_I_CONNECTION_REFUSED);
        CASE(TCP_I_CONNECTION_RESET);
        CASE(TCP_I_TIMED_OUT);
        CASE(TCP_I_STATUS);
        CASE(TCP_I_SEND_MSG);
        CASE(TCP_I_ICMPv4_ERROR);
        CASE(TCP_I_ICMPv6_ERROR);
    }
    return s;
#undef CASE
}

const char *TcpConnection::optionName(int option)
{
    switch (option) {
        case TCPOPTION_END_OF_OPTION_LIST:
            return "EOL";

        case TCPOPTION_NO_OPERATION:
            return "NOP";

        case TCPOPTION_MAXIMUM_SEGMENT_SIZE:
            return "MSS";

        case TCPOPTION_WINDOW_SCALE:
            return "WS";

        case TCPOPTION_SACK_PERMITTED:
            return "SACK_PERMITTED";

        case TCPOPTION_SACK:
            return "SACK";

        case TCPOPTION_TIMESTAMP:
            return "TS";

        default:
            return "unknown";
    }
}

void TcpConnection::printConnBrief() const
{
    EV_DETAIL << "Connection "
              << localAddr << ":" << localPort << " to " << remoteAddr << ":" << remotePort
              << "  on socketId=" << socketId
              << "  in " << stateName(fsm.getState())
              << "\n";
}

void TcpConnection::printSegmentBrief(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader)
{
    EV_STATICCONTEXT;
    EV_INFO << "." << tcpHeader->getSrcPort() << " > ";
    EV_INFO << "." << tcpHeader->getDestPort() << ": ";

    if (tcpHeader->getSynBit())
        EV_INFO << (tcpHeader->getAckBit() ? "SYN+ACK " : "SYN ");

    if (tcpHeader->getFinBit())
        EV_INFO << "FIN(+ACK) ";

    if (tcpHeader->getRstBit())
        EV_INFO << (tcpHeader->getAckBit() ? "RST+ACK " : "RST ");

    if (tcpHeader->getPshBit())
        EV_INFO << "PSH ";

    auto payloadLength = tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>();
    if (payloadLength > 0 || tcpHeader->getSynBit()) {
        EV_INFO << "[" << tcpHeader->getSequenceNo() << ".." << (tcpHeader->getSequenceNo() + payloadLength) << ") ";
        EV_INFO << "(l=" << payloadLength << ") ";
    }

    if (tcpHeader->getAckBit())
        EV_INFO << "ack " << tcpHeader->getAckNo() << " ";

    EV_INFO << "win " << tcpHeader->getWindow() << " ";

    if (tcpHeader->getUrgBit())
        EV_INFO << "urg " << tcpHeader->getUrgentPointer() << " ";

    if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH) { // Header options present?
        EV_INFO << "options ";

        for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
            const TcpOption *option = tcpHeader->getHeaderOption(i);
            short kind = option->getKind();
            EV_INFO << optionName(kind) << " ";
        }
    }
    EV_INFO << "\n";
}

void TcpConnection::initClonedConnection(TcpConnection *listenerConn)
{
    Enter_Method("initClonedConnection");
    listeningSocketId = listenerConn->getSocketId();

    // following code to be kept consistent with initConnection()
    const char *sendQueueClass = listenerConn->sendQueue->getClassName();
    sendQueue = check_and_cast<TcpSendQueue *>(inet::utils::createOne(sendQueueClass));
    sendQueue->setConnection(this);

    const char *receiveQueueClass = listenerConn->receiveQueue->getClassName();
    receiveQueue = check_and_cast<TcpReceiveQueue *>(inet::utils::createOne(receiveQueueClass));
    receiveQueue->setConnection(this);

    // create SACK retransmit queue
    rexmitQueue = new TcpSackRexmitQueue();
    rexmitQueue->setConnection(this);

    const char *tcpAlgorithmClass = listenerConn->tcpAlgorithm->getClassName();
    tcpAlgorithm = check_and_cast<TcpAlgorithm *>(inet::utils::createOne(tcpAlgorithmClass));
    tcpAlgorithm->setConnection(this);

    state = tcpAlgorithm->getStateVariables();
    configureStateVariables();
    tcpAlgorithm->initialize();

    // put it into LISTEN, with our localAddr/localPort
    state->active = false;
    state->fork = true;
    localAddr = listenerConn->localAddr;
    localPort = listenerConn->localPort;
    autoRead = listenerConn->autoRead;

    FSM_Goto(fsm, TCP_S_LISTEN);
}

TcpConnection *TcpConnection::cloneListeningConnection()
{
    auto moduleType = cModuleType::get("inet.transportlayer.tcp.TcpConnection");
    int newSocketId = getActiveSimulationOrEnvir()->getUniqueNumber();
    char submoduleName[24];
    snprintf(submoduleName, sizeof(submoduleName), "conn-%d", newSocketId);
    auto conn = check_and_cast<TcpConnection *>(moduleType->createScheduleInit(submoduleName, tcpMain));
    conn->initConnection(tcpMain, newSocketId);
    conn->initClonedConnection(this);
    return conn;
}

void TcpConnection::sendToIP(Packet *tcpSegment, const Ptr<TcpHeader>& tcpHeader)
{
    sentSegments++;
    lastSentAck = tcpHeader->getAckNo();

    // record seq (only if we do send data) and ackno
    if (tcpSegment->getByteLength() > tcpHeader->getChunkLength().get<B>())
        emit(sndNxtSignal, tcpHeader->getSequenceNo());

    emit(sndAckSignal, tcpHeader->getAckNo());

    // final touches on the segment before sending
    tcpHeader->setSrcPort(localPort);
    tcpHeader->setDestPort(remotePort);
    ASSERT(tcpHeader->getHeaderLength() >= TCP_MIN_HEADER_LENGTH);
    ASSERT(tcpHeader->getHeaderLength() <= TCP_MAX_HEADER_LENGTH);
    ASSERT(tcpHeader->getChunkLength() == tcpHeader->getHeaderLength());

    EV_INFO << "Sending: ";
    printSegmentBrief(tcpSegment, tcpHeader);

    // TODO reuse next function for sending

    const IL3AddressType *addressType = remoteAddr.getAddressType();
    tcpSegment->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());

    if (ttl != -1 && tcpSegment->findTag<HopLimitReq>() == nullptr)
        tcpSegment->addTag<HopLimitReq>()->setHopLimit(ttl);

    if (dscp != -1 && tcpSegment->findTag<DscpReq>() == nullptr)
        tcpSegment->addTag<DscpReq>()->setDifferentiatedServicesCodePoint(dscp);

    if (tos != -1 && tcpSegment->findTag<TosReq>() == nullptr)
        tcpSegment->addTag<TosReq>()->setTos(tos);

    // PMTUD (RFC 1191): set Don't Fragment bit on IPv4 segments
    if (state->pmtudEnabled && remoteAddr.getType() == L3Address::IPv4)
        tcpSegment->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    auto addresses = tcpSegment->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(localAddr);
    addresses->setDestAddress(remoteAddr);

    // ECN:
    // We decided to use ECT(1) to indicate ECN capable transport.
    //
    // rfc-3168, page 6:
    // Routers treat the ECT(0) and ECT(1) codepoints
    // as equivalent.  Senders are free to use either the ECT(0) or the
    // ECT(1) codepoint to indicate ECT.
    //
    // rfc-3168, page 20:
    // For the current generation of TCP congestion control algorithms, pure
    // acknowledgement packets (e.g., packets that do not contain any
    // accompanying data) MUST be sent with the not-ECT codepoint.
    //
    // rfc-3168, page 20:
    // ECN-capable TCP implementations MUST NOT set either ECT codepoint
    // (ECT(0) or ECT(1)) in the IP header for retransmitted data packets
    tcpSegment->addTagIfAbsent<EcnReq>()->setExplicitCongestionNotification((state->ect && !state->sndAck && !state->rexmit) ? IP_ECN_ECT_1 : IP_ECN_NOT_ECT);

    tcpHeader->setChecksum(0);
    tcpHeader->setChecksumMode(tcpMain->checksumMode);

    insertTransportProtocolHeader(tcpSegment, Protocol::tcp, tcpHeader);

    tcpMain->sendToIp(tcpSegment);
}

void TcpConnection::sendToIP(Packet *tcpSegment, const Ptr<TcpHeader>& tcpHeader, L3Address src, L3Address dest)
{
    EV_STATICCONTEXT;
    EV_INFO << "Sending: ";
    printSegmentBrief(tcpSegment, tcpHeader);

    const IL3AddressType *addressType = dest.getAddressType();
    ASSERT(tcpHeader->getChunkLength() == tcpHeader->getHeaderLength());
    tcpSegment->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());

    if (ttl != -1 && tcpSegment->findTag<HopLimitReq>() == nullptr)
        tcpSegment->addTag<HopLimitReq>()->setHopLimit(ttl);

    if (dscp != -1 && tcpSegment->findTag<DscpReq>() == nullptr)
        tcpSegment->addTag<DscpReq>()->setDifferentiatedServicesCodePoint(dscp);

    if (tos != -1 && tcpSegment->findTag<TosReq>() == nullptr)
        tcpSegment->addTag<TosReq>()->setTos(tos);

    auto addresses = tcpSegment->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(src);
    addresses->setDestAddress(dest);

    insertTransportProtocolHeader(tcpSegment, Protocol::tcp, tcpHeader);

    tcpMain->sendToIp(tcpSegment);
}

void TcpConnection::signalConnectionTimeout()
{
    sendIndicationToApp(TCP_I_TIMED_OUT);
}

void TcpConnection::sendIndicationToApp(int code, const int id)
{
    EV_INFO << "Notifying app: " << indicationName(code) << "\n";
    auto indication = new Indication(indicationName(code), code);
    TcpCommand *ind = new TcpCommand();
    ind->setUserId(id);
    indication->addTag<SocketInd>()->setSocketId(socketId);
    indication->setControlInfo(ind);
    sendToApp(indication);
}

bool TcpConnection::processIcmpv4Error(Indication *indication)
{
    Enter_Method("processIcmpv4Error");
    take(indication);

    auto& errorInd = indication->getTag<Icmpv4ErrorInd>();

    EV_WARN << "ICMPv4 error for connection " << localAddr << ":" << localPort
            << " > " << remoteAddr << ":" << remotePort
            << " type=" << errorInd->getType() << " code=" << errorInd->getCode() << "\n";

    // Hard errors abort the connection during setup (RFC 5461).
    // Once ESTABLISHED, even hard errors are treated as soft to prevent
    // blind reset attacks.
    if (isHardIcmpv4Error(errorInd->getType(), errorInd->getCode()) && fsm.getState() == TCP_S_SYN_SENT) {
        EV_DETAIL << "Hard ICMPv4 error during connection setup -- connection refused\n";

        sendQueue->discardUpTo(sendQueue->getBufferEndSeq());
        if (state->sack_enabled)
            rexmitQueue->discardUpTo(rexmitQueue->getBufferEndSeq());

        sendIndicationToApp(TCP_I_CONNECTION_REFUSED);
        delete indication;

        return performStateTransition(TCP_E_RCV_RST);
    }

    // PMTUD (RFC 1191): if Fragmentation Needed and DF Set, reduce snd_mss
    if (state->pmtudEnabled && isFragNeeded(errorInd->getType(), errorInd->getCode())) {
        int mtu = errorInd->getMtu();
        if (mtu > 0) {
            // 20 bytes IPv4 header + 20 bytes TCP minimum header
            uint32_t newMss = mtu - 40;
            if (newMss < state->snd_mss) {
                EV_DETAIL << "PMTUD: reducing snd_mss from " << state->snd_mss
                          << " to " << newMss << " (reported MTU=" << mtu << ")\n";
                state->snd_mss = newMss;
                state->pmtudLastMssReduction = simTime();
                retransmitOneSegment(true);
            }
        }
    }

    // Soft notification: forward the original indication to the application, no state change.
    // Don't forward if the connection is in a state where the app may not have
    // the socket registered (would cause MessageDispatcher error).
    int fsmState = fsm.getState();
    if (fsmState != TCP_S_ESTABLISHED && fsmState != TCP_S_FIN_WAIT_1 &&
        fsmState != TCP_S_FIN_WAIT_2 && fsmState != TCP_S_CLOSE_WAIT &&
        fsmState != TCP_S_CLOSING)
    {
        EV_DETAIL << "Ignoring soft ICMPv4 error in " << stateName(fsmState) << " state (app may not have socket registered)\n";
        delete indication;
        return true;
    }

    // The Icmpv4ErrorInd tag is already attached; just relabel and add SocketInd.
    indication->setName(indicationName(TCP_I_ICMPv4_ERROR));
    indication->setKind(TCP_I_ICMPv4_ERROR);
    indication->addTag<SocketInd>()->setSocketId(socketId);
    sendToApp(indication);
    return true;
}

bool TcpConnection::processIcmpv6Error(Indication *indication)
{
    Enter_Method("processIcmpv6Error");
    take(indication);

    auto& errorInd = indication->getTag<Icmpv6ErrorInd>();

    EV_WARN << "ICMPv6 error for connection " << localAddr << ":" << localPort
            << " > " << remoteAddr << ":" << remotePort
            << " type=" << errorInd->getType() << " code=" << errorInd->getCode() << "\n";

    // Hard errors abort the connection during setup (RFC 5461).
    // Once ESTABLISHED, even hard errors are treated as soft to prevent
    // blind reset attacks.
    if (isHardIcmpv6Error(errorInd->getType(), errorInd->getCode()) && fsm.getState() == TCP_S_SYN_SENT) {
        EV_DETAIL << "Hard ICMPv6 error during connection setup -- connection refused\n";

        sendQueue->discardUpTo(sendQueue->getBufferEndSeq());
        if (state->sack_enabled)
            rexmitQueue->discardUpTo(rexmitQueue->getBufferEndSeq());

        sendIndicationToApp(TCP_I_CONNECTION_REFUSED);
        delete indication;

        return performStateTransition(TCP_E_RCV_RST);
    }

    // PMTUD (RFC 1981): if Packet Too Big, reduce snd_mss
    if (state->pmtudEnabled && isPacketTooBig(errorInd->getType(), errorInd->getCode())) {
        int mtu = errorInd->getMtu();
        if (mtu > 0) {
            // 40 bytes IPv6 header + 20 bytes TCP minimum header
            uint32_t newMss = mtu - 60;
            if (newMss < state->snd_mss) {
                EV_DETAIL << "PMTUD: reducing snd_mss from " << state->snd_mss
                          << " to " << newMss << " (reported MTU=" << mtu << ")\n";
                state->snd_mss = newMss;
                state->pmtudLastMssReduction = simTime();
                retransmitOneSegment(true);
            }
        }
    }

    // Soft notification: forward the original indication to the application, no state change.
    // Don't forward if the connection is in a closing/closed state where the app
    // may have already cleaned up its socket (would cause MessageDispatcher error).
    int fsmState = fsm.getState();
    if (fsmState != TCP_S_ESTABLISHED && fsmState != TCP_S_FIN_WAIT_1 &&
        fsmState != TCP_S_FIN_WAIT_2 && fsmState != TCP_S_CLOSE_WAIT &&
        fsmState != TCP_S_CLOSING)
    {
        EV_DETAIL << "Ignoring soft ICMPv6 error in " << stateName(fsmState) << " state (app may not have socket registered)\n";
        delete indication;
        return true;
    }

    // The Icmpv6ErrorInd tag is already attached; just relabel and add SocketInd.
    indication->setName(indicationName(TCP_I_ICMPv6_ERROR));
    indication->setKind(TCP_I_ICMPv6_ERROR);
    indication->addTag<SocketInd>()->setSocketId(socketId);
    sendToApp(indication);
    return true;
}

bool TcpConnection::isHardIcmpv4Error(int type, int code)
{
    // ICMPv4 Destination Unreachable with protocol/port unreachable or admin prohibited
    return type == ICMP_DESTINATION_UNREACHABLE
           && (code == ICMP_DU_PROTOCOL_UNREACHABLE
               || code == ICMP_DU_PORT_UNREACHABLE
               || code == ICMP_DU_COMMUNICATION_PROHIBITED);
}

bool TcpConnection::isHardIcmpv6Error(int type, int code)
{
    // ICMPv6 Destination Unreachable with port unreachable or admin prohibited
    return type == ICMPv6_DESTINATION_UNREACHABLE
           && (code == PORT_UNREACHABLE
               || code == COMM_WITH_DEST_PROHIBITED);
}

bool TcpConnection::isFragNeeded(IcmpType type, int code)
{
    // ICMPv4 Destination Unreachable / Fragmentation Needed and DF Set (RFC 1191)
    return type == ICMP_DESTINATION_UNREACHABLE
           && code == ICMP_DU_FRAGMENTATION_NEEDED;
}

bool TcpConnection::isPacketTooBig(Icmpv6Type type, int code)
{
    // ICMPv6 Packet Too Big (RFC 1981)
    return type == ICMPv6_PACKET_TOO_BIG;
}

void TcpConnection::sendAvailableIndicationToApp()
{
    EV_INFO << "Notifying app: " << indicationName(TCP_I_AVAILABLE) << "\n";
    auto indication = new Indication(indicationName(TCP_I_AVAILABLE), TCP_I_AVAILABLE);
    TcpAvailableInfo *ind = new TcpAvailableInfo();
    ind->setNewSocketId(socketId);
    ind->setLocalAddr(localAddr);
    ind->setRemoteAddr(remoteAddr);
    ind->setLocalPort(localPort);
    ind->setRemotePort(remotePort);
    ind->setAutoRead(autoRead);

    indication->addTag<SocketInd>()->setSocketId(listeningSocketId);
    indication->setControlInfo(ind);
    sendToApp(indication);
}

void TcpConnection::sendEstabIndicationToApp()
{
    EV_INFO << "Notifying app: " << indicationName(TCP_I_ESTABLISHED) << "\n";
    auto indication = new Indication(indicationName(TCP_I_ESTABLISHED), TCP_I_ESTABLISHED);
    TcpConnectInfo *ind = new TcpConnectInfo();
    ind->setLocalAddr(localAddr);
    ind->setRemoteAddr(remoteAddr);
    ind->setLocalPort(localPort);
    ind->setRemotePort(remotePort);
    ind->setAutoRead(autoRead);
    indication->addTag<SocketInd>()->setSocketId(socketId);
    indication->setControlInfo(ind);
    sendToApp(indication);
}

void TcpConnection::sendToApp(cMessage *msg)
{
    tcpMain->sendToApp(msg);
}

void TcpConnection::sendAvailableDataToApp()
{
    if (receiveQueue->getAmountOfBufferedBytes()) {
        if (autoRead || maxByteCountRequested > 0) {
            uint32_t endSeqNo = state->rcv_nxt;
            if (!autoRead) {
                uint32_t requestedEndPos = receiveQueue->getFirstSeqNo() + maxByteCountRequested;
                if (seqLess(requestedEndPos, endSeqNo))
                    endSeqNo = requestedEndPos;
            }
            while (auto msg = receiveQueue->extractBytesUpTo(endSeqNo)) {
                msg->setKind(TCP_I_DATA);    // TBD currently we never send TCP_I_URGENT_DATA
                msg->addTag<SocketInd>()->setSocketId(socketId);
                sendToApp(msg);
                if (!autoRead) {
                    maxByteCountRequested = 0;
                    break;
                }
            }
        }
    }
}

void TcpConnection::initConnection(TcpOpenCommand *openCmd)
{
    // create send queue
    sendQueue = tcpMain->createSendQueue();
    sendQueue->setConnection(this);

    // create receive queue
    receiveQueue = tcpMain->createReceiveQueue();
    receiveQueue->setConnection(this);

    // create SACK retransmit queue
    rexmitQueue = new TcpSackRexmitQueue();
    rexmitQueue->setConnection(this);

    // create algorithm
    const char *tcpAlgorithmClass = openCmd->getTcpAlgorithmClass();

    if (opp_isempty(tcpAlgorithmClass))
        tcpAlgorithmClass = tcpMain->par("tcpAlgorithmClass");

    tcpAlgorithm = check_and_cast<TcpAlgorithm *>(inet::utils::createOne(tcpAlgorithmClass));
    tcpAlgorithm->setConnection(this);

    // create state block
    state = tcpAlgorithm->getStateVariables();
    configureStateVariables();
    tcpAlgorithm->initialize();
}

void TcpConnection::configureStateVariables()
{
    uint32_t advertisedWindow = tcpMain->par("advertisedWindow");
    state->ws_support = tcpMain->par("windowScalingSupport");
    int windowScalingFactor = tcpMain->par("windowScalingFactor");
    if (windowScalingFactor < -1 || windowScalingFactor > 14)
        throw cRuntimeError("Invalid parameter value windowScalingFactor=%d -- valid values are 0..14, and -1 for automatic selection based on advertisedWindow", windowScalingFactor);
    if (state->ws_support) {
        uint32_t maxAdvertisedWindow = TCP_MAX_WIN << (windowScalingFactor==-1 ? 14 : windowScalingFactor);
        if (advertisedWindow > maxAdvertisedWindow)
            throw cRuntimeError("Invalid parameter value: advertisedWindow=%" PRIu32 " exceeds representable maximum %" PRIu32 " with windowScalingFactor=%d", advertisedWindow, maxAdvertisedWindow, windowScalingFactor);
    }
    else if (advertisedWindow > TCP_MAX_WIN) {
        throw cRuntimeError("Invalid parameter value: advertisedWindow=%" PRIu32 " exceeds representable maximum %lu, try turning on window scaling (windowScalingSupport=true)", advertisedWindow, TCP_MAX_WIN);
    }
    state->ws_manual_scale = windowScalingFactor;

    state->rcv_wnd = advertisedWindow;
    state->rcv_adv = advertisedWindow;

    if (state->ws_support && advertisedWindow > TCP_MAX_WIN) {
        state->rcv_wnd = TCP_MAX_WIN; // we cannot to guarantee that the other end is also supporting the Window Scale (header option) (RFC 1122)
        state->rcv_adv = TCP_MAX_WIN; // therefore TCP_MAX_WIN is used as initial value for rcv_wnd and rcv_adv
    }

    state->maxRcvBuffer = advertisedWindow;
    // receive-buffer capacity decoupled from the advertised window (Linux
    // sk_rcvbuf vs the offered window): out-of-order data may be buffered up
    // to this limit; -1 keeps the historical conflation
    {
        int64_t rcvBufBytes = (int64_t)tcpMain->par("receiveBufferSize").doubleValue();
        state->rcvBufferSize = rcvBufBytes >= 0 ? (uint32_t)rcvBufBytes : 0;
    }
    // receiver window auto-tuning (Linux tcp_grow_window): offer starts at
    // advertisedWindow, grows with received data toward the clamp (Linux
    // tcp_win_from_space halves the buffer at the default scaling ratio)
    if (tcpMain->par("windowAutoTuning") && state->rcvBufferSize > 0) {
        state->rcv_ssthresh = advertisedWindow;
        state->window_clamp = std::max(advertisedWindow, state->rcvBufferSize / 2);
    }
    // windowShrinkAllowed: the whole window model follows the REAL buffer
    // (Linux sk_rcvbuf = receiveBufferSize), not the advertisedWindow param --
    // the initial offer is tcp_select_initial_window's win_from_space at the
    // DEFAULT 50% scaling ratio, so the maximum promise the peer can hold us
    // to (rcv_adv / Linux rcv_mwnd_seq) starts buffer-derived too
    // (rcv_wnd_shrink_allowed's max promise comes solely from the first
    // data-time offer).
    if (tcpMain->par("windowShrinkAllowed").boolValue() && state->rcvBufferSize > 0) {
        state->maxRcvBuffer = state->rcvBufferSize;
        uint32_t initWin = std::min<uint32_t>(state->rcvBufferSize / 2, TCP_MAX_WIN);
        state->rcv_wnd = initWin;
        state->rcv_adv = initWin;
        if (tcpMain->par("windowAutoTuning") && state->rcvBufferSize > 0) {
            state->rcv_ssthresh = std::max<uint32_t>(state->rcv_ssthresh, state->rcvBufferSize);
            state->window_clamp = std::max<uint32_t>(state->window_clamp, state->rcvBufferSize);
        }
    }
    state->delayed_acks_enabled = tcpMain->par("delayedAcksEnabled"); // delayed ACK algorithm (RFC 1122) enabled/disabled
    state->delayedAckFrameCount = tcpMain->par("delayedAckFrameCount");
    state->nagle_enabled = tcpMain->par("nagleEnabled"); // Nagle's algorithm (RFC 1122) enabled/disabled
    // A runtime TCP_NODELAY / TCP_CORK received before OPEN (nodelaySockopt/corkSockopt,
    // INT_MIN = never set) overrides the compiled-in defaults (mirrors userMss/notsentLowat).
    if (nodelaySockopt != INT_MIN)
        state->nagle_enabled = (nodelaySockopt == 0);
    if (corkSockopt != INT_MIN)
        state->tcp_cork = (corkSockopt != 0);
    state->adaptiveDelayedAcks = tcpMain->par("adaptiveDelayedAcks"); // Linux-shaped quickack/ATO/pingpong dynamics
    state->pushOnWriteBoundary = tcpMain->par("pushSegmentsOnWriteBoundary"); // Linux-parity PSH-on-drain
    state->limited_transmit_enabled = tcpMain->par("limitedTransmitEnabled"); // Limited Transmit algorithm (RFC 3042) enabled/disabled
    state->increased_IW_enabled = tcpMain->par("increasedIWEnabled"); // Increased Initial Window (RFC 3390) enabled/disabled
    const char *initialWindow = tcpMain->par("initialWindow");
    if (state->increased_IW_enabled) {
        // deprecated knob: map to RFC 3390 unless initialWindow was also set
        if (strcmp(initialWindow, "rfc2001") != 0)
            throw cRuntimeError("Tcp: set either the deprecated increasedIWEnabled or initialWindow, not both");
        EV_WARN << "Tcp: increasedIWEnabled is deprecated; use initialWindow=\"rfc3390\"\n";
        state->init_cwnd_mode = 1;
    }
    else if (!strcmp(initialWindow, "rfc3390"))
        state->init_cwnd_mode = 1;
    else if (!strcmp(initialWindow, "rfc6928"))
        state->init_cwnd_mode = 2;
    else
        state->init_cwnd_mode = 0;
    // Maximum Segment Size (RFC 9293). mss=-1 is a sentinel meaning "derive from the
    // address family" -- resolved in writeHeaderOptions() once remoteAddr is known.
    // Read as signed first: assigning -1 straight into the uint32_t snd_mss trips
    // OMNeT++'s cPar overflow check.
    int mssPar = tcpMain->par("mss");
    if (mssPar == -1)
        state->snd_mss = (uint32_t)-1;
    else if (mssPar >= 64 && mssPar <= 65535)
        state->snd_mss = (uint32_t)mssPar;
    else
        throw cRuntimeError("mss must be -1 (address-family default) or in the range 64..65535, but is %d", mssPar);
    state->snd_effmss = calculateEffectiveMss();
    state->ts_support = tcpMain->par("timestampSupport"); // if set, this means that current host supports TS (RFC 7323)
    state->ecnWillingness = tcpMain->par("ecnWillingness"); // if set, current host is willing to use ECN
    state->advertisedMss = state->snd_mss; // our own receive limit; stays unclamped when snd_mss later shrinks to the peer's MSS
    // TCP_MAXSEG (setsockopt SOL_TCP, via TcpSetMaxSegCommand before OPEN): the app
    // caps the MSS -- both what we advertise in our SYN/SYN-ACK and the effective
    // send MSS (Linux rx_opt.user_mss). A later peer MSS option still clamps snd_mss
    // further down (writeHeaderOptions/readHeaderOptions min), so a smaller peer MSS
    // wins, but the peer can never raise us above userMss.
    if (userMss > 0) {
        state->advertisedMss = userMss;
        if (state->snd_mss == (uint32_t)-1 || (uint32_t)userMss < state->snd_mss) {
            state->snd_mss = userMss;
            state->snd_effmss = calculateEffectiveMss();
        }
    }
    // TCP_NOTSENT_LOWAT. -1 (default) disables it; same signed-read-
    // first pattern as mss above, since -1 doesn't fit directly into the uint32_t field.
    // A runtime TcpSetNotsentLowatCommand received before OPEN (notsentLowatSockopt,
    // INT_MIN = never set) overrides the module parameter.
    int notsentLowatPar = (notsentLowatSockopt != INT_MIN) ? notsentLowatSockopt : (int)tcpMain->par("notsentLowat");
    state->notsentLowat = (notsentLowatPar < 0) ? (uint32_t)-1 : (uint32_t)notsentLowatPar;
    // ECN mode resolution (AccECN): tcpEcnMode supersedes the deprecated
    // ecnWillingness. Exact precedent: increasedIWEnabled vs initialWindow (above).
    bool ecnWillingnessDeprecated = tcpMain->par("ecnWillingness");
    const char *tcpEcnModeStr = tcpMain->par("tcpEcnMode");
    if (ecnWillingnessDeprecated) {
        if (strcmp(tcpEcnModeStr, "off") != 0)
            throw cRuntimeError("Tcp: set either the deprecated ecnWillingness or tcpEcnMode, not both");
        EV_WARN << "Tcp: ecnWillingness is deprecated; use tcpEcnMode=\"rfc3168\"\n";
        state->ecnMode = TCP_ECN_MODE_RFC3168;
    }
    else if (!strcmp(tcpEcnModeStr, "passive"))
        state->ecnMode = TCP_ECN_MODE_PASSIVE;
    else if (!strcmp(tcpEcnModeStr, "rfc3168"))
        state->ecnMode = TCP_ECN_MODE_RFC3168;
    else if (!strcmp(tcpEcnModeStr, "accecn"))
        state->ecnMode = TCP_ECN_MODE_ACCECN;
    else if (!strcmp(tcpEcnModeStr, "accecn-passive"))
        state->ecnMode = TCP_ECN_MODE_ACCECN_PASSIVE;
    else
        state->ecnMode = TCP_ECN_MODE_OFF;
    // state->ecnWillingness reflects "willing to use ECN in some capacity" (accept and/or
    // initiate) and is what the PASSIVE-open call sites (processSynInListen/sendSynAck) key
    // off, unchanged since before AccECN -- true for rfc3168 and both AccECN modes (passive
    // mode's own asymmetry, and accecn-passive's, is about *initiating*, which the ACTIVE-open
    // call site (sendSyn()) decides separately, directly from ecnMode, not from this flag).
    state->ecnWillingness = state->ecnMode >= TCP_ECN_MODE_RFC3168;
    state->accEcnOptionEnabled = tcpMain->par("accEcnOptionEnabled");
    state->accEcnOptionBeaconAcks = tcpMain->par("accEcnOptionBeaconAcks");
    state->accEcnOptionKindAlternates = tcpMain->par("accEcnOptionKindAlternates");
    state->dupthresh = tcpMain->par("dupthresh");
    state->frtoEnabled = tcpMain->par("frtoEnabled");
    state->tlpEnabled = tcpMain->par("tlpEnabled");
    state->seedRttFromHandshake = tcpMain->par("seedRttFromHandshake");
    state->adaptiveReorderingEnabled = tcpMain->par("adaptiveReorderingEnabled");
    state->maxReordering = tcpMain->par("maxReordering");
    state->reordering = state->dupthresh; // dynamic DupThresh starts at the static value
    state->lossUndoEnabled = tcpMain->par("lossUndoEnabled");
    state->prrEnabled = tcpMain->par("prrEnabled");
    state->lossDetectionMode = !strcmp(tcpMain->par("lossDetectionMode"), "rack") ? 1 : 0;
    state->sack_support = tcpMain->par("sackSupport"); // if set, this means that current host supports SACK (RFC 2018, 2883, 6675)
    // SACK-based (RFC 6675) loss recovery is provided by flavours whose createRecovery()
    // can return an Rfc6675Recovery (TcpReno, TcpNewReno). Other flavours (TcpTahoe,
    // TcpVegas, TcpWestwood, DumbTcp, ...) have no SACK recovery path. Rather than error
    // -- which would make it impossible to turn sackSupport on by default -- treat
    // sackSupport as a willingness (as Linux does; SACK is orthogonal to the congestion
    // control) and simply do not use SACK for a flavour that cannot recover with it.
    if (state->sack_support && !tcpAlgorithm->supportsSackRecovery()) {
        EV_WARN << "sackSupport=true but tcpAlgorithmClass=\"" << tcpAlgorithm->getClassName()
                << "\" has no SACK-based loss recovery; disabling SACK for this connection\n";
        state->sack_support = false;
    }
    if (state->lossDetectionMode == 1 && !state->sack_support) {
        // RACK needs the SACK scoreboard. Rather than make the connection
        // unusable, fall back to classic DupThresh -- the same "willingness"
        // treatment sackSupport itself gets just above, so that turning RACK on
        // by default cannot break a flavour or peer that ends up without SACK.
        EV_WARN << "lossDetectionMode=\"rack\" requires SACK, which is not enabled for this "
                   "connection; falling back to DupThresh loss detection\n";
        state->lossDetectionMode = 0;
    }
    state->pmtudEnabled = tcpMain->par("pmtudEnabled"); // Path MTU Discovery (RFC 1191, RFC 1981)
    state->pmtudTimeout = tcpMain->par("pmtudTimeout"); // time after which original MSS is restored
    state->pmtudLastMssReduction = -1; // never reduced yet
    state->dsack_enabled = tcpMain->par("dsackEnabled"); // if set, this means that current host supports SACK (RFC 2018, 2883, 6675)

    // TCP_INFO trio: idle/not-limited until the first SEND/sendData() call says
    // otherwise (enqueueSendCommandData()/sendData()).
    state->busyStartTime = -1;
    state->rwndLimitedStartTime = -1;
    state->sndbufLimitedStartTime = -1;

    state->fastopenClientEnabled = tcpMain->par("fastopenClientEnabled"); // TCP Fast Open (RFC 7413)
    state->fastopenServerEnabled = tcpMain->par("fastopenServerEnabled");
    state->fastopenAcceptWithoutCookie = tcpMain->par("fastopenAcceptWithoutCookie");
    state->fastopenLenientCookieValidation = tcpMain->par("fastopenLenientCookieValidation");
    state->fastopenExpOptionEnabled = tcpMain->par("fastopenExpOptionEnabled");
    int fastopenCookieBytes = tcpMain->par("fastopenCookieBytes");
    if (fastopenCookieBytes < 4 || fastopenCookieBytes > 16)
        throw cRuntimeError("fastopenCookieBytes must be in the range 4..16 (RFC 7413 SS4), but is %d", fastopenCookieBytes);
    state->fastopenCookieBytes = fastopenCookieBytes;

    WATCH_EXPR("snd_nxt", state->snd_nxt);
    WATCH_EXPR("rcv_nxt", state->rcv_nxt);
    WATCH_EXPR("snd_una", state->snd_una);

}

void TcpConnection::selectInitialSeqNum()
{
    // set the initial send sequence number
    int64_t iss = tcpMain->par("initialSendSequenceNumber");
    state->iss = iss != -1 ? iss : (unsigned long)fmod(SIMTIME_DBL(simTime()) * 250000.0, 1.0 + (double)(unsigned)0xffffffffUL) & 0xffffffffUL;

    state->snd_una = state->snd_nxt = state->snd_max = state->iss;

    sendQueue->init(state->iss + 1); // + 1 is for SYN
    rexmitQueue->init(state->iss + 1); // + 1 is for SYN
}

bool TcpConnection::isSegmentAcceptable(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader) const
{
    // check that segment entirely falls in receive window
    // RFC 793, page 69:
    // "There are four cases for the acceptability test for an incoming segment:
    //    Segment Receive  Test
    //    Length  Window
    //    ------- -------  -------------------------------------------
    //       0       0     SEG.SEQ = RCV.NXT
    //       0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
    //      >0       0     not acceptable
    //      >0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
    //                  or RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND"
    uint32_t len = tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>();
    uint32_t seqNo = tcpHeader->getSequenceNo();
    uint32_t ackNo = tcpHeader->getAckNo();
    uint32_t rcvWndEnd = state->rcv_nxt + state->rcv_wnd;
    bool ret;

    if (len == 0) {
        if (state->rcv_wnd == 0)
            ret = (seqNo == state->rcv_nxt);
        else // rcv_wnd > 0
//            ret = seqLE(state->rcv_nxt, seqNo) && seqLess(seqNo, rcvWndEnd);
            ret = seqLE(state->rcv_nxt, seqNo) && seqLE(seqNo, rcvWndEnd); // Accept an ACK on end of window
    }
    else { // len > 0
        if (state->rcv_wnd == 0)
            ret = false;
        else // rcv_wnd > 0
            ret = (seqLE(state->rcv_nxt, seqNo) && seqLess(seqNo, rcvWndEnd))
                || (seqLess(state->rcv_nxt, seqNo + len) && seqLE(seqNo + len, rcvWndEnd)); // Accept an ACK on end of window
    }

    // RFC 793, page 25:
    // "A new acknowledgment (called an "acceptable ack"), is one for which
    // the inequality below holds:
    //    SND.UNA < SEG.ACK =< SND.NXT"
    if (!ret && len == 0) {
        if (!state->afterRto)
            ret = (seqLess(state->snd_una, ackNo) && seqLE(ackNo, state->snd_nxt));
        else
            ret = (seqLess(state->snd_una, ackNo) && seqLE(ackNo, state->snd_max)); // after RTO snd_nxt is reduced therefore we need to use snd_max instead of snd_nxt here
    }

    if (!ret)
        EV_WARN << "Not Acceptable segment. seqNo=" << seqNo << " ackNo=" << ackNo << " len=" << len << " rcv_nxt="
                << state->rcv_nxt << " rcv_wnd=" << state->rcv_wnd << " afterRto=" << state->afterRto << "\n";

    return ret;
}

void TcpConnection::sendSyn()
{
    if (remoteAddr.isUnspecified() || remotePort == -1)
        throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: foreign socket unspecified");

    if (localPort == -1)
        throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: local port unspecified");

    // create segment
    const auto& tcpHeader = makeShared<TcpHeader>();
    tcpHeader->setSequenceNo(state->iss);
    tcpHeader->setSynBit(true);
    updateRcvWnd();
    tcpHeader->setWindow(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss + 1;

    // ECN
    if (state->ecnWillingness) {
        tcpHeader->setEceBit(true);
        tcpHeader->setCwrBit(true);
        state->ecnSynSent = true;
        EV << "ECN-setup SYN packet sent\n";
    }
    else {
        // rfc 3168 page 16:
        // A host that is not willing to use ECN on a TCP connection SHOULD
        // clear both the ECE and CWR flags in all non-ECN-setup SYN and/or
        // SYN-ACK packets that it sends to indicate this unwillingness.
        tcpHeader->setEceBit(false);
        tcpHeader->setCwrBit(false);
        state->ecnSynSent = false;
//        EV << "non-ECN-setup SYN packet sent\n";
    }

    // write header options
    writeHeaderOptions(tcpHeader);
    Packet *fp = new Packet("SYN");

    // send it
    sendToIP(fp, tcpHeader);
}

void TcpConnection::sendSynAck()
{
    // create segment
    const auto& tcpHeader = makeShared<TcpHeader>();
    tcpHeader->setSequenceNo(state->iss);
    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setSynBit(true);
    tcpHeader->setAckBit(true);
    updateRcvWnd();
    tcpHeader->setWindow(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss + 1;

    // ECN
    if (state->ecnWillingness) {
        tcpHeader->setEceBit(true);
        tcpHeader->setCwrBit(false);
        EV << "ECN-setup SYN-ACK packet sent\n";
    }
    else {
        tcpHeader->setEceBit(false);
        tcpHeader->setCwrBit(false);
        if (state->endPointIsWillingECN)
            EV << "non-ECN-setup SYN-ACK packet sent\n";
    }
    if (state->ecnWillingness && state->endPointIsWillingECN) {
        state->ect = true;
        EV << "both end-points are willing to use ECN... ECN is enabled\n";
    }
    else { // TODO not sure if we have to.
           // rfc-3168, page 16:
           // A host that is not willing to use ECN on a TCP connection SHOULD
           // clear both the ECE and CWR flags in all non-ECN-setup SYN and/or
           // SYN-ACK packets that it sends to indicate this unwillingness.
        state->ect = false;
        if (state->endPointIsWillingECN)
            EV << "ECN is disabled\n";
    }

    // write header options
    writeHeaderOptions(tcpHeader);

    Packet *fp = new Packet("SYN+ACK");

    // send it
    sendToIP(fp, tcpHeader);

    // notify
    tcpAlgorithm->ackSent();
}

void TcpConnection::sendRst(uint32_t seqNo)
{
    sendRst(seqNo, localAddr, remoteAddr, localPort, remotePort);
}

void TcpConnection::sendRst(uint32_t seq, L3Address src, L3Address dest, int srcPort, int destPort)
{
    const auto& tcpHeader = makeShared<TcpHeader>();

    tcpHeader->setSrcPort(srcPort);
    tcpHeader->setDestPort(destPort);

    tcpHeader->setRstBit(true);
    tcpHeader->setSequenceNo(seq);
    tcpHeader->setChecksumMode(tcpMain->checksumMode);
    tcpHeader->setChecksum(0);

    Packet *fp = new Packet("RST");

    // send it
    sendToIP(fp, tcpHeader, src, dest);
}

void TcpConnection::sendRstAck(uint32_t seq, uint32_t ack, L3Address src, L3Address dest, int srcPort, int destPort)
{
    const auto& tcpHeader = makeShared<TcpHeader>();

    tcpHeader->setSrcPort(srcPort);
    tcpHeader->setDestPort(destPort);

    tcpHeader->setRstBit(true);
    tcpHeader->setAckBit(true);
    tcpHeader->setSequenceNo(seq);
    tcpHeader->setAckNo(ack);
    tcpHeader->setChecksumMode(tcpMain->checksumMode);
    tcpHeader->setChecksum(0);

    Packet *fp = new Packet("RST+ACK");

    // send it
    sendToIP(fp, tcpHeader, src, dest);

    // notify
    if (tcpAlgorithm)
        tcpAlgorithm->ackSent();
}

void TcpConnection::sendAck()
{
    const auto& tcpHeader = makeShared<TcpHeader>();

    tcpHeader->setAckBit(true);
    tcpHeader->setSequenceNo(state->snd_nxt);
    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setWindow(updateRcvWnd());

    // rfc-3168, pages 19-20:
    // When TCP receives a CE data packet at the destination end-system, the
    // TCP data receiver sets the ECN-Echo flag in the TCP header of the
    // subsequent ACK packet.
    // ...
    // After a TCP receiver sends an ACK packet with the ECN-Echo bit set,
    // that TCP receiver continues to set the ECN-Echo flag in all the ACK
    // packets it sends (whether they acknowledge CE data packets or non-CE
    // data packets) until it receives a CWR packet (a packet with the CWR
    // flag set).  After the receipt of the CWR packet, acknowledgments for
    // subsequent non-CE data packets do not have the ECN-Echo flag set.

    TcpStateVariables *state = getStateForUpdate();
    if (state && state->ect) {
        if (tcpAlgorithm->shouldMarkAck()) {
            tcpHeader->setEceBit(true);
            EV_INFO << "In ecnEcho state... send ACK with ECE bit set\n";
        }
    }

    // write header options
    writeHeaderOptions(tcpHeader);
    Packet *fp = new Packet("TcpAck");

    // rfc-3168 page 20: pure ack packets must be sent with not-ECT codepoint
    state->sndAck = true;

    // send it
    sendToIP(fp, tcpHeader);

    state->sndAck = false;

    // notify
    tcpAlgorithm->ackSent();
}

void TcpConnection::sendFin()
{
    const auto& tcpHeader = makeShared<TcpHeader>();

    // Note: ACK bit *must* be set for both FIN and FIN+ACK. What makes
    // the difference for FIN+ACK is that its ackNo acks the remote Tcp's FIN.
    tcpHeader->setFinBit(true);
    tcpHeader->setAckBit(true);
    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setSequenceNo(state->snd_nxt);
    tcpHeader->setWindow(updateRcvWnd());
    Packet *fp = new Packet("FIN");

    // send it
    sendToIP(fp, tcpHeader);

    // notify
    tcpAlgorithm->ackSent();
}

uint32_t TcpConnection::sendSegment(uint32_t bytes)
{
    // PMTUD (RFC 1191): if the probe timeout has elapsed since the last MSS reduction,
    // restore the original negotiated MSS to probe for increased path MTU.
    if (state->pmtudEnabled && state->pmtudLastMssReduction >= SIMTIME_ZERO
        && simTime() - state->pmtudLastMssReduction >= state->pmtudTimeout)
    {
        EV_INFO << "PMTUD: probe timeout elapsed, restoring snd_mss from " << state->snd_mss
                << " to original " << state->pmtudOriginalMss << "\n";
        state->snd_mss = state->pmtudOriginalMss;
        state->pmtudLastMssReduction = -1;
    }

    // FIXME check it: where is the right place for the next code (sacked/rexmitted)
    if (state->sack_enabled && state->afterRto) {
        // check rexmitQ and try to forward snd_nxt before sending new data
        uint32_t forward = rexmitQueue->checkRexmitQueueForSackedOrRexmittedSegments(state->snd_nxt);

        if (forward > 0) {
            EV_INFO << "sendSegment(" << bytes << ") forwarded " << forward << " bytes of snd_nxt from " << state->snd_nxt;
            state->snd_nxt += forward;
            EV_INFO << " to " << state->snd_nxt << endl;
            EV_DETAIL << rexmitQueue->detailedInfo();
        }
    }

    uint32_t buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (bytes > buffered) // last segment?
        bytes = buffered;

    // if header options will be added, this could reduce the number of data bytes allowed for this segment,
    // because following condition must to be respected:
    //     bytes + options_len <= snd_mss
    const auto& tmpTcpHeader = makeShared<TcpHeader>();
    tmpTcpHeader->setAckBit(true); // needed for TS option, otherwise TSecr will be set to 0
    writeHeaderOptions(tmpTcpHeader);
    uint options_len = (tmpTcpHeader->getHeaderLength() - TCP_MIN_HEADER_LENGTH).get<B>();

    ASSERT(options_len < state->snd_mss);

    if (bytes + options_len > state->snd_mss)
        bytes = state->snd_mss - options_len;

    uint32_t sentBytes = bytes;

    // send one segment of 'bytes' bytes from snd_nxt, and advance snd_nxt
    Packet *tcpSegment = sendQueue->createSegmentWithBytes(state->snd_nxt, bytes);
    const auto& tcpHeader = makeShared<TcpHeader>();
    tcpHeader->setSequenceNo(state->snd_nxt);
    ASSERT(tcpHeader != nullptr);

    // Remember old_snd_next to store in SACK rexmit queue.
    uint32_t old_snd_nxt = state->snd_nxt;

    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setAckBit(true);
    tcpHeader->setWindow(updateRcvWnd());

    // ECN
    if (state->ect && state->sndCwr) {
        tcpHeader->setCwrBit(true);
        EV_INFO << "set CWR bit\n";
        state->sndCwr = false;
    }

    // TODO when to set PSH bit?
    // TODO set URG bit if needed
    ASSERT(bytes == tcpSegment->getByteLength());

    state->snd_nxt += bytes;

    // check if afterRto bit can be reset
    if (state->afterRto && seqGE(state->snd_nxt, state->snd_max))
        state->afterRto = false;

    if (state->send_fin && state->snd_nxt == state->snd_fin_seq) {
        EV_DETAIL << "Setting FIN on segment\n";
        tcpHeader->setFinBit(true);
        state->snd_nxt = state->snd_fin_seq + 1;
    }

    // if sack_enabled copy region of tcpHeader to rexmitQueue
    if (state->sack_enabled)
        rexmitQueue->enqueueSentData(old_snd_nxt, state->snd_nxt);

    // add header options and update header length (from tcpseg_temp)
    for (uint i = 0; i < tmpTcpHeader->getHeaderOptionArraySize(); i++)
        tcpHeader->appendHeaderOption(tmpTcpHeader->getHeaderOption(i)->dup());
    tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + tcpHeader->getHeaderOptionArrayLength());
    tcpHeader->setChunkLength(B(tcpHeader->getHeaderLength()));

    ASSERT(tcpHeader->getHeaderLength() == tmpTcpHeader->getHeaderLength());

    // send it
    sendToIP(tcpSegment, tcpHeader);

    // let application fill queue again, if there is space
    const uint32_t alreadyQueued = sendQueue->getBytesAvailable(sendQueue->getBufferStartSeq());
    const uint32_t abated = (state->sendQueueLimit > alreadyQueued) ? state->sendQueueLimit - alreadyQueued : 0;
    if ((state->sendQueueLimit > 0) && !state->queueUpdate && (abated >= state->snd_mss)) { // request more data if space >= 1 MSS
        // Tell upper layer readiness to accept more data
        sendIndicationToApp(TCP_I_SEND_MSG, abated);
        state->queueUpdate = true;
    }

    // TCP_NOTSENT_LOWAT: independent low-water-mark check on the
    // not-yet-transmitted portion of the queue (snd_nxt has just advanced past this
    // segment, above). Disarmed/re-armed separately from sendQueueLimit/queueUpdate.
    if (state->notsentLowat != (uint32_t)-1 && !state->notsentLowatUpdate) {
        uint32_t notsentBytes = sendQueue->getBytesAvailable(state->snd_nxt);
        if (notsentBytes <= state->notsentLowat) {
            sendIndicationToApp(TCP_I_SEND_MSG, notsentBytes);
            state->notsentLowatUpdate = true;
        }
    }

    // Minshall bookkeeping (Linux tcp_minshall_update, called only for NEW
    // data in tcp_write_xmit): a freshly sent sub-MSS segment records its end
    // seq so Nagle can hold the NEXT partial until this one is acked.
    // Retransmissions (snd_nxt at or below the old snd_max) don't count.
    // "Small" is judged against what THIS segment could have carried
    // (Linux compares skb->len to pcount * mss_now, the options-adjusted
    // MSS) -- not against snd_effmss, which still includes option headroom.
    if (bytes > 0 && bytes + options_len < state->snd_mss && seqGreater(state->snd_nxt, state->snd_max))
        state->snd_sml = state->snd_nxt;

    // remember highest seq sent (snd_nxt may be set back on retransmission,
    // but we'll need snd_max to check validity of ACKs -- they must ack
    // something we really sent)
    if (seqGreater(state->snd_nxt, state->snd_max)) {
        state->snd_max = state->snd_nxt;
        emit(sndMaxSignal, state->snd_max);
    }

    // Track peak segments in flight (Linux max_packets_out) for the RFC 5681
    // cwnd-limited slow-start gate: an application-limited flow that never fills
    // the congestion window must not be allowed to inflate it. Round up so a
    // partial trailing segment counts as a whole packet (Linux accounts in
    // packets, not bytes).
    if (state->snd_mss > 0) {
        uint32_t packetsOut = (state->snd_max - state->snd_una + state->snd_mss - 1) / state->snd_mss;
        if (packetsOut > state->maxPacketsOut)
            state->maxPacketsOut = packetsOut;
    }

    updateSndbufLimitedChrono(); // this send may have drained the queue

    return sentBytes;
}

void TcpConnection::enqueueSendCommandData(Packet *packet)
{
    // TCP_INFO trio (busy_time): read-only bookkeeping -- if the connection was
    // fully idle (nothing outstanding, nothing queued) before this SEND, it becomes
    // busy now. See processAckInEstabEtc() for the matching "back to idle" exit.
    if (state->busyStartTime < SIMTIME_ZERO && state->snd_una == state->snd_max
        && sendQueue->getBytesAvailable(state->snd_nxt) == 0)
    {
        state->busyStartTime = simTime();
    }

    bool eor = packet->findTag<TcpSendEorReq>() != nullptr;
    bool zerocopy = packet->findTag<TcpSendZerocopyReq>() != nullptr;
    // TCP_CORK / MSG_MORE: MSG_MORE corks this one send's trailing partial. A write
    // WITHOUT MSG_MORE while corking is active marks the held tail for PSH on flush
    // (Linux tcp_mark_push). msgMoreThisSend is consumed at the top of sendData().
    bool msgMore = packet->findTag<TcpSendMoreReq>() != nullptr;
    state->msgMoreThisSend = msgMore;
    if (!msgMore && (state->tcp_cork || state->corkedDataPending))
        state->pushHeldPartial = true;
    uint32_t writeStartSeq = sendQueue->getBufferEndSeq(); // Linux tp->write_seq before this write
    sendQueue->enqueueAppData(packet);
    if (eor) {
        uint32_t boundarySeq = sendQueue->getBufferEndSeq();
        eorSeqNums.insert(boundarySeq);
        EV_DETAIL << "MSG_EOR: recorded record boundary at seq=" << boundarySeq << "\n";
    }
    // Linux tcp_mark_push tags the tail skb of every write (without MSG_MORE)
    // AT WRITE TIME -- the PSH survives later writes queuing behind it and is
    // retained on retransmission. Recording the boundary here (instead of a
    // buffer-drained check at transmit time) is what keeps the last segment of
    // a write PSH-marked even when the next write is already buffered
    // Linux forced_push (tcp_sendmsg copy loop): while a large write is being
    // copied into the send queue, every time an skb fills with more than
    // max_window/2 of data beyond the last PSH mark, that skb is PSH-marked
    // (tcp_mark_push) so the receiver keeps delivering without waiting for the
    // whole write to drain. skb fill geometry: size_goal quantizes
    // tcp_bound_to_half_wnd(max_window/2, aligned by Linux's ALIGN() with the
    // PMTU-derived mss_cache 1460) down to whole MSS units. The mark's PSH
    // surfaces on the marked skb's SECOND mss slice, i.e. the wire segment
    // ending at skbStart + 2*mss.
    if (state->pushOnWriteBoundary && !msgMore && state->max_window > 0) {
        if (seqLess(state->pushed_seq, state->snd_una))
            state->pushed_seq = state->snd_una; // lazy seed: first write of a connection
        uint32_t halfWnd = state->max_window >> 1;
        const uint32_t mssCache = 1460; // Linux tp->mss_cache (IPv4 PMTU 1500 - 40)
        uint32_t alignQ = (halfWnd + mssCache - 1) & ~(mssCache - 1); // Linux ALIGN() verbatim (the macro assumes a power of 2; Linux applies it to mss_cache anyway, so replicate bit-for-bit)
        uint32_t sizeGoal = std::max((alignQ / state->snd_mss) * state->snd_mss, state->snd_mss);
        uint32_t writeEndSeq = sendQueue->getBufferEndSeq();
        for (uint32_t f = writeStartSeq + sizeGoal; seqLE(f, writeEndSeq); f += sizeGoal) {
            if (seqGreater(f, state->pushed_seq + halfWnd)) {
                uint32_t pshSeq = f - sizeGoal + 2 * state->snd_mss;
                forcedPushSeqNums.insert(pshSeq);
                state->pushed_seq = f;
                EV_DETAIL << "forced_push: recorded mid-write PSH boundary at seq=" << pshSeq << "\n";
            }
        }
    }
    if (state->pushOnWriteBoundary && !msgMore) {
        pushSeqNums.insert(sendQueue->getBufferEndSeq());
        state->pushed_seq = sendQueue->getBufferEndSeq(); // Linux tcp_push -> tcp_mark_push on the write's tail skb
    }

    if (zerocopy) {
        uint32_t boundarySeq = sendQueue->getBufferEndSeq();
        uint32_t zerocopyId = nextZerocopyId++;
        zerocopySeqNums[boundarySeq] = zerocopyId;
        EV_DETAIL << "MSG_ZEROCOPY: recorded pending completion id=" << zerocopyId << " at seq=" << boundarySeq << "\n";
    }

    updateSndbufLimitedChrono(); // fresh unsent data: the starved interval (if any) ends
}

int TcpConnection::deriveLinuxCaState() const
{
    if (state->afterRto)
        return 4; // TCP_CA_Loss
    if (state->lossRecovery)
        return 3; // TCP_CA_Recovery
    if (state->sndCwr)
        return 2; // TCP_CA_CWR
    // TCP_CA_Disorder: SACK/dup information has arrived (segments sit above
    // snd_una) but not enough to enter recovery yet -- Linux tcp_fastretrans_alert
    // holds ca_state at Disorder while sacked_out > 0 without a confirmed loss.
    // sackedBytes is kept current on both the SACK and the cumulative-ACK path, so
    // this reverts to Open as soon as snd_una catches up to the SACKed data.
    if (state->sack_enabled && state->sackedBytes > 0)
        return 1; // TCP_CA_Disorder
    return 0; // TCP_CA_Open
}

bool TcpConnection::sendData(uint32_t congestionWindow)
{
    // MSG_MORE corks the trailing partial only for THIS send-driven sendData().
    // Read-and-clear it up front so it never leaks to the ACK-driven or cork-timer
    // send paths (there, only the persistent TCP_CORK holds).
    bool msgMoreHold = state->msgMoreThisSend;
    state->msgMoreThisSend = false;

    // we'll start sending from snd_max, if not after RTO
    if (!state->afterRto)
        state->snd_nxt = state->snd_max;

    uint32_t old_highRxt = 0;

    if (state->sack_enabled)
        old_highRxt = rexmitQueue->getHighestRexmittedSeqNum();

    // check how many bytes we have
    uint32_t buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (buffered == 0)
        return false;

    // The receiver may shrink its advertised window (or close it entirely) while
    // data is still in flight, so snd_wnd can legitimately be smaller than the
    // unacknowledged range -- e.g. during zero-window probing. Saturate at zero
    // instead of underflowing the unsigned subtraction.
    uint32_t unackedInWindow = state->snd_nxt - state->snd_una;
    uint32_t spaceLeftInSendWindow = state->snd_wnd > unackedInWindow ? state->snd_wnd - unackedInWindow : 0;
    uint32_t bytesInFlight = tcpAlgorithm->getBytesInFlight();
    uint32_t spaceLeftInCongestionWindow = bytesInFlight >= congestionWindow ? 0 : congestionWindow - bytesInFlight;

    uint32_t allowedToSend = std::min(spaceLeftInSendWindow, spaceLeftInCongestionWindow);
    // TCP_INFO trio (rwnd_limited): read-only bookkeeping, consulted only by
    // TcpStatusInfo -- never influences the send decision below. "rwnd-limited"
    // here means: there is more buffered data than can be sent right now, and the
    // peer's advertised window (not the congestion window) is the binding
    // constraint.
    bool rwndBinding = (state->snd_wnd < congestionWindow)
        && ((int64_t)buffered > std::max<int64_t>(allowedToSend, 0));
    if (rwndBinding) {
        if (state->rwndLimitedStartTime < SIMTIME_ZERO)
            state->rwndLimitedStartTime = simTime();
    }
    else if (state->rwndLimitedStartTime >= SIMTIME_ZERO) {
        state->rwndLimitedAccumulated += simTime() - state->rwndLimitedStartTime;
        state->rwndLimitedStartTime = -1;
    }

    if (allowedToSend <= 0) {
        EV_WARN << "AllowedToSend is zero (advertised window " << state->snd_wnd
                << ", congestion window " << congestionWindow << "), cannot send.\n";
        return false;
    }

    if (allowedToSend < state->snd_effmss && buffered > allowedToSend) {
        EV_WARN << "Not sending to prevent Silly Window Syndrome.\n";
        return false;
    }

    uint32_t bytesToSend = std::min(buffered, (uint32_t)allowedToSend);

    // make a temporary tcp header for detecting tcp options length (copied from 'TcpConnection::sendSegment(uint32_t bytes)' )
    const auto& tmpTcpHeader = makeShared<TcpHeader>();
    tmpTcpHeader->setAckBit(true); // needed for TS option, otherwise TSecr will be set to 0
    writeHeaderOptions(tmpTcpHeader);
    uint options_len = (tmpTcpHeader->getHeaderLength() - TCP_MIN_HEADER_LENGTH).get<B>();
    ASSERT(options_len < state->snd_mss);
    uint32_t effectiveMss = state->snd_mss - options_len;

    uint32_t old_snd_nxt = state->snd_nxt;

    // start sending 'bytesToSend' bytes
    EV_INFO << "May send " << bytesToSend << " bytes (effectiveWindow " << effectiveWin << ", in buffer " << buffered << " bytes)\n";

    // send whole segments
    while (bytesToSend >= effectiveMss) {
        uint32_t sentBytes = sendSegment(effectiveMss);
        ASSERT(bytesToSend >= sentBytes);
        bytesToSend -= sentBytes;
    }

    if (bytesToSend > 0) {
        // Nagle's algorithm: when a TCP connection has outstanding data that has not
        // yet been acknowledged, small segments cannot be sent until the outstanding
        // data is acknowledged.
        bool unacknowledgedData = (state->snd_una != state->snd_max);
        // The unacknowledged SYN's sequence-number slot is not data: a TCP
        // Fast Open server sending its response from SYN_RCVD still has
        // snd_una at the SYN (iss) with snd_max = iss+1 -- Nagle must not
        // hold the sub-MSS response hostage to a handshake segment (Linux's
        // nagle test walks the data write queue, where the SYN-ACK never
        // appears, so it sends immediately too).
        if (unacknowledgedData && fsm.getState() == TCP_S_SYN_RCVD
                && state->snd_una == state->iss && state->snd_max == state->iss + 1)
            unacknowledgedData = false;
        bool containsFin = state->send_fin && (state->snd_nxt + bytesToSend) == state->snd_fin_seq;
        // TCP_CORK / MSG_MORE hold the trailing sub-MSS partial (full segments were
        // already sent by the loop above). Unlike Nagle, corking holds even with an
        // empty pipe. A flush in progress (corkFlush: uncork/nodelay/timer) bypasses
        // both holds.
        bool corkHold = (state->tcp_cork || msgMoreHold) && !state->corkFlush
                        && !containsFin && buffered < state->snd_effmss;
        // Minshall's variant of the Nagle check (Linux tcp_nagle_check +
        // tcp_minshall_check): a trailing partial is held only while an
        // earlier SMALL segment is still unacknowledged; a pipe full of
        // nothing but full-MSS segments never delays it
        // (client_accecn_options_lost pins the 976-byte tail of a 3000-byte
        // write leaving back-to-back with its two full siblings).
        bool unackedSmallSegment = seqGreater(state->snd_sml, state->snd_una)
                                   && seqLE(state->snd_sml, state->snd_nxt);
        bool nagleHold = state->nagle_enabled && unacknowledgedData && unackedSmallSegment
                         && !containsFin && buffered < state->snd_effmss && !state->corkFlush;
        if (allowedToSend < state->snd_effmss && buffered > allowedToSend)
            EV_WARN << "Not sending to prevent Silly Window Syndrome.\n";
        else if (corkHold || nagleHold) {
            if (corkHold)
                state->corkedDataPending = true;
            EV_WARN << "Holding partial segment ("
                    << (corkHold ? "TCP_CORK/MSG_MORE" : "Nagle") << ")\n";
        }
        else {
            // If this partial is a flush of previously-corked data, it may carry PSH:
            // when the producing write(s) lacked MSG_MORE (pushHeldPartial) or the
            // flush is the cork timer (forcePushHeld). sendSegment reads pushThisSegment.
            bool flushingCorked = state->corkedDataPending || state->corkFlush;
            state->pushThisSegment = flushingCorked && (state->pushHeldPartial || state->forcePushHeld);
            sendSegment(bytesToSend);
            state->pushThisSegment = false;
            state->corkedDataPending = false;
            state->pushHeldPartial = false;
        }
    }

    // Cork (RTO/probe) timer: arm it only while a corked partial is withheld AND
    // nothing is in flight (Linux ICSK_TIME_PROBE0 needs packets_out == 0); an
    // incoming ACK re-runs sendData and flushes otherwise. Disarm as soon as the
    // partial goes out or data becomes outstanding. A Nagle hold always has
    // snd_una != snd_max, so it never arms this timer.
    if (state->corkedDataPending && state->snd_una == state->snd_max)
        tcpAlgorithm->scheduleCorkTimer();
    else
        tcpAlgorithm->cancelCorkTimer();

    if (old_snd_nxt == state->snd_nxt)
        return false; // no data sent

    emit(unackedSignal, state->snd_max - state->snd_una);

    // notify (once is enough)
    tcpAlgorithm->ackSent();

    if (state->sack_enabled && state->lossRecovery && old_highRxt != state->highRxt) {
        // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
        EV_DETAIL << "Retransmission sent during recovery, restarting REXMIT timer.\n";
        tcpAlgorithm->restartRexmitTimer();
    }
    else // don't measure RTT for retransmitted packets
        tcpAlgorithm->dataSent(old_snd_nxt);

    return true;
}

bool TcpConnection::sendProbe()
{
    // we'll start sending from snd_max
    state->snd_nxt = state->snd_max;

    // check we have 1 byte to send
    if (sendQueue->getBytesAvailable(state->snd_nxt) == 0) {
        EV_WARN << "Cannot send probe because send buffer is empty\n";
        return false;
    }

    uint32_t old_snd_nxt = state->snd_nxt;

    EV_INFO << "Sending 1 byte as probe, with seq=" << state->snd_nxt << "\n";
    sendSegment(1);

    // remember highest seq sent (snd_nxt may be set back on retransmission,
    // but we'll need snd_max to check validity of ACKs -- they must ack
    // something we really sent)
    state->snd_max = state->snd_nxt;

    emit(unackedSignal, state->snd_max - state->snd_una);

    // notify
    tcpAlgorithm->ackSent();
    tcpAlgorithm->dataSent(old_snd_nxt);

    return true;
}

void TcpConnection::retransmitOneSegment(bool called_at_rto)
{
    // rfc-3168, page 20:
    // ECN-capable TCP implementations MUST NOT set either ECT codepoint
    // (ECT(0) or ECT(1)) in the IP header for retransmitted data packets
    if (state && state->ect)
        state->rexmit = true;

    uint32_t old_snd_nxt = state->snd_nxt;

    // retransmit one segment at snd_una, and set snd_nxt accordingly (if not called at RTO)
    state->snd_nxt = state->snd_una;

    // The SYN-ACK's sequence slot is not data: a TCP Fast Open server can be
    // retransmitting response data from SYN_RCVD while the SYN-ACK itself is
    // still unacknowledged (snd_una == iss; the SYN-REXMIT timer owns that
    // slot). Data retransmission starts at the first data byte, or the send
    // and rexmit queues (which begin at iss+1) would be walked out of range.
    if (fsm.getState() == TCP_S_SYN_RCVD && seqLess(state->snd_nxt, state->iss + 1))
        state->snd_nxt = state->iss + 1;

    // When FIN sent the snd_max - snd_nxt larger than bytes available in queue
    uint32_t bytes = std::min(std::min(state->snd_effmss, state->snd_max - state->snd_nxt),
                sendQueue->getBytesAvailable(state->snd_nxt));

    // FIN (without user data) needs to be resent
    if (bytes == 0 && state->send_fin && state->snd_fin_seq == sendQueue->getBufferEndSeq()) {
        state->snd_max = sendQueue->getBufferEndSeq();
        EV_DETAIL << "No outstanding DATA, resending FIN, advancing snd_nxt over the FIN\n";
        state->snd_nxt = state->snd_max;
        sendFin();
        tcpAlgorithm->segmentRetransmitted(state->snd_nxt, state->snd_nxt + 1);
        state->snd_max = ++state->snd_nxt;

        emit(unackedSignal, state->snd_max - state->snd_una);
    }
    else {
        ASSERT(bytes != 0);

        sendSegment(bytes);
        tcpAlgorithm->segmentRetransmitted(state->snd_una, state->snd_nxt);

        if (!called_at_rto) {
            if (seqGreater(old_snd_nxt, state->snd_nxt))
                state->snd_nxt = old_snd_nxt;
        }

        // notify
        tcpAlgorithm->ackSent();

        if (state->sack_enabled) {
            // RFC 3517, page 7: "(3) Retransmit the first data segment presumed dropped -- the segment
            // starting with sequence number HighACK + 1.  To prevent repeated
            // retransmission of the same data, set HighRxt to the highest
            // sequence number in the retransmitted segment."
            state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
        }
    }

    if (state && state->ect)
        state->rexmit = false;
}

void TcpConnection::retransmitData()
{
    // rfc-3168, page 20:
    // ECN-capable TCP implementations MUST NOT set either ECT codepoint
    // (ECT(0) or ECT(1)) in the IP header for retransmitted data packets
    if (state && state->ect)
        state->rexmit = true;

    // retransmit everything from snd_una
    state->snd_nxt = state->snd_una;

    uint32_t bytesToSend = state->snd_max - state->snd_nxt;

    // FIN (without user data) needs to be resent
    if (bytesToSend == 0 && state->send_fin && state->snd_fin_seq == sendQueue->getBufferEndSeq()) {
        state->snd_max = sendQueue->getBufferEndSeq();
        EV_DETAIL << "No outstanding DATA, resending FIN, advancing snd_nxt over the FIN\n";
        state->snd_nxt = state->snd_max;
        sendFin();
        state->snd_max = ++state->snd_nxt;

        emit(unackedSignal, state->snd_max - state->snd_una);
        return;
    }

    ASSERT(bytesToSend != 0);

    // TODO - avoid to send more than allowed - check cwnd and rwnd before retransmitting data!
    while (bytesToSend > 0) {
        uint32_t bytes = std::min(bytesToSend, state->snd_effmss);
        bytes = std::min(bytes, sendQueue->getBytesAvailable(state->snd_nxt));
        uint32_t sentBytes = sendSegment(bytes);

        // Do not send packets after the FIN.
        // fixes bug that occurs in examples/inet/bulktransfer at event #64043  T=13.861159213744
        if (state->send_fin && state->snd_nxt == state->snd_fin_seq + 1)
            break;

        ASSERT(bytesToSend >= sentBytes);
        bytesToSend -= sentBytes;
    }
    tcpAlgorithm->segmentRetransmitted(state->snd_una, state->snd_nxt);

    if (state && state->ect)
        state->rexmit = false;
}

void TcpConnection::readHeaderOptions(const Ptr<const TcpHeader>& tcpHeader)
{
    EV_INFO << "Tcp Header Option(s) received:\n";

    for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        short kind = option->getKind();
        short length = option->getLength();
        bool ok = true;

        EV_DETAIL << "Option type " << kind << " (" << optionName(kind) << "), length " << length << "\n";

        switch (kind) {
            case TCPOPTION_END_OF_OPTION_LIST: // EOL=0
            case TCPOPTION_NO_OPERATION: // NOP=1
                if (length != 1) {
                    EV_ERROR << "ERROR: option length incorrect\n";
                    ok = false;
                }
                break;

            case TCPOPTION_MAXIMUM_SEGMENT_SIZE: // MSS=2
                ok = processMSSOption(tcpHeader, *check_and_cast<const TcpOptionMaxSegmentSize *>(option));
                break;

            case TCPOPTION_WINDOW_SCALE: // WS=3
                ok = processWSOption(tcpHeader, *check_and_cast<const TcpOptionWindowScale *>(option));
                break;

            case TCPOPTION_SACK_PERMITTED: // SACK_PERMITTED=4
                ok = processSACKPermittedOption(tcpHeader, *check_and_cast<const TcpOptionSackPermitted *>(option));
                break;

            case TCPOPTION_SACK: // SACK=5
                ok = processSACKOption(tcpHeader, *check_and_cast<const TcpOptionSack *>(option));
                break;

            case TCPOPTION_TIMESTAMP: // TS=8
                ok = processTSOption(tcpHeader, *check_and_cast<const TcpOptionTimestamp *>(option));
                break;

            // TODO add new TCPOptions here once they are implemented
            // TODO delegate to TcpAlgorithm as well -- it may want to recognized additional options

            default:
                EV_ERROR << "ERROR: Unsupported Tcp option kind " << kind << "\n";
                break;
        }
        (void)ok; // unused
    }
}

bool TcpConnection::processMSSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionMaxSegmentSize& option)
{
    if (option.getLength() != 4) {
        EV_ERROR << "ERROR: MSS option length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: Tcp Header Option MSS received, but in unexpected state\n";
        return false;
    }

    // RFC 5681, page 3:
    // "The SMSS is the size of the largest segment that the sender can transmit.
    // This value can be based on the maximum transmission unit of the network,
    // the path MTU discovery [RFC1191, RFC4821] algorithm, RMSS (see next item), or other
    // factors.  The size does not include the TCP/IP headers and options."
    //
    // "The RMSS is the size of the largest segment the receiver is willing to accept.
    // This is the value specified in the MSS option sent by the receiver during
    // connection startup.  Or, if the MSS option is not used, it is 536 bytes [RFC1122].
    // The size does not include the TCP/IP headers and options."
    //
    //
    // The value of snd_mss (SMSS) is set to the minimum of snd_mss (local parameter) and
    // the value specified in the MSS option received during connection startup.
    state->peerAdvertisedMss = option.getMaxSegmentSize(); // raw, pre-clamp (TFO metrics cache)
    state->snd_mss = std::min(state->snd_mss, (uint32_t)option.getMaxSegmentSize());

    if (state->snd_mss == 0)
        // RFC 9293, section 3.7.1
        //"
        // If an MSS Option is not received at connection setup,
        // TCP implementations MUST assume a default send MSS of
        // 536 (576 - 40) for IPv4 or 1220 (1280 - 60) for IPv6 (MUST-15).
        //"
        state->snd_mss = remoteAddr.getType() == L3Address::IPv4 ? 536 : 1220;

    // Store negotiated MSS for PMTUD: this is the value we restore after the probe timeout
    state->pmtudOriginalMss = state->snd_mss;
    state->snd_effmss = calculateEffectiveMss();

    EV_INFO << "Tcp Header Option MSS(=" << option.getMaxSegmentSize() << ") received, SMSS is set to " << state->snd_mss << "\n";
    return true;
}

bool TcpConnection::processWSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionWindowScale& option)
{
    if (option.getLength() != 3) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: Tcp Header Option WS received, but in unexpected state\n";
        return false;
    }

    state->rcv_ws = true;
    state->ws_enabled = state->ws_support && state->snd_ws && state->rcv_ws;
    state->snd_wnd_scale = option.getWindowScale();
    EV_INFO << "Tcp Header Option WS(=" << state->snd_wnd_scale << ") received, WS (ws_enabled) is set to " << state->ws_enabled << "\n";

    if (state->snd_wnd_scale > 14) { // RFC 1323, page 11: "the shift count must be limited to 14"
        EV_ERROR << "ERROR: Tcp Header Option WS received but shift count value is exceeding 14\n";
        state->snd_wnd_scale = 14;
    }

    return true;
}

bool TcpConnection::processTSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTimestamp& option)
{
    if (option.getLength() != 10) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if ((!state->ts_enabled && fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) ||
        (state->ts_enabled && fsm.getState() != TCP_S_SYN_RCVD && fsm.getState() != TCP_S_ESTABLISHED &&
         fsm.getState() != TCP_S_FIN_WAIT_1 && fsm.getState() != TCP_S_FIN_WAIT_2))
    {
        EV_ERROR << "ERROR: Tcp Header Option TS received, but in unexpected state\n";
        return false;
    }

    if (!state->ts_enabled) {
        state->rcv_initial_ts = true;
        state->ts_enabled = state->ts_support && state->snd_initial_ts && state->rcv_initial_ts;
        EV_INFO << "Tcp Header Option TS(TSval=" << option.getSenderTimestamp() << ", TSecr=" << option.getEchoedTimestamp() << ") received, TS (ts_enabled) is set to " << state->ts_enabled << "\n";
    }
    else
        EV_INFO << "Tcp Header Option TS(TSval=" << option.getSenderTimestamp() << ", TSecr=" << option.getEchoedTimestamp() << ") received\n";

    // RFC 1323, page 35:
    // "Check whether the segment contains a Timestamps option and bit
    // Snd.TS.OK is on.  If so:
    //   If SEG.TSval < TS.Recent, then test whether connection has
    //   been idle less than 24 days; if both are true, then the
    //   segment is not acceptable; follow steps below for an
    //   unacceptable segment.
    //   If SEG.SEQ is equal to Last.ACK.sent, then save SEG.[TSval] in
    //   variable TS.Recent."
    if (state->ts_enabled) {
        if (seqLess(option.getSenderTimestamp(), state->ts_recent)) {
            if ((simTime() - state->time_last_data_sent) > PAWS_IDLE_TIME_THRESH) { // PAWS_IDLE_TIME_THRESH = 24 days
                EV_DETAIL << "PAWS: Segment is not acceptable, TSval=" << option.getSenderTimestamp() << " in " << stateName(fsm.getState()) << " state received: dropping segment\n";
                return false;
            }
        }
        else if (seqLE(tcpHeader->getSequenceNo(), state->last_ack_sent)) { // Note: test is modified according to the latest proposal of the tcplw@cray.com list (Braden 1993/04/26)
            state->ts_recent = option.getSenderTimestamp();
            EV_DETAIL << "Updating ts_recent from segment: new ts_recent=" << state->ts_recent << "\n";
        }
    }

    return true;
}

bool TcpConnection::processSACKPermittedOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionSackPermitted& option)
{
    if (option.getLength() != 2) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: Tcp Header Option SACK_PERMITTED received, but in unexpected state\n";
        return false;
    }

    state->rcv_sack_perm = true;
    state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
    EV_INFO << "Tcp Header Option SACK_PERMITTED received, SACK (sack_enabled) is set to " << state->sack_enabled << "\n";
    return true;
}

uint32_t TcpConnection::calculateEffectiveMss()
{
    // calculate mss minus TCP options length for cwnd calculations
    // TCP options used during the handshake is ignored
    // only TCP options used in established connections are considered
    // we only support two such options: timestamp and sack options
    // we only calculate with the timestamp option
    // the sack option is ignored because it is variable width and
    // the number of sack blocks is not yet known when this value is needed
    // also it is not important during recovery and during bidirectional traffic
    return state->snd_mss - (state->ts_enabled ? 10 + 1 + 1 : 0); // timestamp option + end of options + padding
}

TcpHeader TcpConnection::writeHeaderOptions(const Ptr<TcpHeader>& tcpHeader)
{
    // SYN flag set and the connection state is INIT or LISTEN state (or after synRexmit
    // timeout, or sending a TCP Fast Open deferred SYN for the first time --
    // fastopenSynDeferred stays true through sendSyn() itself for exactly this purpose,
    // see process_SEND)
    if (state->advertisedMss == (uint32_t)-1)
        state->advertisedMss = (remoteAddr.getType() == L3Address::IPv6) ? 1440 : 1460;
    // Resolve the address-family-derived default MSS (mss = -1 sentinel) now that
    // the remote address is known and before we advertise/use it. IPv6 has 40 bytes
    // of base header vs IPv4's 20, so a 1500-byte MTU yields 1440 vs 1460.
    if (state->snd_mss == (uint32_t)-1) {
        state->snd_mss = (remoteAddr.getType() == L3Address::IPv6) ? 1440 : 1460;
        state->snd_effmss = calculateEffectiveMss();
        EV_DETAIL << "Derived default MSS from address family: snd_mss=" << state->snd_mss << "\n";
    }

    if (tcpHeader->getSynBit() && (fsm.getState() == TCP_S_INIT || fsm.getState() == TCP_S_LISTEN
                                || ((fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD)
                                    && (state->syn_rexmit_count > 0 || state->fastopenSynDeferred
                                        // simultaneous open: the crossing-SYN reply (a first
                                        // SYN-ACK sent while still in SYN_SENT) carries the
                                        // full handshake option set, same as any other
                                        // handshake segment -- without this it fell into the
                                        // established-states branch and went out bare
                                        || tcpHeader->getAckBit()))))
    {
        // MSS header option: announces OUR receive limit (advertisedMss), not
        // snd_mss -- by SYN-ACK time snd_mss is already clamped to the peer's
        // announced MSS, and echoing that back is wrong (RFC 793/9293: each
        // side announces its own limit; Linux advertises its own 1460 in the
        // SYN-ACK regardless of the client's smaller MSS).
        if (tcpMain->sendMssOption && state->advertisedMss > 0) {
            TcpOptionMaxSegmentSize *option = new TcpOptionMaxSegmentSize();
            option->setMaxSegmentSize(state->advertisedMss);
            tcpHeader->appendHeaderOption(option);
            EV_INFO << "Tcp Header Option MSS(=" << state->advertisedMss << ") sent\n";
        }

        // TS header option
        if (state->ts_support && (state->rcv_initial_ts || (fsm.getState() == TCP_S_INIT
                                                            || (fsm.getState() == TCP_S_SYN_SENT && (state->syn_rexmit_count > 0 || state->fastopenSynDeferred)))))
        {
            if (tcpMain->alignOptions && !state->sack_support) { // if SACK is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
            }

            TcpOptionTimestamp *option = new TcpOptionTimestamp();

            // Update TS variables
            // RFC 7323, page 12: "The TSval field contains the current value of the timestamp clock of the Tcp sending the option."
            option->setSenderTimestamp(convertSimtimeToTS(simTime()));

            // RFC 7323, page 17: "(3) When a TSopt is sent, its TSecr field is set to the current TS.Recent value."
            // RFC 7323, page 12:
            // "The TSecr field is valid if the ACK bit is set in the TCP header.  If
            // the ACK bit is not set in the outgoing TCP header, the sender of that
            // segment SHOULD set the TSecr field to zero.  When the ACK bit is set
            // in an outgoing segment, the sender MUST echo a recently received
            // TSval sent by the remote TCP in the TSval field of a Timestamps
            // option."
            option->setEchoedTimestamp(tcpHeader->getAckBit() ? state->ts_recent : 0);

            state->snd_initial_ts = true;
            state->ts_enabled = state->ts_support && state->snd_initial_ts && state->rcv_initial_ts;
            EV_INFO << "Tcp Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent, TS (ts_enabled) is set to " << state->ts_enabled << "\n";
            tcpHeader->appendHeaderOption(option);
        }

        // TCP Fast Open (RFC 7413) cookie option, server side: echo a (possibly
        // fresh) cookie in the SYN-ACK. 2 trailing NOPs pad the 2- or 10-byte
        // (with the default 8-byte cookie) option to a 4-byte-aligned option area --
        // unlike MSS/WS/SACK_PERMITTED/TS, no other option's NOPs can double up here
        // since TFO is server-to-client-only at this point in the plan.
        if (state->fastopenServerEnabled && state->fastopenSendCookieOption) {
            if (state->fastopenPeerUsedExpOption) {
                // Echo in the experimental form the client used (RFC 7413
                // Appendix A: kind 254 + 0xF989 magic), as Linux does
                // (foc->exp propagates request->response). The 12-byte option
                // (4 base + 8 cookie) is 4-byte-aligned on its own, so no NOP
                // padding -- matches the corpus's "<mss 1460,nop,nop,sackOK,
                // FOEXP ...>" SYN-ACK layout.
                TcpOptionTcpFastOpenExp *option = new TcpOptionTcpFastOpenExp();
                option->setExpId(0xF989);
                option->setCookieArraySize(state->fastopenCookieToSend.size());
                for (size_t i = 0; i < state->fastopenCookieToSend.size(); i++)
                    option->setCookie(i, state->fastopenCookieToSend[i]);
                option->setLength(4 + state->fastopenCookieToSend.size());
                tcpHeader->appendHeaderOption(option);
            }
            else {
                tcpHeader->appendHeaderOption(new TcpOptionNop());
                tcpHeader->appendHeaderOption(new TcpOptionNop());
                TcpOptionTcpFastOpen *fastOpenOption = new TcpOptionTcpFastOpen();
                fastOpenOption->setCookieArraySize(state->fastopenCookieToSend.size());
                for (size_t i = 0; i < state->fastopenCookieToSend.size(); i++)
                    fastOpenOption->setCookie(i, state->fastopenCookieToSend[i]);
                fastOpenOption->setLength(2 + state->fastopenCookieToSend.size());
                tcpHeader->appendHeaderOption(fastOpenOption);
            }
            EV_INFO << "Tcp Header Option Fast Open cookie (" << state->fastopenCookieToSend.size()
                    << " bytes, kind " << (state->fastopenPeerUsedExpOption ? 254 : 34) << ") sent\n";
        }

        // TCP Fast Open (RFC 7413) cookie option, client side: echo the cached
        // cookie (data-bearing SYN, process_SEND's deferred path) or an empty
        // cookie (dataless SYN requesting one, process_OPEN_ACTIVE's immediate path).
        // Gated on fastopenRequested (this connection's own connect() opted in), not
        // just the module-wide fastopenClientEnabled param -- otherwise a plain
        // connect() to a destination with a cookie cached from an earlier TFO
        // connection would attach that cookie uninvited, which Linux's per-connection
        // opt-in (MSG_FASTOPEN / TCP_FASTOPEN_CONNECT) never does.
        // Linux drops the Fast Open option on SYN retransmits (RFC 7413
        // section 4.1.3 / tcp_retransmit_skb clearing the fastopen request:
        // a lost option-bearing SYN plausibly means a middlebox ate it) --
        // the corpus pins this ("SYN retransmit should not include Fast Open
        // Cookie Request", cookie-req-timeout; and the data-SYN rexmits in
        // syn-data-timeout / *-sendto-errnos are bare too).
        // ... never on an ACK-bearing SYN (a simultaneous-open SYN-ACK carries
        // no client cookie in Linux), and never in cookie-less client mode
        // (tcp_fastopen bit 0x4: the SYN+data goes out with NO FO option even
        // when a cookie happens to be cached -- tcp_fastopen_no_cookie()).
        if (state->fastopenClientEnabled && state->fastopenRequested && state->syn_rexmit_count == 0
            && !tcpHeader->getAckBit() && !tcpMain->par("fastopenClientNoCookieRequired").boolValue()) {
            std::vector<uint8_t> cachedCookie;
            // fastopenCookieRequestPending is the authoritative "this connection is in
            // cookie-REQUEST mode" signal set once, at connect() time, by
            // process_OPEN_ACTIVE -- true both for a genuinely empty cache and for a
            // cache hit overridden by isActiveFastOpenDisabled() (blackhole detection,
            // F5.1): either way this SYN must look like "no cookie cached" (an empty
            // request), not silently reveal a real cached cookie it chose not to use.
            // Deliberately NOT re-checking isActiveFastOpenDisabled() here directly --
            // that would also suppress an *already*-deferred connection's own SYN
            // retransmissions if blackhole detection trips mid-flight (after this
            // connection committed to using the cache), corrupting an in-flight
            // data-bearing SYN into a data-bearing-but-cookie-less one.
            bool haveCachedCookie = !state->fastopenCookieRequestPending && tcpMain->getFastOpenCookie(remoteAddr, cachedCookie);
            if (haveCachedCookie || state->fastopenCookieRequestPending) {
                tcpHeader->appendHeaderOption(new TcpOptionNop());
                tcpHeader->appendHeaderOption(new TcpOptionNop());
                TcpOptionTcpFastOpen *clientOption = new TcpOptionTcpFastOpen();
                clientOption->setCookieArraySize(cachedCookie.size());
                for (size_t i = 0; i < cachedCookie.size(); i++)
                    clientOption->setCookie(i, cachedCookie[i]);
                clientOption->setLength(2 + cachedCookie.size());
                tcpHeader->appendHeaderOption(clientOption);
                state->fastopenSynCarriedOption = true; // Linux tp->syn_fastopen
                EV_INFO << "Tcp Header Option Fast Open cookie (" << cachedCookie.size() << " bytes) sent\n";
            }
        }

        // WS header option
        if (state->ws_support && (state->rcv_ws || (fsm.getState() == TCP_S_INIT
                                                    || (fsm.getState() == TCP_S_SYN_SENT && (state->syn_rexmit_count > 0 || state->fastopenSynDeferred)))))
        {
            if (tcpMain->alignOptions) // align
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP

            // Update WS variables
            if (state->ws_manual_scale > -1) {
                state->rcv_wnd_scale = state->ws_manual_scale;
            }
            else {
                ulong scaled_rcv_wnd = state->maxRcvBuffer - receiveQueue->getAcknowledgedDataLength();
                state->rcv_wnd_scale = 0;

                while (scaled_rcv_wnd > TCP_MAX_WIN && state->rcv_wnd_scale < 14) { // RFC 7323, page 10: "the shift count must be limited to 14"
                    scaled_rcv_wnd = scaled_rcv_wnd >> 1;
                    state->rcv_wnd_scale++;
                }
            }

            TcpOptionWindowScale *option = new TcpOptionWindowScale();
            option->setWindowScale(state->rcv_wnd_scale); // rcv_wnd_scale is also set in scaleRcvWnd()
            state->snd_ws = true;
            state->ws_enabled = state->ws_support && state->snd_ws && state->rcv_ws;
            EV_INFO << "Tcp Header Option WS(=" << option->getWindowScale() << ") sent, WS (ws_enabled) is set to " << state->ws_enabled << "\n";
            tcpHeader->appendHeaderOption(option);
        }

        // SACK_PERMITTED header option
        if (state->sack_support && (state->rcv_sack_perm || (fsm.getState() == TCP_S_INIT
                                                             || (fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count > 0))))
        {
            if (!state->ts_support) { // if TS is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
            }

            tcpHeader->appendHeaderOption(new TcpOptionSackPermitted());

            // Update SACK variables
            state->snd_sack_perm = true;
            state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
            EV_INFO << "Tcp Header Option SACK_PERMITTED sent, SACK (sack_enabled) is set to " << state->sack_enabled << "\n";
        }

        // TS header option
        if (state->ts_support && (state->rcv_initial_ts || (fsm.getState() == TCP_S_INIT
                                                            || (fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count > 0))))
        {
            if (!state->sack_support) { // if SACK is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
            }

            TcpOptionTimestamp *option = new TcpOptionTimestamp();

            // Update TS variables
            // RFC 1323, page 13: "The Timestamp Value field (TSval) contains the current value of the timestamp clock of the Tcp sending the option."
            option->setSenderTimestamp(convertSimtimeToTS(simTime()));

            // RFC 1323, page 16: "(3) When a TSopt is sent, its TSecr field is set to the current TS.Recent value."
            // RFC 1323, page 13:
            // "The Timestamp Echo Reply field (TSecr) is only valid if the ACK
            // bit is set in the Tcp header; if it is valid, it echos a times-
            // tamp value that was sent by the remote Tcp in the TSval field
            // of a Timestamps option.  When TSecr is not valid, its value
            // must be zero."
            option->setEchoedTimestamp(tcpHeader->getAckBit() ? state->ts_recent : 0);

            state->snd_initial_ts = true;
            state->ts_enabled = state->ts_support && state->snd_initial_ts && state->rcv_initial_ts;
            EV_INFO << "Tcp Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent, TS (ts_enabled) is set to " << state->ts_enabled << "\n";
            tcpHeader->appendHeaderOption(option);
        }

        // TODO add new TCPOptions here once they are implemented
    }
    else if (fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD
             || fsm.getState() == TCP_S_ESTABLISHED || fsm.getState() == TCP_S_FIN_WAIT_1
             || fsm.getState() == TCP_S_FIN_WAIT_2 || fsm.getState() == TCP_S_CLOSE_WAIT) // connetion is not in INIT or LISTEN state
    {
        // TS header option
        if (state->ts_enabled) { // Is TS enabled?
            if (!(state->sack_enabled && (state->snd_sack || state->snd_dsack))) { // if SACK is enabled and SACKs need to be added, do not add NOPs to this segment
                // 2 padding bytes
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
            }

            TcpOptionTimestamp *option = new TcpOptionTimestamp();

            // Update TS variables
            // RFC 1323, page 13: "The Timestamp Value field (TSval) contains the current value of the timestamp clock of the Tcp sending the option."
            option->setSenderTimestamp(convertSimtimeToTS(simTime()));

            // RFC 1323, page 16: "(3) When a TSopt is sent, its TSecr field is set to the current TS.Recent value."
            // RFC 1323, page 13:
            // "The Timestamp Echo Reply field (TSecr) is only valid if the ACK
            // bit is set in the Tcp header; if it is valid, it echos a times-
            // tamp value that was sent by the remote Tcp in the TSval field
            // of a Timestamps option.  When TSecr is not valid, its value
            // must be zero."
            option->setEchoedTimestamp(tcpHeader->getAckBit() ? state->ts_recent : 0);

            EV_INFO << "Tcp Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent\n";
            tcpHeader->appendHeaderOption(option);
        }

        // SACK header option

        // RFC 2018, page 4:
        // "If sent at all, SACK options SHOULD be included in all ACKs which do
        // not ACK the highest sequence number in the data receiver's queue.  In
        // this situation the network has lost or mis-ordered data, such that
        // the receiver holds non-contiguous data in its queue.  RFC 1122,
        // Section 4.2.2.21, discusses the reasons for the receiver to send ACKs
        // in response to additional segments received in this state.  The
        // receiver SHOULD send an ACK for every valid segment that arrives
        // containing new data, and each of these "duplicate" ACKs SHOULD bear a
        // SACK option."
        if (state->sack_enabled && (state->snd_sack || state->snd_dsack)) {
            addSacks(tcpHeader);
        }

        // TODO add new TCPOptions here once they are implemented
        // TODO delegate to TcpAlgorithm as well -- it may want to append additional options
    }

    if (tcpHeader->getHeaderOptionArraySize() != 0) {
        B options_len = tcpHeader->getHeaderOptionArrayLength();

        if (options_len <= TCP_OPTIONS_MAX_SIZE) { // Options length allowed? - maximum: 40 Bytes
            tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + options_len);
            tcpHeader->setChunkLength(B(TCP_MIN_HEADER_LENGTH + options_len));
        }
        else {
            tcpHeader->dropHeaderOptions(); // drop all options
            tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH);
            tcpHeader->setChunkLength(TCP_MIN_HEADER_LENGTH);
            EV_ERROR << "ERROR: Options length exceeded! Segment will be sent without options" << "\n";
        }
    }

    return *tcpHeader;
}

uint32_t TcpConnection::getTSval(const Ptr<const TcpHeader>& tcpHeader) const
{
    for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_TIMESTAMP)
            return check_and_cast<const TcpOptionTimestamp *>(option)->getSenderTimestamp();
    }

    return 0;
}

uint32_t TcpConnection::getTSecr(const Ptr<const TcpHeader>& tcpHeader) const
{
    for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_TIMESTAMP)
            return check_and_cast<const TcpOptionTimestamp *>(option)->getEchoedTimestamp();
    }

    return 0;
}

void TcpConnection::updateRcvQueueVars()
{
    // update receive queue related state variables
    state->freeRcvBuffer = receiveQueue->getAmountOfFreeBytes(state->maxRcvBuffer);
    state->usedRcvBuffer = state->maxRcvBuffer - state->freeRcvBuffer;

    // update receive queue related statistics
    emit(tcpRcvQueueBytesSignal, state->usedRcvBuffer);

//    tcpEV << "receiveQ: receiveQLength=" << receiveQueue->getQueueLength() << " maxRcvBuffer=" << state->maxRcvBuffer << " usedRcvBuffer=" << state->usedRcvBuffer << " freeRcvBuffer=" << state->freeRcvBuffer << "\n";
}

uint16_t TcpConnection::updateRcvWnd()
{
    uint32_t win = 0;

    // update receive queue related state variables and statistics
    updateRcvQueueVars();
    win = state->freeRcvBuffer;

    // Following lines are based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, chapter 26.7, pages 878-879]:
    // Don't advertise less than one full-sized segment to avoid SWS
    if (win < (state->maxRcvBuffer / 4) && win < state->snd_mss)
        win = 0;

    // Do not shrink window
    // (rcv_adv minus rcv_nxt) is the amount of space still available to the sender that was previously advertised
    if (win < state->rcv_adv - state->rcv_nxt)
        win = state->rcv_adv - state->rcv_nxt;

    // Observe upper limit for advertised window on this connection
    const uint32_t maxWin = (state->ws_enabled && state->rcv_wnd_scale) ? (TCP_MAX_WIN << state->rcv_wnd_scale) : TCP_MAX_WIN; // TCP_MAX_WIN = 65535 (16 bit)
    if (win > maxWin)
        win = maxWin; // Note: The window size is limited to a 16 bit value in the TCP header if WINDOW SCALE option (RFC 1323) is not used

    // Note: The order of the "Do not shrink window" and "Observe upper limit" parts has been changed to the order used in FreeBSD Release 7.1

    // update rcv_adv if needed
    if (win > 0 && seqGE(state->rcv_nxt + win, state->rcv_adv)) {
        state->rcv_adv = state->rcv_nxt + win;

        emit(rcvAdvSignal, state->rcv_adv);
    }

    state->rcv_wnd = win;

    emit(rcvWndSignal, state->rcv_wnd);

    // scale rcv_wnd:
    uint32_t scaled_rcv_wnd = state->rcv_wnd;
    if (state->ws_enabled && state->rcv_wnd_scale) {
        ASSERT(state->rcv_wnd_scale <= 14); // RFC 1323, page 11: "the shift count must be limited to 14"
        scaled_rcv_wnd = scaled_rcv_wnd >> state->rcv_wnd_scale;
    }

    ASSERT(scaled_rcv_wnd == (uint16_t)scaled_rcv_wnd);

    return (uint16_t)scaled_rcv_wnd;
}

void TcpConnection::updateWndInfo(const Ptr<const TcpHeader>& tcpHeader, bool doAlways)
{
    uint32_t true_window = tcpHeader->getWindow();
    // RFC 1323, page 10:
    // "The window field (SEG.WND) in the header of every incoming
    // segment, with the exception of SYN segments, is left-shifted
    // by Snd.Wind.Scale bits before updating SND.WND:
    //    SND.WND = SEG.WND << Snd.Wind.Scale"
    if (state->ws_enabled && !tcpHeader->getSynBit())
        true_window = tcpHeader->getWindow() << state->snd_wnd_scale;

    // Following lines are based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 982]:
    if (doAlways || (tcpHeader->getAckBit()
                     && (seqLess(state->snd_wl1, tcpHeader->getSequenceNo()) ||
                         (state->snd_wl1 == tcpHeader->getSequenceNo() && seqLE(state->snd_wl2, tcpHeader->getAckNo())) ||
                         (state->snd_wl2 == tcpHeader->getAckNo() && true_window > state->snd_wnd))))
    {
        // send window should be updated
        state->snd_wnd = true_window;
        EV_INFO << "Updating send window from segment: new wnd=" << state->snd_wnd << "\n";
        state->snd_wl1 = tcpHeader->getSequenceNo();
        state->snd_wl2 = tcpHeader->getAckNo();

        emit(sndWndSignal, state->snd_wnd);
    }
}

void TcpConnection::sendOneNewSegment(bool fullSegmentsOnly, uint32_t congestionWindow)
{
    ASSERT(state->limited_transmit_enabled);

    // RFC 3042, page 3:
    // "When a TCP sender has previously unsent data queued for transmission
    // it SHOULD use the Limited Transmit algorithm, which calls for a TCP
    // sender to transmit new data upon the arrival of the first two
    // consecutive duplicate ACKs when the following conditions are
    // satisfied:
    //
    //  * The receiver's advertised window allows the transmission of the
    //  segment.
    //
    //  * The amount of outstanding data would remain less than or equal
    //  to the congestion window plus 2 segments.  In other words, the
    //  sender can only send two segments beyond the congestion window
    //  (cwnd).
    //
    // The congestion window (cwnd) MUST NOT be changed when these new
    // segments are transmitted.  Assuming that these new segments and the
    // corresponding ACKs are not dropped, this procedure allows the sender
    // to infer loss using the standard Fast Retransmit threshold of three
    // duplicate ACKs [RFC2581].  This is more robust to reordered packets
    // than if an old packet were retransmitted on the first or second
    // duplicate ACK.
    //
    // Note: If the connection is using selective acknowledgments [RFC2018],
    // the data sender MUST NOT send new segments in response to duplicate
    // ACKs that contain no new SACK information, as a misbehaving receiver
    // can generate such ACKs to trigger inappropriate transmission of data
    // segments.  See [SCWA99] for a discussion of attacks by misbehaving
    // receivers."
    if (!state->sack_enabled || (state->sack_enabled && state->sackedBytes_old != state->sackedBytes)) {
        // check how many bytes we have
        uint32_t buffered = sendQueue->getBytesAvailable(state->snd_max);

        if (buffered >= state->snd_mss || (!fullSegmentsOnly && buffered > 0)) {
            uint32_t outstandingData = state->snd_max - state->snd_una;

            // check conditions from RFC 3042
            if (outstandingData + state->snd_mss <= state->snd_wnd &&
                outstandingData + state->snd_mss <= congestionWindow + 2 * state->snd_mss)
            {
                // RFC 3042, page 3: "(...)the sender can only send two segments beyond the congestion window (cwnd)."
                uint32_t effectiveWin = std::min(state->snd_wnd, congestionWindow) - outstandingData + 2 * state->snd_mss;

                // bytes: number of bytes we're allowed to send now
                uint32_t bytes = std::min(effectiveWin, state->snd_mss);

                if (bytes >= state->snd_mss || (!fullSegmentsOnly && bytes > 0)) {
                    uint32_t old_snd_nxt = state->snd_nxt;
                    // we'll start sending from snd_max
                    state->snd_nxt = state->snd_max;

                    EV_DETAIL << "Limited Transmit algorithm enabled. Sending one new segment.\n";
                    uint32_t sentBytes = sendSegment(bytes);

                    if (seqGreater(state->snd_nxt, state->snd_max))
                        state->snd_max = state->snd_nxt;

                    emit(unackedSignal, state->snd_max - state->snd_una);

                    // reset snd_nxt if needed
                    if (state->afterRto)
                        state->snd_nxt = old_snd_nxt + sentBytes;

                    // notify
                    tcpAlgorithm->ackSent();
                    tcpAlgorithm->dataSent(old_snd_nxt);
                }
            }
        }
    }
}

uint32_t TcpConnection::convertSimtimeToTS(simtime_t simtime)
{
    ASSERT(SimTime::getScaleExp() <= -3);

    uint32_t timestamp = (uint32_t)(simtime.inUnit(SIMTIME_MS));
    return timestamp;
}

simtime_t TcpConnection::convertTSToSimtime(uint32_t timestamp)
{
    ASSERT(SimTime::getScaleExp() <= -3);

    simtime_t simtime(timestamp, SIMTIME_MS);
    return simtime;
}

bool TcpConnection::isSendQueueEmpty()
{
    return sendQueue->getBytesAvailable(state->snd_nxt) == 0;
}

} // namespace tcp
} // namespace inet

