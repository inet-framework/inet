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

#include "inet/transportlayer/tcp/TCP.h"
#include "inet/transportlayer/tcp/TCPConnection.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/transportlayer/tcp/TCPSendQueue.h"
#include "inet/transportlayer/tcp/TCPSACKRexmitQueue.h"
#include "inet/transportlayer/tcp/TCPReceiveQueue.h"
#include "inet/transportlayer/tcp/TCPAlgorithm.h"
#include "inet/common/INETUtils.h"

namespace inet {

namespace tcp {

//
// helper functions
//

const char *TCPConnection::stateName(int state)
{
#define CASE(x)    case x: \
        s = #x + 6; break
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

const char *TCPConnection::eventName(int event)
{
#define CASE(x)    case x: \
        s = #x + 6; break
    const char *s = "unknown";
    switch (event) {
        CASE(TCP_E_IGNORE);
        CASE(TCP_E_OPEN_ACTIVE);
        CASE(TCP_E_OPEN_PASSIVE);
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

const char *TCPConnection::indicationName(int code)
{
#define CASE(x)    case x: \
        s = #x + 6; break
    const char *s = "unknown";
    switch (code) {
        CASE(TCP_I_DATA);
        CASE(TCP_I_URGENT_DATA);
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

const char *TCPConnection::optionName(int option)
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

void TCPConnection::printConnBrief() const
{
    EV_DETAIL << "Connection "
              << localAddr << ":" << localPort << " to " << remoteAddr << ":" << remotePort
              << "  on app[" << appGateIndex << "], connId=" << connId
              << "  in " << stateName(fsm.getState())
              << "\n";
}

void TCPConnection::printSegmentBrief(TCPSegment *tcpseg)
{
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

    if (tcpseg->getPayloadLength() > 0 || tcpseg->getSynBit()) {
        EV_INFO << "[" << tcpseg->getSequenceNo() << ".." << (tcpseg->getSequenceNo() + tcpseg->getPayloadLength()) << ") ";
        EV_INFO << "(l=" << tcpseg->getPayloadLength() << ") ";
    }

    if (tcpseg->getAckBit())
        EV_INFO << "ack " << tcpseg->getAckNo() << " ";

    EV_INFO << "win " << tcpseg->getWindow() << " ";

    if (tcpseg->getUrgBit())
        EV_INFO << "urg " << tcpseg->getUrgentPointer() << " ";

    if (tcpseg->getHeaderLength() > TCP_HEADER_OCTETS) {    // Header options present? TCP_HEADER_OCTETS = 20
        EV_INFO << "options ";

        for (uint i = 0; i < tcpseg->getHeaderOptionArraySize(); i++) {
            const TCPOption *option = tcpseg->getHeaderOption(i);
            short kind = option->getKind();
            EV_INFO << optionName(kind) << " ";
        }
    }
    EV_INFO << "\n";
}

TCPConnection *TCPConnection::cloneListeningConnection()
{
    TCPConnection *conn = new TCPConnection(tcpMain, appGateIndex, connId);

    conn->transferMode = transferMode;
    // following code to be kept consistent with initConnection()
    const char *sendQueueClass = sendQueue->getClassName();
    conn->sendQueue = check_and_cast<TCPSendQueue *>(inet::utils::createOne(sendQueueClass));
    conn->sendQueue->setConnection(conn);

    const char *receiveQueueClass = receiveQueue->getClassName();
    conn->receiveQueue = check_and_cast<TCPReceiveQueue *>(inet::utils::createOne(receiveQueueClass));
    conn->receiveQueue->setConnection(conn);

    // create SACK retransmit queue
    conn->rexmitQueue = new TCPSACKRexmitQueue();
    conn->rexmitQueue->setConnection(this);

    const char *tcpAlgorithmClass = tcpAlgorithm->getClassName();
    conn->tcpAlgorithm = check_and_cast<TCPAlgorithm *>(inet::utils::createOne(tcpAlgorithmClass));
    conn->tcpAlgorithm->setConnection(conn);

    conn->state = conn->tcpAlgorithm->getStateVariables();
    configureStateVariables();
    conn->tcpAlgorithm->initialize();

    // put it into LISTEN, with our localAddr/localPort
    conn->state->active = false;
    conn->state->fork = true;
    conn->localAddr = localAddr;
    conn->localPort = localPort;
    FSM_Goto(conn->fsm, TCP_S_LISTEN);

    return conn;
}

void TCPConnection::sendToIP(TCPSegment *tcpseg)
{
    // record seq (only if we do send data) and ackno
    if (sndNxtVector && tcpseg->getPayloadLength() != 0)
        sndNxtVector->record(tcpseg->getSequenceNo());

    if (sndAckVector)
        sndAckVector->record(tcpseg->getAckNo());

    // final touches on the segment before sending
    tcpseg->setSrcPort(localPort);
    tcpseg->setDestPort(remotePort);
    ASSERT(tcpseg->getHeaderLength() >= TCP_HEADER_OCTETS);    // TCP_HEADER_OCTETS = 20 (without options)
    ASSERT(tcpseg->getHeaderLength() <= TCP_MAX_HEADER_OCTETS);    // TCP_MAX_HEADER_OCTETS = 60
    tcpseg->setByteLength(tcpseg->getHeaderLength() + tcpseg->getPayloadLength());
    state->sentBytes = tcpseg->getPayloadLength();    // resetting sentBytes to 0 if sending a segment without data (e.g. ACK)

    EV_INFO << "Sending: ";
    printSegmentBrief(tcpseg);

    // TBD reuse next function for sending

    IL3AddressType *addressType = remoteAddr.getAddressType();
    INetworkProtocolControlInfo *controlInfo = addressType->createNetworkProtocolControlInfo();
    controlInfo->setTransportProtocol(IP_PROT_TCP);
    controlInfo->setSourceAddress(localAddr);
    controlInfo->setDestinationAddress(remoteAddr);
    tcpseg->setControlInfo(check_and_cast<cObject *>(controlInfo));
    tcpMain->send(tcpseg, "ipOut");
}

void TCPConnection::sendToIP(TCPSegment *tcpseg, L3Address src, L3Address dest)
{
    EV_INFO << "Sending: ";
    printSegmentBrief(tcpseg);

    IL3AddressType *addressType = dest.getAddressType();
    INetworkProtocolControlInfo *controlInfo = addressType->createNetworkProtocolControlInfo();
    controlInfo->setTransportProtocol(IP_PROT_TCP);
    controlInfo->setSourceAddress(src);
    controlInfo->setDestinationAddress(dest);
    tcpseg->setControlInfo(check_and_cast<cObject *>(controlInfo));
    tcpseg->setByteLength(tcpseg->getHeaderLength() + tcpseg->getPayloadLength());
    check_and_cast<TCP *>(getSimulation()->getContextModule())->send(tcpseg, "ipOut");
}

TCPSegment *TCPConnection::createTCPSegment(const char *name)
{
    return new TCPSegment(name);
}

void TCPConnection::signalConnectionTimeout()
{
    sendIndicationToApp(TCP_I_TIMED_OUT);
}

void TCPConnection::sendIndicationToApp(int code, const int id)
{
    EV_INFO << "Notifying app: " << indicationName(code) << "\n";
    cMessage *msg = new cMessage(indicationName(code));
    msg->setKind(code);
    TCPCommand *ind = new TCPCommand();
    ind->setConnId(connId);
    ind->setUserId(id);
    msg->setControlInfo(ind);
    sendToApp(msg);
}

void TCPConnection::sendEstabIndicationToApp()
{
    EV_INFO << "Notifying app: " << indicationName(TCP_I_ESTABLISHED) << "\n";
    cMessage *msg = new cMessage(indicationName(TCP_I_ESTABLISHED));
    msg->setKind(TCP_I_ESTABLISHED);

    TCPConnectInfo *ind = new TCPConnectInfo();
    ind->setConnId(connId);
    ind->setLocalAddr(localAddr);
    ind->setRemoteAddr(remoteAddr);
    ind->setLocalPort(localPort);
    ind->setRemotePort(remotePort);

    msg->setControlInfo(ind);
    sendToApp(msg);
}

void TCPConnection::sendToApp(cMessage *msg)
{
    tcpMain->send(msg, "appOut", appGateIndex);
}

void TCPConnection::initConnection(TCPOpenCommand *openCmd)
{
    transferMode = (TCPDataTransferMode)(openCmd->getDataTransferMode());
    // create send queue
    sendQueue = tcpMain->createSendQueue(transferMode);
    sendQueue->setConnection(this);

    // create receive queue
    receiveQueue = tcpMain->createReceiveQueue(transferMode);
    receiveQueue->setConnection(this);

    // create SACK retransmit queue
    rexmitQueue = new TCPSACKRexmitQueue();
    rexmitQueue->setConnection(this);

    // create algorithm
    const char *tcpAlgorithmClass = openCmd->getTcpAlgorithmClass();

    if (!tcpAlgorithmClass || !tcpAlgorithmClass[0])
        tcpAlgorithmClass = tcpMain->par("tcpAlgorithmClass");

    tcpAlgorithm = check_and_cast<TCPAlgorithm *>(inet::utils::createOne(tcpAlgorithmClass));
    tcpAlgorithm->setConnection(this);

    // create state block
    state = tcpAlgorithm->getStateVariables();
    configureStateVariables();
    tcpAlgorithm->initialize();
}

void TCPConnection::configureStateVariables()
{
    long advertisedWindowPar = tcpMain->par("advertisedWindow").longValue();
    state->ws_support = tcpMain->par("windowScalingSupport");    // if set, this means that current host supports WS (RFC 1323)

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
    state->snd_mss = tcpMain->par("mss").longValue();    // Maximum Segment Size (RFC 793)
    state->ts_support = tcpMain->par("timestampSupport");    // if set, this means that current host supports TS (RFC 1323)
    state->sack_support = tcpMain->par("sackSupport");    // if set, this means that current host supports SACK (RFC 2018, 2883, 3517)

    if (state->sack_support) {
        std::string algorithmName1 = "TCPReno";
        std::string algorithmName2 = tcpMain->par("tcpAlgorithmClass");

        if (algorithmName1 != algorithmName2) {    // TODO add additional checks for new SACK supporting algorithms here once they are implemented
            EV_DEBUG << "If you want to use TCP SACK please set tcpAlgorithmClass to TCPReno\n";

            ASSERT(false);
        }
    }
}

void TCPConnection::selectInitialSeqNum()
{
    // set the initial send sequence number
    state->iss = (unsigned long)fmod(SIMTIME_DBL(simTime()) * 250000.0, 1.0 + (double)(unsigned)0xffffffffUL) & 0xffffffffUL;

    state->snd_una = state->snd_nxt = state->snd_max = state->iss;

    sendQueue->init(state->iss + 1);    // + 1 is for SYN
    rexmitQueue->init(state->iss + 1);    // + 1 is for SYN
}

bool TCPConnection::isSegmentAcceptable(TCPSegment *tcpseg) const
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
    uint32 len = tcpseg->getPayloadLength();
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

void TCPConnection::sendSyn()
{
    if (remoteAddr.isUnspecified() || remotePort == -1)
        throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: foreign socket unspecified");

    if (localPort == -1)
        throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: local port unspecified");

    // create segment
    TCPSegment *tcpseg = createTCPSegment("SYN");
    tcpseg->setSequenceNo(state->iss);
    tcpseg->setSynBit(true);
    updateRcvWnd();
    tcpseg->setWindow(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss + 1;

    // write header options
    writeHeaderOptions(tcpseg);

    // send it
    sendToIP(tcpseg);
}

void TCPConnection::sendSynAck()
{
    // create segment
    TCPSegment *tcpseg = createTCPSegment("SYN+ACK");
    tcpseg->setSequenceNo(state->iss);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setSynBit(true);
    tcpseg->setAckBit(true);
    updateRcvWnd();
    tcpseg->setWindow(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss + 1;

    // write header options
    writeHeaderOptions(tcpseg);

    // send it
    sendToIP(tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}

void TCPConnection::sendRst(uint32 seqNo)
{
    sendRst(seqNo, localAddr, remoteAddr, localPort, remotePort);
}

void TCPConnection::sendRst(uint32 seq, L3Address src, L3Address dest, int srcPort, int destPort)
{
    TCPSegment *tcpseg = createTCPSegment("RST");

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setSequenceNo(seq);

    // send it
    sendToIP(tcpseg, src, dest);
}

void TCPConnection::sendRstAck(uint32 seq, uint32 ack, L3Address src, L3Address dest, int srcPort, int destPort)
{
    TCPSegment *tcpseg = createTCPSegment("RST+ACK");

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setAckBit(true);
    tcpseg->setSequenceNo(seq);
    tcpseg->setAckNo(ack);

    // send it
    sendToIP(tcpseg, src, dest);

    // notify
    if (tcpAlgorithm)
        tcpAlgorithm->ackSent();
}

void TCPConnection::sendAck()
{
    TCPSegment *tcpseg = createTCPSegment("ACK");

    tcpseg->setAckBit(true);
    tcpseg->setSequenceNo(state->snd_nxt);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setWindow(updateRcvWnd());

    // write header options
    writeHeaderOptions(tcpseg);

    // send it
    sendToIP(tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}

void TCPConnection::sendFin()
{
    TCPSegment *tcpseg = createTCPSegment("FIN");

    // Note: ACK bit *must* be set for both FIN and FIN+ACK. What makes
    // the difference for FIN+ACK is that its ackNo acks the remote TCP's FIN.
    tcpseg->setFinBit(true);
    tcpseg->setAckBit(true);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setSequenceNo(state->snd_nxt);
    tcpseg->setWindow(updateRcvWnd());

    // send it
    sendToIP(tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}

void TCPConnection::sendSegment(uint32 bytes)
{
    if (state->sack_enabled && state->afterRto) {
        // check rexmitQ and try to forward snd_nxt before sending new data
        uint32 forward = rexmitQueue->checkRexmitQueueForSackedOrRexmittedSegments(state->snd_nxt);

        if (forward > 0) {
            EV_INFO << "sendSegment(" << bytes << ") forwarded " << forward << " bytes of snd_nxt from " << state->snd_nxt;
            state->snd_nxt += forward;
            EV_INFO << " to " << state->snd_nxt << endl;
            rexmitQueue->info();
        }
    }

    ulong buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (bytes > buffered) // last segment?
        bytes = buffered;

    // if header options will be added, this could reduce the number of data bytes allowed for this segment,
    // because following condition must to be respected:
    //     bytes + options_len <= snd_mss
    TCPSegment *tcpseg_temp = createTCPSegment(nullptr);
    tcpseg_temp->setAckBit(true);    // needed for TS option, otherwise TSecr will be set to 0
    writeHeaderOptions(tcpseg_temp);
    uint options_len = tcpseg_temp->getHeaderLength() - TCP_HEADER_OCTETS;    // TCP_HEADER_OCTETS = 20

    ASSERT(options_len < state->snd_mss);

    if (bytes + options_len > state->snd_mss)
        bytes = state->snd_mss - options_len;

    state->sentBytes = bytes;

    // send one segment of 'bytes' bytes from snd_nxt, and advance snd_nxt
    TCPSegment *tcpseg = sendQueue->createSegmentWithBytes(state->snd_nxt, bytes);

    // if sack_enabled copy region of tcpseg to rexmitQueue
    if (state->sack_enabled)
        rexmitQueue->enqueueSentData(state->snd_nxt, state->snd_nxt + bytes);

    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setAckBit(true);
    tcpseg->setWindow(updateRcvWnd());

    // TBD when to set PSH bit?
    // TBD set URG bit if needed
    ASSERT(bytes == tcpseg->getPayloadLength());

    state->snd_nxt += bytes;

    // check if afterRto bit can be reset
    if (state->afterRto && seqGE(state->snd_nxt, state->snd_max))
        state->afterRto = false;

    if (state->send_fin && state->snd_nxt == state->snd_fin_seq) {
        EV_DETAIL << "Setting FIN on segment\n";
        tcpseg->setFinBit(true);
        state->snd_nxt = state->snd_fin_seq + 1;
    }

    // add header options and update header length (from tcpseg_temp)
    for (uint i = 0; i < tcpseg_temp->getHeaderOptionArraySize(); i++)
        tcpseg->addHeaderOption(tcpseg_temp->getHeaderOption(i)->dup());

    tcpseg->setHeaderLength(tcpseg_temp->getHeaderLength());
    delete tcpseg_temp;

    // send it
    sendToIP(tcpseg);

    // let application fill queue again, if there is space
    const uint32 alreadyQueued = sendQueue->getBytesAvailable(sendQueue->getBufferStartSeq());
    const uint32 abated = (state->sendQueueLimit > alreadyQueued) ? state->sendQueueLimit - alreadyQueued : 0;
    if ((state->sendQueueLimit > 0) && !state->queueUpdate && (abated >= state->snd_mss)) {    // request more data if space >= 1 MSS
        // Tell upper layer readiness to accept more data
        sendIndicationToApp(TCP_I_SEND_MSG, abated);
        state->queueUpdate = true;
    }
}

bool TCPConnection::sendData(bool fullSegmentsOnly, uint32 congestionWindow)
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

    uint32 effectiveMaxBytesSend = state->snd_mss;

    if (state->ts_enabled)
        effectiveMaxBytesSend -= TCP_OPTION_TS_SIZE;

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

#ifdef TCP_SENDFRAGMENTS    /* normally undefined */
    // make agressive use of the window until the last byte
    while (bytesToSend > 0) {
        ulong bytes = std::min(bytesToSend, state->snd_mss);
        sendSegment(bytes);
        bytesToSend -= state->sentBytes;
    }
#else // ifdef TCP_SENDFRAGMENTS
      // send < MSS segments only if it's the only segment we can send now
      // Note: if (bytesToSend == 1010 && MSS == 1012 && ts_enabled == true) => we may send
      // 2 segments (1000 payload + 12 optionsHeader and 10 payload + 12 optionsHeader)
      // FIXME this should probably obey Nagle's alg -- to be checked
    if (bytesToSend <= state->snd_mss) {
        sendSegment(bytesToSend);
        bytesToSend -= state->sentBytes;
    }
    else {    // send whole segments only (nagle_enabled)
        while (bytesToSend >= effectiveMaxBytesSend) {
            sendSegment(state->snd_mss);
            bytesToSend -= state->sentBytes;
        }
    }

    // check how many bytes we have - last segment could be less than state->snd_mss
    buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (bytesToSend == buffered && buffered != 0) // last segment?
        sendSegment(bytesToSend);
    else if (bytesToSend > 0)
        EV_DETAIL << bytesToSend << " bytes of space left in effectiveWindow\n";
#endif // ifdef TCP_SENDFRAGMENTS

    // remember highest seq sent (snd_nxt may be set back on retransmission,
    // but we'll need snd_max to check validity of ACKs -- they must ack
    // something we really sent)
    if (seqGreater(state->snd_nxt, state->snd_max))
        state->snd_max = state->snd_nxt;

    if (unackedVector)
        unackedVector->record(state->snd_max - state->snd_una);

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

bool TCPConnection::sendProbe()
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

    if (unackedVector)
        unackedVector->record(state->snd_max - state->snd_una);

    // notify
    tcpAlgorithm->ackSent();
    tcpAlgorithm->dataSent(old_snd_nxt);

    return true;
}

void TCPConnection::retransmitOneSegment(bool called_at_rto)
{
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

        if (unackedVector)
            unackedVector->record(state->snd_max - state->snd_una);
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
}

void TCPConnection::retransmitData()
{
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

        if (unackedVector)
            unackedVector->record(state->snd_max - state->snd_una);
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

        bytesToSend -= state->sentBytes;
    }
    tcpAlgorithm->segmentRetransmitted(state->snd_una, state->snd_nxt);
}

void TCPConnection::readHeaderOptions(TCPSegment *tcpseg)
{
    EV_INFO << "TCP Header Option(s) received:\n";

    for (uint i = 0; i < tcpseg->getHeaderOptionArraySize(); i++) {
        const TCPOption *option = tcpseg->getHeaderOption(i);
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
                ok = processMSSOption(tcpseg, *check_and_cast<const TCPOptionMaxSegmentSize *>(option));
                break;

            case TCPOPTION_WINDOW_SCALE:    // WS=3
                ok = processWSOption(tcpseg, *check_and_cast<const TCPOptionWindowScale *>(option));
                break;

            case TCPOPTION_SACK_PERMITTED:    // SACK_PERMITTED=4
                ok = processSACKPermittedOption(tcpseg, *check_and_cast<const TCPOptionSackPermitted *>(option));
                break;

            case TCPOPTION_SACK:    // SACK=5
                ok = processSACKOption(tcpseg, *check_and_cast<const TCPOptionSack *>(option));
                break;

            case TCPOPTION_TIMESTAMP:    // TS=8
                ok = processTSOption(tcpseg, *check_and_cast<const TCPOptionTimestamp *>(option));
                break;

            // TODO add new TCPOptions here once they are implemented
            // TODO delegate to TCPAlgorithm as well -- it may want to recognized additional options

            default:
                EV_ERROR << "ERROR: Unsupported TCP option kind " << kind << "\n";
                break;
        }
        (void)ok;    // unused
    }
}

bool TCPConnection::processMSSOption(TCPSegment *tcpseg, const TCPOptionMaxSegmentSize& option)
{
    if (option.getLength() != 4) {
        EV_ERROR << "ERROR: MSS option length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: TCP Header Option MSS received, but in unexpected state\n";
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

    EV_INFO << "TCP Header Option MSS(=" << option.getMaxSegmentSize() << ") received, SMSS is set to " << state->snd_mss << "\n";
    return true;
}

bool TCPConnection::processWSOption(TCPSegment *tcpseg, const TCPOptionWindowScale& option)
{
    if (option.getLength() != 3) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: TCP Header Option WS received, but in unexpected state\n";
        return false;
    }

    state->rcv_ws = true;
    state->ws_enabled = state->ws_support && state->snd_ws && state->rcv_ws;
    state->snd_wnd_scale = option.getWindowScale();
    EV_INFO << "TCP Header Option WS(=" << state->snd_wnd_scale << ") received, WS (ws_enabled) is set to " << state->ws_enabled << "\n";

    if (state->snd_wnd_scale > 14) {    // RFC 1323, page 11: "the shift count must be limited to 14"
        EV_ERROR << "ERROR: TCP Header Option WS received but shift count value is exceeding 14\n";
        state->snd_wnd_scale = 14;
    }

    return true;
}

bool TCPConnection::processTSOption(TCPSegment *tcpseg, const TCPOptionTimestamp& option)
{
    if (option.getLength() != 10) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if ((!state->ts_enabled && fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) ||
        (state->ts_enabled && fsm.getState() != TCP_S_SYN_RCVD && fsm.getState() != TCP_S_ESTABLISHED &&
         fsm.getState() != TCP_S_FIN_WAIT_1 && fsm.getState() != TCP_S_FIN_WAIT_2))
    {
        EV_ERROR << "ERROR: TCP Header Option TS received, but in unexpected state\n";
        return false;
    }

    if (!state->ts_enabled) {
        state->rcv_initial_ts = true;
        state->ts_enabled = state->ts_support && state->snd_initial_ts && state->rcv_initial_ts;
        EV_INFO << "TCP Header Option TS(TSval=" << option.getSenderTimestamp() << ", TSecr=" << option.getEchoedTimestamp() << ") received, TS (ts_enabled) is set to " << state->ts_enabled << "\n";
    }
    else
        EV_INFO << "TCP Header Option TS(TSval=" << option.getSenderTimestamp() << ", TSecr=" << option.getEchoedTimestamp() << ") received\n";

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

bool TCPConnection::processSACKPermittedOption(TCPSegment *tcpseg, const TCPOptionSackPermitted& option)
{
    if (option.getLength() != 2) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: TCP Header Option SACK_PERMITTED received, but in unexpected state\n";
        return false;
    }

    state->rcv_sack_perm = true;
    state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
    EV_INFO << "TCP Header Option SACK_PERMITTED received, SACK (sack_enabled) is set to " << state->sack_enabled << "\n";
    return true;
}

TCPSegment TCPConnection::writeHeaderOptions(TCPSegment *tcpseg)
{
    // SYN flag set and connetion in INIT or LISTEN state (or after synRexmit timeout)
    if (tcpseg->getSynBit() && (fsm.getState() == TCP_S_INIT || fsm.getState() == TCP_S_LISTEN
                                || ((fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD)
                                    && state->syn_rexmit_count > 0)))
    {
        // MSS header option
        if (state->snd_mss > 0) {
            TCPOptionMaxSegmentSize *option = new TCPOptionMaxSegmentSize();
            option->setMaxSegmentSize(state->snd_mss);
            tcpseg->addHeaderOption(option);
            EV_INFO << "TCP Header Option MSS(=" << state->snd_mss << ") sent\n";
        }

        // WS header option
        if (state->ws_support && (state->rcv_ws || (fsm.getState() == TCP_S_INIT
                                                    || (fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count > 0))))
        {
            // 1 padding byte
            tcpseg->addHeaderOption(new TCPOptionNop());    // NOP

            // Update WS variables
            //ulong scaled_rcv_wnd = receiveQueue->getAmountOfFreeBytes(state->maxRcvBuffer);
            ulong scaled_rcv_wnd = receiveQueue->getFirstSeqNo() + state->maxRcvBuffer - state->rcv_nxt;
            state->rcv_wnd_scale = 0;

            while (scaled_rcv_wnd > TCP_MAX_WIN && state->rcv_wnd_scale < 14) {    // RFC 1323, page 11: "the shift count must be limited to 14"
                scaled_rcv_wnd = scaled_rcv_wnd >> 1;
                state->rcv_wnd_scale++;
            }

            TCPOptionWindowScale *option = new TCPOptionWindowScale();
            option->setWindowScale(state->rcv_wnd_scale);    // rcv_wnd_scale is also set in scaleRcvWnd()
            state->snd_ws = true;
            state->ws_enabled = state->ws_support && state->snd_ws && state->rcv_ws;
            EV_INFO << "TCP Header Option WS(=" << option->getWindowScale() << ") sent, WS (ws_enabled) is set to " << state->ws_enabled << "\n";
            tcpseg->addHeaderOption(option);
        }

        // SACK_PERMITTED header option
        if (state->sack_support && (state->rcv_sack_perm || (fsm.getState() == TCP_S_INIT
                                                             || (fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count > 0))))
        {
            if (!state->ts_support) {    // if TS is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpseg->addHeaderOption(new TCPOptionNop());    // NOP
                tcpseg->addHeaderOption(new TCPOptionNop());    // NOP
            }

            tcpseg->addHeaderOption(new TCPOptionSackPermitted());

            // Update SACK variables
            state->snd_sack_perm = true;
            state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
            EV_INFO << "TCP Header Option SACK_PERMITTED sent, SACK (sack_enabled) is set to " << state->sack_enabled << "\n";
        }

        // TS header option
        if (state->ts_support && (state->rcv_initial_ts || (fsm.getState() == TCP_S_INIT
                                                            || (fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count > 0))))
        {
            if (!state->sack_support) {    // if SACK is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpseg->addHeaderOption(new TCPOptionNop());    // NOP
                tcpseg->addHeaderOption(new TCPOptionNop());    // NOP
            }

            TCPOptionTimestamp *option = new TCPOptionTimestamp();

            // Update TS variables
            // RFC 1323, page 13: "The Timestamp Value field (TSval) contains the current value of the timestamp clock of the TCP sending the option."
            option->setSenderTimestamp(convertSimtimeToTS(simTime()));

            // RFC 1323, page 16: "(3) When a TSopt is sent, its TSecr field is set to the current TS.Recent value."
            // RFC 1323, page 13:
            // "The Timestamp Echo Reply field (TSecr) is only valid if the ACK
            // bit is set in the TCP header; if it is valid, it echos a times-
            // tamp value that was sent by the remote TCP in the TSval field
            // of a Timestamps option.  When TSecr is not valid, its value
            // must be zero."
            option->setEchoedTimestamp(tcpseg->getAckBit() ? state->ts_recent : 0);

            state->snd_initial_ts = true;
            state->ts_enabled = state->ts_support && state->snd_initial_ts && state->rcv_initial_ts;
            EV_INFO << "TCP Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent, TS (ts_enabled) is set to " << state->ts_enabled << "\n";
            tcpseg->addHeaderOption(option);
        }

        // TODO add new TCPOptions here once they are implemented
    }
    else if (fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD
             || fsm.getState() == TCP_S_ESTABLISHED || fsm.getState() == TCP_S_FIN_WAIT_1
             || fsm.getState() == TCP_S_FIN_WAIT_2)    // connetion is not in INIT or LISTEN state
    {
        // TS header option
        if (state->ts_enabled) {    // Is TS enabled?
            if (!(state->sack_enabled && (state->snd_sack || state->snd_dsack))) {    // if SACK is enabled and SACKs need to be added, do not add NOPs to this segment
                // 2 padding bytes
                tcpseg->addHeaderOption(new TCPOptionNop());    // NOP
                tcpseg->addHeaderOption(new TCPOptionNop());    // NOP
            }

            TCPOptionTimestamp *option = new TCPOptionTimestamp();

            // Update TS variables
            // RFC 1323, page 13: "The Timestamp Value field (TSval) contains the current value of the timestamp clock of the TCP sending the option."
            option->setSenderTimestamp(convertSimtimeToTS(simTime()));

            // RFC 1323, page 16: "(3) When a TSopt is sent, its TSecr field is set to the current TS.Recent value."
            // RFC 1323, page 13:
            // "The Timestamp Echo Reply field (TSecr) is only valid if the ACK
            // bit is set in the TCP header; if it is valid, it echos a times-
            // tamp value that was sent by the remote TCP in the TSval field
            // of a Timestamps option.  When TSecr is not valid, its value
            // must be zero."
            option->setEchoedTimestamp(tcpseg->getAckBit() ? state->ts_recent : 0);

            EV_INFO << "TCP Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent\n";
            tcpseg->addHeaderOption(option);
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
        // TODO delegate to TCPAlgorithm as well -- it may want to append additional options
    }

    if (tcpseg->getHeaderOptionArraySize() != 0) {
        uint options_len = tcpseg->getHeaderOptionArrayLength();

        if (options_len <= TCP_OPTIONS_MAX_SIZE) // Options length allowed? - maximum: 40 Bytes
            tcpseg->setHeaderLength(TCP_HEADER_OCTETS + options_len); // TCP_HEADER_OCTETS = 20
        else {
            tcpseg->setHeaderLength(TCP_HEADER_OCTETS);    // TCP_HEADER_OCTETS = 20
            tcpseg->dropHeaderOptions();    // drop all options
            EV_ERROR << "ERROR: Options length exceeded! Segment will be sent without options" << "\n";
        }
    }

    return *tcpseg;
}

uint32 TCPConnection::getTSval(TCPSegment *tcpseg) const
{
    for (uint i = 0; i < tcpseg->getHeaderOptionArraySize(); i++) {
        const TCPOption *option = tcpseg->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_TIMESTAMP)
            return check_and_cast<const TCPOptionTimestamp *>(option)->getSenderTimestamp();
    }

    return 0;
}

uint32 TCPConnection::getTSecr(TCPSegment *tcpseg) const
{
    for (uint i = 0; i < tcpseg->getHeaderOptionArraySize(); i++) {
        const TCPOption *option = tcpseg->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_TIMESTAMP)
            return check_and_cast<const TCPOptionTimestamp *>(option)->getEchoedTimestamp();
    }

    return 0;
}

void TCPConnection::updateRcvQueueVars()
{
    // update receive queue related state variables
    state->freeRcvBuffer = receiveQueue->getAmountOfFreeBytes(state->maxRcvBuffer);
    state->usedRcvBuffer = state->maxRcvBuffer - state->freeRcvBuffer;

    // update receive queue related statistics
    if (tcpRcvQueueBytesVector)
        tcpRcvQueueBytesVector->record(state->usedRcvBuffer);

//    tcpEV << "receiveQ: receiveQLength=" << receiveQueue->getQueueLength() << " maxRcvBuffer=" << state->maxRcvBuffer << " usedRcvBuffer=" << state->usedRcvBuffer << " freeRcvBuffer=" << state->freeRcvBuffer << "\n";
}

unsigned short TCPConnection::updateRcvWnd()
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
    if (win > TCP_MAX_WIN && !state->ws_enabled) // TCP_MAX_WIN = 65535 (16 bit)
        win = TCP_MAX_WIN; // Note: The window size is limited to a 16 bit value in the TCP header if WINDOW SCALE option (RFC 1323) is not used

    // Note: The order of the "Do not shrink window" and "Observe upper limit" parts has been changed to the order used in FreeBSD Release 7.1

    // update rcv_adv if needed
    if (win > 0 && seqGE(state->rcv_nxt + win, state->rcv_adv)) {
        state->rcv_adv = state->rcv_nxt + win;

        if (rcvAdvVector)
            rcvAdvVector->record(state->rcv_adv);
    }

    state->rcv_wnd = win;

    if (rcvWndVector)
        rcvWndVector->record(state->rcv_wnd);

    // scale rcv_wnd:
    uint32 scaled_rcv_wnd = state->rcv_wnd;
    if (state->ws_enabled && state->rcv_wnd_scale) {
        ASSERT(state->rcv_wnd_scale <= 14);   // RFC 1323, page 11: "the shift count must be limited to 14"
        scaled_rcv_wnd = scaled_rcv_wnd >> state->rcv_wnd_scale;
    }

    ASSERT(scaled_rcv_wnd == (unsigned short)scaled_rcv_wnd);

    return (unsigned short)scaled_rcv_wnd;
}

void TCPConnection::updateWndInfo(TCPSegment *tcpseg, bool doAlways)
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

        if (sndWndVector)
            sndWndVector->record(state->snd_wnd);
    }
}

void TCPConnection::sendOneNewSegment(bool fullSegmentsOnly, uint32 congestionWindow)
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

                    if (unackedVector)
                        unackedVector->record(state->snd_max - state->snd_una);

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

uint32 TCPConnection::convertSimtimeToTS(simtime_t simtime)
{
    ASSERT(SimTime::getScaleExp() <= -3);

    uint32 timestamp = (uint32)(simtime.inUnit(SIMTIME_MS));
    return timestamp;
}

simtime_t TCPConnection::convertTSToSimtime(uint32 timestamp)
{
    ASSERT(SimTime::getScaleExp() <= -3);

    simtime_t simtime(timestamp, SIMTIME_MS);
    return simtime;
}

bool TCPConnection::isSendQueueEmpty()
{
    return sendQueue->getBytesAvailable(state->snd_nxt) == 0;
}

} // namespace tcp

} // namespace inet

