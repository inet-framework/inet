//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009-2011 Thomas Reschka
// Copyright (C) 2011 Zoltan Bojthe
// Copyright (C) 2015 Martin Becke
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <string.h>
#include <algorithm>    // min,max

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"

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
        CASE(TCP_E_STATUS);
        CASE(TCP_E_QUEUE_BYTES_LIMIT);
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

void TcpConnection::printSegmentBrief(Packet *packet, const Ptr<const TcpHeader>& tcpseg)
{
    EV_STATICCONTEXT;
    EV_INFO << "." << tcpseg->getSrcPort() << " > ";
    EV_INFO << "." << tcpseg->getDestPort() << ": ";

    if (tcpseg->getSynBit())
        EV_INFO << (tcpseg->getAckBit() ? "SYN+ACK " : "SYN ");

    if (tcpseg->getFinBit())
        EV_INFO << "FIN(+ACK) ";

    if (tcpseg->getRstBit())
        EV_INFO << (tcpseg->getAckBit() ? "RST+ACK " : "RST ");

    if (tcpseg->getPshBit())
        EV_INFO << "PSH ";

    auto payloadLength = packet->getByteLength() - B(tcpseg->getHeaderLength()).get();
    if (payloadLength > 0 || tcpseg->getSynBit()) {
        EV_INFO << "[" << tcpseg->getSequenceNo() << ".." << (tcpseg->getSequenceNo() + payloadLength) << ") ";
        EV_INFO << "(l=" << payloadLength << ") ";
    }

    if (tcpseg->getAckBit())
        EV_INFO << "ack " << tcpseg->getAckNo() << " ";

    EV_INFO << "win " << tcpseg->getWindow() << " ";

    if (tcpseg->getUrgBit())
        EV_INFO << "urg " << tcpseg->getUrgentPointer() << " ";

    if (tcpseg->getHeaderLength() > TCP_MIN_HEADER_LENGTH) {    // Header options present?
        EV_INFO << "options ";

        for (uint i = 0; i < tcpseg->getHeaderOptionArraySize(); i++) {
            const TcpOption *option = tcpseg->getHeaderOption(i);
            short kind = option->getKind();
            EV_INFO << optionName(kind) << " ";
        }
    }
    EV_INFO << "\n";
}

void TcpConnection::initClonedConnection(TcpConnection *listenerConn)
{
    Enter_Method_Silent();
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
    FSM_Goto(fsm, TCP_S_LISTEN);
}

TcpConnection *TcpConnection::cloneListeningConnection()
{
//    TcpConnection *conn = new TcpConnection();
//    conn->initConnection(tcpMain, socketId);
    auto moduleType = cModuleType::get("inet.transportlayer.tcp.TcpConnection");
    int newSocketId = getEnvir()->getUniqueNumber();
    char submoduleName[24];
    sprintf(submoduleName, "conn-%d", newSocketId);
    auto conn = check_and_cast<TcpConnection *>(moduleType->createScheduleInit(submoduleName, tcpMain));
    conn->initConnection(tcpMain, newSocketId);
    conn->initClonedConnection(this);
    // FSM_Goto(conn->fsm, TCP_S_LISTEN);
    //FSM_Goto(fsm, TCP_S_LISTEN);

    return conn;
}

void TcpConnection::sendToIP(Packet *packet, const Ptr<TcpHeader>& tcpseg)
{
    // record seq (only if we do send data) and ackno
    if (packet->getByteLength() > B(tcpseg->getChunkLength()).get())
        emit(sndNxtSignal, tcpseg->getSequenceNo());

    emit(sndAckSignal, tcpseg->getAckNo());

    // final touches on the segment before sending
    tcpseg->setSrcPort(localPort);
    tcpseg->setDestPort(remotePort);
    ASSERT(tcpseg->getHeaderLength() >= TCP_MIN_HEADER_LENGTH);
    ASSERT(tcpseg->getHeaderLength() <= TCP_MAX_HEADER_LENGTH);
    ASSERT(tcpseg->getChunkLength() == tcpseg->getHeaderLength());
    state->sentBytes = packet->getByteLength();    // resetting sentBytes to 0 if sending a segment without data (e.g. ACK)

    EV_INFO << "Sending: ";
    printSegmentBrief(packet, tcpseg);

    // TBD reuse next function for sending

    IL3AddressType *addressType = remoteAddr.getAddressType();
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());

    if (ttl != -1 && packet->findTag<HopLimitReq>() == nullptr)
        packet->addTag<HopLimitReq>()->setHopLimit(ttl);

    if (dscp != -1 && packet->findTag<DscpReq>() == nullptr)
        packet->addTag<DscpReq>()->setDifferentiatedServicesCodePoint(dscp);

    if (tos != -1 && packet->findTag<TosReq>() == nullptr)
        packet->addTag<TosReq>()->setTos(tos);

    auto addresses = packet->addTagIfAbsent<L3AddressReq>();
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
    packet->addTagIfAbsent<EcnReq>()->setExplicitCongestionNotification((state->ect && !state->sndAck && !state->rexmit) ? IP_ECN_ECT_1 : IP_ECN_NOT_ECT);

    tcpseg->setCrc(0);
    tcpseg->setCrcMode(tcpMain->crcMode);

    insertTransportProtocolHeader(packet, Protocol::tcp, tcpseg);

    tcpMain->sendFromConn(packet, "ipOut");
}

void TcpConnection::sendToIP(Packet *packet, const Ptr<TcpHeader>& tcpseg, L3Address src, L3Address dest)
{
    EV_STATICCONTEXT;
    EV_INFO << "Sending: ";
    printSegmentBrief(packet, tcpseg);

    IL3AddressType *addressType = dest.getAddressType();
    ASSERT(tcpseg->getChunkLength() == tcpseg->getHeaderLength());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());

    if (ttl != -1 && packet->findTag<HopLimitReq>() == nullptr)
        packet->addTag<HopLimitReq>()->setHopLimit(ttl);

    if (dscp != -1 && packet->findTag<DscpReq>() == nullptr)
        packet->addTag<DscpReq>()->setDifferentiatedServicesCodePoint(dscp);

    if (tos != -1 && packet->findTag<TosReq>() == nullptr)
        packet->addTag<TosReq>()->setTos(tos);

    auto addresses = packet->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(src);
    addresses->setDestAddress(dest);

    insertTransportProtocolHeader(packet, Protocol::tcp, tcpseg);

    tcpMain->sendFromConn(packet, "ipOut");
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
    indication->addTag<SocketInd>()->setSocketId(socketId);
    indication->setControlInfo(ind);
    sendToApp(indication);
}

void TcpConnection::sendToApp(cMessage *msg)
{
    tcpMain->sendFromConn(msg, "appOut");
}

void TcpConnection::sendAvailableDataToApp()
{
    if (receiveQueue->getAmountOfBufferedBytes()) {
        if (tcpMain->useDataNotification) {
            auto indication = new Indication("Data Notification", TCP_I_DATA_NOTIFICATION); // TBD currently we never send TCP_I_URGENT_DATA
            TcpCommand *cmd = new TcpCommand();
            indication->addTag<SocketInd>()->setSocketId(socketId);
            indication->setControlInfo(cmd);
            sendToApp(indication);
        } else {
            while (auto msg = receiveQueue->extractBytesUpTo(state->rcv_nxt)) {
                msg->setKind(TCP_I_DATA);    // TBD currently we never send TCP_I_URGENT_DATA
                msg->addTag<SocketInd>()->setSocketId(socketId);
                sendToApp(msg);
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

    if (!tcpAlgorithmClass || !tcpAlgorithmClass[0])
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
    state->dupthresh = tcpMain->par("dupthresh");
    long advertisedWindowPar = tcpMain->par("advertisedWindow");
    state->ws_support = tcpMain->par("windowScalingSupport");    // if set, this means that current host supports WS (RFC 1323)
    state->ws_manual_scale = tcpMain->par("windowScalingFactor"); // scaling factor (set manually) to help for Tcp validation
    state->ecnWillingness = tcpMain->par("ecnWillingness"); // if set, current host is willing to use ECN
    if (!state->ws_support && (advertisedWindowPar > TCP_MAX_WIN || advertisedWindowPar <= 0))
        throw cRuntimeError("Invalid advertisedWindow parameter: %ld", advertisedWindowPar);

    state->rcv_wnd = advertisedWindowPar;
    state->rcv_adv = advertisedWindowPar;

    if (state->ws_support && advertisedWindowPar > TCP_MAX_WIN) {
        state->rcv_wnd = TCP_MAX_WIN;    // we cannot to guarantee that the other end is also supporting the Window Scale (header option) (RFC 1322)
        state->rcv_adv = TCP_MAX_WIN;    // therefore TCP_MAX_WIN is used as initial value for rcv_wnd and rcv_adv
    }

    state->maxRcvBuffer = advertisedWindowPar;
    state->delayed_acks_enabled = tcpMain->par("delayedAcksEnabled");    // delayed ACK algorithm (RFC 1122) enabled/disabled
    state->nagle_enabled = tcpMain->par("nagleEnabled");    // Nagle's algorithm (RFC 896) enabled/disabled
    state->limited_transmit_enabled = tcpMain->par("limitedTransmitEnabled");    // Limited Transmit algorithm (RFC 3042) enabled/disabled
    state->increased_IW_enabled = tcpMain->par("increasedIWEnabled");    // Increased Initial Window (RFC 3390) enabled/disabled
    state->snd_mss = tcpMain->par("mss");    // Maximum Segment Size (RFC 793)
    state->ts_support = tcpMain->par("timestampSupport");    // if set, this means that current host supports TS (RFC 1323)
    state->sack_support = tcpMain->par("sackSupport");    // if set, this means that current host supports SACK (RFC 2018, 2883, 3517)

    if (state->sack_support) {
        std::string algorithmName1 = "TcpReno";
        std::string algorithmName2 = tcpMain->par("tcpAlgorithmClass");

        if (algorithmName1 != algorithmName2) {    // TODO add additional checks for new SACK supporting algorithms here once they are implemented
            EV_DEBUG << "If you want to use TCP SACK please set tcpAlgorithmClass to TcpReno\n";

            ASSERT(false);
        }
    }
}

void TcpConnection::selectInitialSeqNum()
{
    // set the initial send sequence number
    state->iss = (unsigned long)fmod(SIMTIME_DBL(simTime()) * 250000.0, 1.0 + (double)(unsigned)0xffffffffUL) & 0xffffffffUL;

    state->snd_una = state->snd_nxt = state->snd_max = state->iss;

    sendQueue->init(state->iss + 1);    // + 1 is for SYN
    rexmitQueue->init(state->iss + 1);    // + 1 is for SYN
}

bool TcpConnection::isSegmentAcceptable(Packet *packet, const Ptr<const TcpHeader>& tcpseg) const
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
    uint32 len = packet->getByteLength() - B(tcpseg->getHeaderLength()).get();
    uint32 seqNo = tcpseg->getSequenceNo();
    uint32 ackNo = tcpseg->getAckNo();
    uint32 rcvWndEnd = state->rcv_nxt + state->rcv_wnd;
    bool ret;

    if (len == 0) {
        if (state->rcv_wnd == 0)
            ret = (seqNo == state->rcv_nxt);
        else // rcv_wnd > 0
//            ret = seqLE(state->rcv_nxt, seqNo) && seqLess(seqNo, rcvWndEnd);
            ret = seqLE(state->rcv_nxt, seqNo) && seqLE(seqNo, rcvWndEnd); // Accept an ACK on end of window
    }
    else {    // len > 0
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
    const auto& tcpseg = makeShared<TcpHeader>();
    tcpseg->setSequenceNo(state->iss);
    tcpseg->setSynBit(true);
    updateRcvWnd();
    tcpseg->setWindow(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss + 1;

    // ECN
    if (state->ecnWillingness) {
        tcpseg->setEceBit(true);
        tcpseg->setCwrBit(true);
        state->ecnSynSent = true;
        EV << "ECN-setup SYN packet sent\n";
    } else {
        // rfc 3168 page 16:
        // A host that is not willing to use ECN on a TCP connection SHOULD
        // clear both the ECE and CWR flags in all non-ECN-setup SYN and/or
        // SYN-ACK packets that it sends to indicate this unwillingness.
        tcpseg->setEceBit(false);
        tcpseg->setCwrBit(false);
        state->ecnSynSent = false;
        // EV << "non-ECN-setup SYN packet sent\n";
    }

    // write header options
    writeHeaderOptions(tcpseg);
    Packet *fp = new Packet("SYN");

    // send it
    sendToIP(fp, tcpseg);
}

void TcpConnection::sendSynAck()
{
    // create segment
    const auto& tcpseg = makeShared<TcpHeader>();
    tcpseg->setSequenceNo(state->iss);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setSynBit(true);
    tcpseg->setAckBit(true);
    updateRcvWnd();
    tcpseg->setWindow(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss + 1;

    //ECN
    if (state->ecnWillingness) {
        tcpseg->setEceBit(true);
        tcpseg->setCwrBit(false);
        EV << "ECN-setup SYN-ACK packet sent\n";
    } else {
        tcpseg->setEceBit(false);
        tcpseg->setCwrBit(false);
        if (state->endPointIsWillingECN)
            EV << "non-ECN-setup SYN-ACK packet sent\n";
    }
    if (state->ecnWillingness && state->endPointIsWillingECN) {
        state->ect = true;
        EV << "both end-points are willing to use ECN... ECN is enabled\n";
    } else { //TODO: not sure if we have to.
             // rfc-3168, page 16:
             // A host that is not willing to use ECN on a TCP connection SHOULD
             // clear both the ECE and CWR flags in all non-ECN-setup SYN and/or
             // SYN-ACK packets that it sends to indicate this unwillingness.
        state->ect = false;
        if (state->endPointIsWillingECN)
            EV << "ECN is disabled\n";
    }

    // write header options
    writeHeaderOptions(tcpseg);

    Packet *fp = new Packet("SYN+ACK");

    // send it
    sendToIP(fp, tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}

void TcpConnection::sendRst(uint32 seqNo)
{
    sendRst(seqNo, localAddr, remoteAddr, localPort, remotePort);
}

void TcpConnection::sendRst(uint32 seq, L3Address src, L3Address dest, int srcPort, int destPort)
{
    const auto& tcpseg = makeShared<TcpHeader>();

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setSequenceNo(seq);
    tcpseg->setCrcMode(tcpMain->crcMode);
    tcpseg->setCrc(0);

    Packet *fp = new Packet("RST");

    // send it
    sendToIP(fp, tcpseg, src, dest);
}

void TcpConnection::sendRstAck(uint32 seq, uint32 ack, L3Address src, L3Address dest, int srcPort, int destPort)
{
    const auto& tcpseg = makeShared<TcpHeader>();

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setAckBit(true);
    tcpseg->setSequenceNo(seq);
    tcpseg->setAckNo(ack);
    tcpseg->setCrcMode(tcpMain->crcMode);
    tcpseg->setCrc(0);

    Packet *fp = new Packet("RST+ACK");

    // send it
    sendToIP(fp, tcpseg, src, dest);

    // notify
    if (tcpAlgorithm)
        tcpAlgorithm->ackSent();
}

void TcpConnection::sendAck()
{
    const auto& tcpseg = makeShared<TcpHeader>();

    tcpseg->setAckBit(true);
    tcpseg->setSequenceNo(state->snd_nxt);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setWindow(updateRcvWnd());

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

    TcpStateVariables* state = getState();
    if (state && state->ect) {
        if (tcpAlgorithm->shouldMarkAck()) {
            tcpseg->setEceBit(true);
            EV_INFO << "In ecnEcho state... send ACK with ECE bit set\n";
        }
    }

    // write header options
    writeHeaderOptions(tcpseg);
    Packet *fp = new Packet("TcpAck");

    // rfc-3168 page 20: pure ack packets must be sent with not-ECT codepoint
    state->sndAck = true;

    // send it
    sendToIP(fp, tcpseg);

    state->sndAck = false;

    // notify
    tcpAlgorithm->ackSent();
}

void TcpConnection::sendFin()
{
    const auto& tcpseg = makeShared<TcpHeader>();

    // Note: ACK bit *must* be set for both FIN and FIN+ACK. What makes
    // the difference for FIN+ACK is that its ackNo acks the remote Tcp's FIN.
    tcpseg->setFinBit(true);
    tcpseg->setAckBit(true);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setSequenceNo(state->snd_nxt);
    tcpseg->setWindow(updateRcvWnd());
    Packet *fp = new Packet("FIN");

    // send it
    sendToIP(fp, tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}

void TcpConnection::sendSegment(uint32 bytes)
{
    //FIXME check it: where is the right place for the next code (sacked/rexmitted)
    if (state->sack_enabled && state->afterRto) {
        // check rexmitQ and try to forward snd_nxt before sending new data
        uint32 forward = rexmitQueue->checkRexmitQueueForSackedOrRexmittedSegments(state->snd_nxt);

        if (forward > 0) {
            EV_INFO << "sendSegment(" << bytes << ") forwarded " << forward << " bytes of snd_nxt from " << state->snd_nxt;
            state->snd_nxt += forward;
            EV_INFO << " to " << state->snd_nxt << endl;
            EV_DETAIL << rexmitQueue->detailedInfo();
        }
    }

    ulong buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (bytes > buffered) // last segment?
        bytes = buffered;

    // if header options will be added, this could reduce the number of data bytes allowed for this segment,
    // because following condition must to be respected:
    //     bytes + options_len <= snd_mss
    const auto& tcpseg_temp = makeShared<TcpHeader>();
    tcpseg_temp->setAckBit(true);    // needed for TS option, otherwise TSecr will be set to 0
    writeHeaderOptions(tcpseg_temp);
    uint options_len = B(tcpseg_temp->getHeaderLength() - TCP_MIN_HEADER_LENGTH).get();

    ASSERT(options_len < state->snd_mss);

    if (bytes + options_len > state->snd_mss)
        bytes = state->snd_mss - options_len;

    state->sentBytes = bytes;

    // send one segment of 'bytes' bytes from snd_nxt, and advance snd_nxt
    Packet *packet = sendQueue->createSegmentWithBytes(state->snd_nxt, bytes);
    const auto& tcpseg = makeShared<TcpHeader>();
    tcpseg->setSequenceNo(state->snd_nxt);
    ASSERT(tcpseg != nullptr);

    //Remember old_snd_next to store in SACK rexmit queue.
    uint32 old_snd_nxt = state->snd_nxt;

    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setAckBit(true);
    tcpseg->setWindow(updateRcvWnd());

    //ECN
    if (state->ect && state->sndCwr) {
        tcpseg->setCwrBit(true);
        EV_INFO << "set CWR bit\n";
        state->sndCwr = false;
    }

    // TBD when to set PSH bit?
    // TBD set URG bit if needed
    ASSERT(bytes == packet->getByteLength());

    state->snd_nxt += bytes;

    // check if afterRto bit can be reset
    if (state->afterRto && seqGE(state->snd_nxt, state->snd_max))
        state->afterRto = false;

    if (state->send_fin && state->snd_nxt == state->snd_fin_seq) {
        EV_DETAIL << "Setting FIN on segment\n";
        tcpseg->setFinBit(true);
        state->snd_nxt = state->snd_fin_seq + 1;
    }

    // if sack_enabled copy region of tcpseg to rexmitQueue
    if (state->sack_enabled)
        rexmitQueue->enqueueSentData(old_snd_nxt, state->snd_nxt);

    // add header options and update header length (from tcpseg_temp)
    for (uint i = 0; i < tcpseg_temp->getHeaderOptionArraySize(); i++)
        tcpseg->insertHeaderOption(tcpseg_temp->getHeaderOption(i)->dup());
    tcpseg->setHeaderLength(TCP_MIN_HEADER_LENGTH + tcpseg->getHeaderOptionArrayLength());
    tcpseg->setChunkLength(B(tcpseg->getHeaderLength()));

    ASSERT(tcpseg->getHeaderLength() == tcpseg_temp->getHeaderLength());

    // send it
    sendToIP(packet, tcpseg);

    // let application fill queue again, if there is space
    const uint32 alreadyQueued = sendQueue->getBytesAvailable(sendQueue->getBufferStartSeq());
    const uint32 abated = (state->sendQueueLimit > alreadyQueued) ? state->sendQueueLimit - alreadyQueued : 0;
    if ((state->sendQueueLimit > 0) && !state->queueUpdate && (abated >= state->snd_mss)) {    // request more data if space >= 1 MSS
        // Tell upper layer readiness to accept more data
        sendIndicationToApp(TCP_I_SEND_MSG, abated);
        state->queueUpdate = true;
    }
}

bool TcpConnection::sendData(bool fullSegmentsOnly, uint32 congestionWindow)
{
    // we'll start sending from snd_max, if not after RTO
    if (!state->afterRto)
        state->snd_nxt = state->snd_max;

    uint32 old_highRxt = 0;

    if (state->sack_enabled)
        old_highRxt = rexmitQueue->getHighestRexmittedSeqNum();

    // check how many bytes we have
    ulong buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (buffered == 0)
        return false;

    // maxWindow is minimum of snd_wnd and congestionWindow (snd_cwnd)
    ulong maxWindow = std::min(state->snd_wnd, congestionWindow);

    // effectiveWindow: number of bytes we're allowed to send now
    long effectiveWin = maxWindow - (state->snd_nxt - state->snd_una);

    if (effectiveWin <= 0) {
        EV_WARN << "Effective window is zero (advertised window " << state->snd_wnd
                << ", congestion window " << congestionWindow << "), cannot send.\n";
        return false;
    }

    ulong bytesToSend = effectiveWin;

    if (bytesToSend > buffered)
        bytesToSend = buffered;

    // make a temporary tcp header for detecting tcp options length (copied from 'TcpConnection::sendSegment(uint32 bytes)' )
    const auto& tmpTcpHeader = makeShared<TcpHeader>();
    tmpTcpHeader->setAckBit(true);    // needed for TS option, otherwise TSecr will be set to 0
    writeHeaderOptions(tmpTcpHeader);
    uint options_len = B(tmpTcpHeader->getHeaderLength() - TCP_MIN_HEADER_LENGTH).get();
    ASSERT(options_len < state->snd_mss);
    uint32 effectiveMaxBytesSend = state->snd_mss - options_len;

    // last segment could be less than state->snd_mss (or less than snd_mss - TCP_OPTION_TS_SIZE if using TS option)
    if (fullSegmentsOnly && (bytesToSend < (effectiveMaxBytesSend))) {
        EV_WARN << "Cannot send, not enough data for a full segment (SMSS=" << state->snd_mss
                << ", effectiveWindow=" << effectiveWin << ", bytesToSend=" << bytesToSend << ", in buffer " << buffered << ")\n";
        return false;
    }

    // start sending 'bytesToSend' bytes
    EV_INFO << "Will send " << bytesToSend << " bytes (effectiveWindow " << effectiveWin
            << ", in buffer " << buffered << " bytes)\n";

    uint32 old_snd_nxt = state->snd_nxt;

    ASSERT(bytesToSend > 0);

      // send < MSS segments only if it's the only segment we can send now
      // Note: if (bytesToSend == 1010 && MSS == 1012 && ts_enabled == true) => we may send
      // 2 segments (1000 payload + 12 optionsHeader and 10 payload + 12 optionsHeader)
      // FIXME this should probably obey Nagle's alg -- to be checked
    if (bytesToSend <= state->snd_mss) {
        sendSegment(bytesToSend);
        ASSERT(bytesToSend >= state->sentBytes);
        bytesToSend -= state->sentBytes;
    }
    else {    // send whole segments only (nagle_enabled)
        while (bytesToSend >= effectiveMaxBytesSend) {
            sendSegment(state->snd_mss);
            ASSERT(bytesToSend >= state->sentBytes);
            bytesToSend -= state->sentBytes;
        }
    }

    // check how many bytes we have - last segment could be less than state->snd_mss
    buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (bytesToSend == buffered && buffered != 0) // last segment?
        sendSegment(bytesToSend);
    else if (bytesToSend > 0)
        EV_DETAIL << bytesToSend << " bytes of space left in effectiveWindow\n";

    // remember highest seq sent (snd_nxt may be set back on retransmission,
    // but we'll need snd_max to check validity of ACKs -- they must ack
    // something we really sent)
    if (seqGreater(state->snd_nxt, state->snd_max))
        state->snd_max = state->snd_nxt;

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

    uint32 old_snd_nxt = state->snd_nxt;

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

    uint32 old_snd_nxt = state->snd_nxt;

    // retransmit one segment at snd_una, and set snd_nxt accordingly (if not called at RTO)
    state->snd_nxt = state->snd_una;

    // When FIN sent the snd_max - snd_nxt larger than bytes available in queue
    ulong bytes = std::min((ulong)std::min(state->snd_mss, state->snd_max - state->snd_nxt),
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

    uint32 bytesToSend = state->snd_max - state->snd_nxt;

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

    // TBD - avoid to send more than allowed - check cwnd and rwnd before retransmitting data!
    while (bytesToSend > 0) {
        uint32 bytes = std::min(bytesToSend, state->snd_mss);
        bytes = std::min(bytes, (uint32)(sendQueue->getBytesAvailable(state->snd_nxt)));
        sendSegment(bytes);

        // Do not send packets after the FIN.
        // fixes bug that occurs in examples/inet/bulktransfer at event #64043  T=13.861159213744
        if (state->send_fin && state->snd_nxt == state->snd_fin_seq + 1)
            break;

        ASSERT(bytesToSend >= state->sentBytes);
        bytesToSend -= state->sentBytes;
    }
    tcpAlgorithm->segmentRetransmitted(state->snd_una, state->snd_nxt);

    if (state && state->ect)
        state->rexmit = false;
}

void TcpConnection::readHeaderOptions(const Ptr<const TcpHeader>& tcpseg)
{
    EV_INFO << "Tcp Header Option(s) received:\n";

    for (uint i = 0; i < tcpseg->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpseg->getHeaderOption(i);
        short kind = option->getKind();
        short length = option->getLength();
        bool ok = true;

        EV_DETAIL << "Option type " << kind << " (" << optionName(kind) << "), length " << length << "\n";

        switch (kind) {
            case TCPOPTION_END_OF_OPTION_LIST:    // EOL=0
            case TCPOPTION_NO_OPERATION:    // NOP=1
                if (length != 1) {
                    EV_ERROR << "ERROR: option length incorrect\n";
                    ok = false;
                }
                break;

            case TCPOPTION_MAXIMUM_SEGMENT_SIZE:    // MSS=2
                ok = processMSSOption(tcpseg, *check_and_cast<const TcpOptionMaxSegmentSize *>(option));
                break;

            case TCPOPTION_WINDOW_SCALE:    // WS=3
                ok = processWSOption(tcpseg, *check_and_cast<const TcpOptionWindowScale *>(option));
                break;

            case TCPOPTION_SACK_PERMITTED:    // SACK_PERMITTED=4
                ok = processSACKPermittedOption(tcpseg, *check_and_cast<const TcpOptionSackPermitted *>(option));
                break;

            case TCPOPTION_SACK:    // SACK=5
                ok = processSACKOption(tcpseg, *check_and_cast<const TcpOptionSack *>(option));
                break;

            case TCPOPTION_TIMESTAMP:    // TS=8
                ok = processTSOption(tcpseg, *check_and_cast<const TcpOptionTimestamp *>(option));
                break;

            // TODO add new TCPOptions here once they are implemented
            // TODO delegate to TcpAlgorithm as well -- it may want to recognized additional options

            default:
                EV_ERROR << "ERROR: Unsupported Tcp option kind " << kind << "\n";
                break;
        }
        (void)ok;    // unused
    }
}

bool TcpConnection::processMSSOption(const Ptr<const TcpHeader>& tcpseg, const TcpOptionMaxSegmentSize& option)
{
    if (option.getLength() != 4) {
        EV_ERROR << "ERROR: MSS option length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: Tcp Header Option MSS received, but in unexpected state\n";
        return false;
    }

    // RFC 2581, page 1:
    // "The SMSS is the size of the largest segment that the sender can transmit.
    // This value can be based on the maximum transmission unit of the network,
    // the path MTU discovery [MD90] algorithm, RMSS (see next item), or other
    // factors.  The size does not include the TCP/IP headers and options."
    //
    // "The RMSS is the size of the largest segment the receiver is willing to accept.
    // This is the value specified in the MSS option sent by the receiver during
    // connection startup.  Or, if the MSS option is not used, 536 bytes [Bra89].
    // The size does not include the TCP/IP headers and options."
    //
    //
    // The value of snd_mss (SMSS) is set to the minimum of snd_mss (local parameter) and
    // the value specified in the MSS option received during connection startup.
    state->snd_mss = std::min(state->snd_mss, (uint32)option.getMaxSegmentSize());

    if (state->snd_mss == 0)
        state->snd_mss = 536;

    EV_INFO << "Tcp Header Option MSS(=" << option.getMaxSegmentSize() << ") received, SMSS is set to " << state->snd_mss << "\n";
    return true;
}

bool TcpConnection::processWSOption(const Ptr<const TcpHeader>& tcpseg, const TcpOptionWindowScale& option)
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

    if (state->snd_wnd_scale > 14) {    // RFC 1323, page 11: "the shift count must be limited to 14"
        EV_ERROR << "ERROR: Tcp Header Option WS received but shift count value is exceeding 14\n";
        state->snd_wnd_scale = 14;
    }

    return true;
}

bool TcpConnection::processTSOption(const Ptr<const TcpHeader>& tcpseg, const TcpOptionTimestamp& option)
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
            if ((simTime() - state->time_last_data_sent) > PAWS_IDLE_TIME_THRESH) {    // PAWS_IDLE_TIME_THRESH = 24 days
                EV_DETAIL << "PAWS: Segment is not acceptable, TSval=" << option.getSenderTimestamp() << " in " << stateName(fsm.getState()) << " state received: dropping segment\n";
                return false;
            }
        }
        else if (seqLE(tcpseg->getSequenceNo(), state->last_ack_sent)) {    // Note: test is modified according to the latest proposal of the tcplw@cray.com list (Braden 1993/04/26)
            state->ts_recent = option.getSenderTimestamp();
            EV_DETAIL << "Updating ts_recent from segment: new ts_recent=" << state->ts_recent << "\n";
        }
    }

    return true;
}

bool TcpConnection::processSACKPermittedOption(const Ptr<const TcpHeader>& tcpseg, const TcpOptionSackPermitted& option)
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

TcpHeader TcpConnection::writeHeaderOptions(const Ptr<TcpHeader>& tcpseg)
{
    // SYN flag set and connetion in INIT or LISTEN state (or after synRexmit timeout)
    if (tcpseg->getSynBit() && (fsm.getState() == TCP_S_INIT || fsm.getState() == TCP_S_LISTEN
                                || ((fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD)
                                    && state->syn_rexmit_count > 0)))
    {
        // MSS header option
        if (state->snd_mss > 0) {
            TcpOptionMaxSegmentSize *option = new TcpOptionMaxSegmentSize();
            option->setMaxSegmentSize(state->snd_mss);
            tcpseg->insertHeaderOption(option);
            EV_INFO << "Tcp Header Option MSS(=" << state->snd_mss << ") sent\n";
        }

        // WS header option
        if (state->ws_support && (state->rcv_ws || (fsm.getState() == TCP_S_INIT
                                                    || (fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count > 0))))
        {
            // 1 padding byte
            tcpseg->insertHeaderOption(new TcpOptionNop());    // NOP

            // Update WS variables
            if (state->ws_manual_scale > -1) {
                state->rcv_wnd_scale = state->ws_manual_scale;
            } else {
                ulong scaled_rcv_wnd = receiveQueue->getFirstSeqNo() + state->maxRcvBuffer - state->rcv_nxt;
                state->rcv_wnd_scale = 0;

                while (scaled_rcv_wnd > TCP_MAX_WIN && state->rcv_wnd_scale < 14) {    // RFC 1323, page 11: "the shift count must be limited to 14"
                    scaled_rcv_wnd = scaled_rcv_wnd >> 1;
                    state->rcv_wnd_scale++;
                }
            }

            TcpOptionWindowScale *option = new TcpOptionWindowScale();
            option->setWindowScale(state->rcv_wnd_scale);    // rcv_wnd_scale is also set in scaleRcvWnd()
            state->snd_ws = true;
            state->ws_enabled = state->ws_support && state->snd_ws && state->rcv_ws;
            EV_INFO << "Tcp Header Option WS(=" << option->getWindowScale() << ") sent, WS (ws_enabled) is set to " << state->ws_enabled << "\n";
            tcpseg->insertHeaderOption(option);
        }

        // SACK_PERMITTED header option
        if (state->sack_support && (state->rcv_sack_perm || (fsm.getState() == TCP_S_INIT
                                                             || (fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count > 0))))
        {
            if (!state->ts_support) {    // if TS is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpseg->insertHeaderOption(new TcpOptionNop());    // NOP
                tcpseg->insertHeaderOption(new TcpOptionNop());    // NOP
            }

            tcpseg->insertHeaderOption(new TcpOptionSackPermitted());

            // Update SACK variables
            state->snd_sack_perm = true;
            state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
            EV_INFO << "Tcp Header Option SACK_PERMITTED sent, SACK (sack_enabled) is set to " << state->sack_enabled << "\n";
        }

        // TS header option
        if (state->ts_support && (state->rcv_initial_ts || (fsm.getState() == TCP_S_INIT
                                                            || (fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count > 0))))
        {
            if (!state->sack_support) {    // if SACK is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpseg->insertHeaderOption(new TcpOptionNop());    // NOP
                tcpseg->insertHeaderOption(new TcpOptionNop());    // NOP
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
            option->setEchoedTimestamp(tcpseg->getAckBit() ? state->ts_recent : 0);

            state->snd_initial_ts = true;
            state->ts_enabled = state->ts_support && state->snd_initial_ts && state->rcv_initial_ts;
            EV_INFO << "Tcp Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent, TS (ts_enabled) is set to " << state->ts_enabled << "\n";
            tcpseg->insertHeaderOption(option);
        }

        // TODO add new TCPOptions here once they are implemented
    }
    else if (fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD
             || fsm.getState() == TCP_S_ESTABLISHED || fsm.getState() == TCP_S_FIN_WAIT_1
             || fsm.getState() == TCP_S_FIN_WAIT_2 || fsm.getState() == TCP_S_CLOSE_WAIT)    // connetion is not in INIT or LISTEN state
    {
        // TS header option
        if (state->ts_enabled) {    // Is TS enabled?
            if (!(state->sack_enabled && (state->snd_sack || state->snd_dsack))) {    // if SACK is enabled and SACKs need to be added, do not add NOPs to this segment
                // 2 padding bytes
                tcpseg->insertHeaderOption(new TcpOptionNop());    // NOP
                tcpseg->insertHeaderOption(new TcpOptionNop());    // NOP
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
            option->setEchoedTimestamp(tcpseg->getAckBit() ? state->ts_recent : 0);

            EV_INFO << "Tcp Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent\n";
            tcpseg->insertHeaderOption(option);
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
            addSacks(tcpseg);
        }

        // TODO add new TCPOptions here once they are implemented
        // TODO delegate to TcpAlgorithm as well -- it may want to append additional options
    }

    if (tcpseg->getHeaderOptionArraySize() != 0) {
        B options_len = tcpseg->getHeaderOptionArrayLength();

        if (options_len <= TCP_OPTIONS_MAX_SIZE) { // Options length allowed? - maximum: 40 Bytes
            tcpseg->setHeaderLength(TCP_MIN_HEADER_LENGTH + options_len);
            tcpseg->setChunkLength(B(TCP_MIN_HEADER_LENGTH + options_len));
        }
        else {
            tcpseg->dropHeaderOptions();    // drop all options
            tcpseg->setHeaderLength(TCP_MIN_HEADER_LENGTH);
            tcpseg->setChunkLength(TCP_MIN_HEADER_LENGTH);
            EV_ERROR << "ERROR: Options length exceeded! Segment will be sent without options" << "\n";
        }
    }

    return *tcpseg;
}

uint32 TcpConnection::getTSval(const Ptr<const TcpHeader>& tcpseg) const
{
    for (uint i = 0; i < tcpseg->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpseg->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_TIMESTAMP)
            return check_and_cast<const TcpOptionTimestamp *>(option)->getSenderTimestamp();
    }

    return 0;
}

uint32 TcpConnection::getTSecr(const Ptr<const TcpHeader>& tcpseg) const
{
    for (uint i = 0; i < tcpseg->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpseg->getHeaderOption(i);
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

unsigned short TcpConnection::updateRcvWnd()
{
    uint32 win = 0;

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
    uint32 scaled_rcv_wnd = state->rcv_wnd;
    if (state->ws_enabled && state->rcv_wnd_scale) {
        ASSERT(state->rcv_wnd_scale <= 14);   // RFC 1323, page 11: "the shift count must be limited to 14"
        scaled_rcv_wnd = scaled_rcv_wnd >> state->rcv_wnd_scale;
    }

    ASSERT(scaled_rcv_wnd == (unsigned short)scaled_rcv_wnd);

    return (unsigned short)scaled_rcv_wnd;
}

void TcpConnection::updateWndInfo(const Ptr<const TcpHeader>& tcpseg, bool doAlways)
{
    uint32 true_window = tcpseg->getWindow();
    // RFC 1323, page 10:
    // "The window field (SEG.WND) in the header of every incoming
    // segment, with the exception of SYN segments, is left-shifted
    // by Snd.Wind.Scale bits before updating SND.WND:
    //    SND.WND = SEG.WND << Snd.Wind.Scale"
    if (state->ws_enabled && !tcpseg->getSynBit())
        true_window = tcpseg->getWindow() << state->snd_wnd_scale;

    // Following lines are based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 982]:
    if (doAlways || (tcpseg->getAckBit()
                     && (seqLess(state->snd_wl1, tcpseg->getSequenceNo()) ||
                         (state->snd_wl1 == tcpseg->getSequenceNo() && seqLE(state->snd_wl2, tcpseg->getAckNo())) ||
                         (state->snd_wl2 == tcpseg->getAckNo() && true_window > state->snd_wnd))))
    {
        // send window should be updated
        state->snd_wnd = true_window;
        EV_INFO << "Updating send window from segment: new wnd=" << state->snd_wnd << "\n";
        state->snd_wl1 = tcpseg->getSequenceNo();
        state->snd_wl2 = tcpseg->getAckNo();

        emit(sndWndSignal, state->snd_wnd);
    }
}

void TcpConnection::sendOneNewSegment(bool fullSegmentsOnly, uint32 congestionWindow)
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
        ulong buffered = sendQueue->getBytesAvailable(state->snd_max);

        if (buffered >= state->snd_mss || (!fullSegmentsOnly && buffered > 0)) {
            ulong outstandingData = state->snd_max - state->snd_una;

            // check conditions from RFC 3042
            if (outstandingData + state->snd_mss <= state->snd_wnd &&
                outstandingData + state->snd_mss <= congestionWindow + 2 * state->snd_mss)
            {
                // RFC 3042, page 3: "(...)the sender can only send two segments beyond the congestion window (cwnd)."
                uint32 effectiveWin = std::min(state->snd_wnd, congestionWindow) - outstandingData + 2 * state->snd_mss;

                // bytes: number of bytes we're allowed to send now
                uint32 bytes = std::min(effectiveWin, state->snd_mss);

                if (bytes >= state->snd_mss || (!fullSegmentsOnly && bytes > 0)) {
                    uint32 old_snd_nxt = state->snd_nxt;
                    // we'll start sending from snd_max
                    state->snd_nxt = state->snd_max;

                    EV_DETAIL << "Limited Transmit algorithm enabled. Sending one new segment.\n";
                    sendSegment(bytes);

                    if (seqGreater(state->snd_nxt, state->snd_max))
                        state->snd_max = state->snd_nxt;

                    emit(unackedSignal, state->snd_max - state->snd_una);

                    // reset snd_nxt if needed
                    if (state->afterRto)
                        state->snd_nxt = old_snd_nxt + state->sentBytes;

                    // notify
                    tcpAlgorithm->ackSent();
                    tcpAlgorithm->dataSent(old_snd_nxt);
                }
            }
        }
    }
}

uint32 TcpConnection::convertSimtimeToTS(simtime_t simtime)
{
    ASSERT(SimTime::getScaleExp() <= -3);

    uint32 timestamp = (uint32)(simtime.inUnit(SIMTIME_MS));
    return timestamp;
}

simtime_t TcpConnection::convertTSToSimtime(uint32 timestamp)
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

