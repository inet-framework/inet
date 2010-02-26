//
// Copyright (C) 2004 Andras Varga
//               2009 Thomas Reschka
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
#include <algorithm>   // min,max
#include "TCP.h"
#include "TCPConnection.h"
#include "TCPSegment.h"
#include "TCPCommand_m.h"
#include "IPControlInfo.h"
#include "IPv6ControlInfo.h"
#include "TCPSendQueue.h"
#include "TCPSACKRexmitQueue.h"
#include "TCPReceiveQueue.h"
#include "TCPAlgorithm.h"

//
// helper functions
//

const char *TCPConnection::stateName(int state)
{
#define CASE(x) case x: s=#x+6; break
    const char *s = "unknown";
    switch (state)
    {
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
#define CASE(x) case x: s=#x+6; break
    const char *s = "unknown";
    switch (event)
    {
        CASE(TCP_E_IGNORE);
        CASE(TCP_E_OPEN_ACTIVE);
        CASE(TCP_E_OPEN_PASSIVE);
        CASE(TCP_E_SEND);
        CASE(TCP_E_CLOSE);
        CASE(TCP_E_ABORT);
        CASE(TCP_E_STATUS);
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
#define CASE(x) case x: s=#x+6; break
    const char *s = "unknown";
    switch (code)
    {
        CASE(TCP_I_DATA);
        CASE(TCP_I_URGENT_DATA);
        CASE(TCP_I_ESTABLISHED);
        CASE(TCP_I_PEER_CLOSED);
        CASE(TCP_I_CLOSED);
        CASE(TCP_I_CONNECTION_REFUSED);
        CASE(TCP_I_CONNECTION_RESET);
        CASE(TCP_I_TIMED_OUT);
        CASE(TCP_I_STATUS);
    }
    return s;
#undef CASE
}

void TCPConnection::printConnBrief()
{
    tcpEV << "Connection ";
    tcpEV << localAddr << ":" << localPort << " to " << remoteAddr << ":" << remotePort;
    tcpEV << "  on app[" << appGateIndex << "],connId=" << connId;
    tcpEV << "  in " << stateName(fsm.getState());
    tcpEV << "  (ptr=0x" << this << ")\n";
}

void TCPConnection::printSegmentBrief(TCPSegment *tcpseg)
{
    tcpEV << "." << tcpseg->getSrcPort() << " > ";
    tcpEV << "." << tcpseg->getDestPort() << ": ";

    if (tcpseg->getSynBit())  tcpEV << (tcpseg->getAckBit() ? "SYN+ACK " : "SYN ");
    if (tcpseg->getFinBit())  tcpEV << "FIN(+ACK) ";
    if (tcpseg->getRstBit())  tcpEV << (tcpseg->getAckBit() ? "RST+ACK " : "RST ");
    if (tcpseg->getPshBit())  tcpEV << "PSH ";

    if (tcpseg->getPayloadLength()>0 || tcpseg->getSynBit())
    {
        tcpEV << "[" << tcpseg->getSequenceNo() << ".." << (tcpseg->getSequenceNo()+tcpseg->getPayloadLength()) << ") ";
        tcpEV << "(l=" << tcpseg->getPayloadLength() << ") ";
    }
    if (tcpseg->getAckBit())  tcpEV << "ack " << tcpseg->getAckNo() << " ";
    tcpEV << "win " << tcpseg->getWindow() << "\n";
    if (tcpseg->getUrgBit())  tcpEV << "urg " << tcpseg->getUrgentPointer() << " ";
}

TCPConnection *TCPConnection::cloneListeningConnection()
{
    TCPConnection *conn = new TCPConnection(tcpMain,appGateIndex,connId);

    // following code to be kept consistent with initConnection()
    const char *sendQueueClass = sendQueue->getClassName();
    conn->sendQueue = check_and_cast<TCPSendQueue *>(createOne(sendQueueClass));
    conn->sendQueue->setConnection(conn);

    const char *receiveQueueClass = receiveQueue->getClassName();
    conn->receiveQueue = check_and_cast<TCPReceiveQueue *>(createOne(receiveQueueClass));
    conn->receiveQueue->setConnection(conn);

    // create SACK retransmit queue
    rexmitQueue = new TCPSACKRexmitQueue();
    rexmitQueue->setConnection(this);

    const char *tcpAlgorithmClass = tcpAlgorithm->getClassName();
    conn->tcpAlgorithm = check_and_cast<TCPAlgorithm *>(createOne(tcpAlgorithmClass));
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
    if (sndNxtVector && tcpseg->getPayloadLength()!=0)
        sndNxtVector->record(tcpseg->getSequenceNo());
    if (sndAckVector)
        sndAckVector->record(tcpseg->getAckNo());

    // final touches on the segment before sending
    tcpseg->setSrcPort(localPort);
    tcpseg->setDestPort(remotePort);
    ASSERT(tcpseg->getHeaderLength() >= TCP_HEADER_OCTETS);     // TCP_HEADER_OCTETS = 20 (without options)
    ASSERT(tcpseg->getHeaderLength() <= TCP_MAX_HEADER_OCTETS); // TCP_MAX_HEADER_OCTETS = 60
    tcpseg->setByteLength(tcpseg->getHeaderLength() + tcpseg->getPayloadLength());


    tcpEV << "Sending: ";
    printSegmentBrief(tcpseg);

    // TBD reuse next function for sending

    if (!remoteAddr.isIPv6())
    {
        // send over IPv4
        IPControlInfo *controlInfo = new IPControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(localAddr.get4());
        controlInfo->setDestAddr(remoteAddr.get4());
        tcpseg->setControlInfo(controlInfo);

        tcpMain->send(tcpseg,"ipOut");
    }
    else
    {
        // send over IPv6
        IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(localAddr.get6());
        controlInfo->setDestAddr(remoteAddr.get6());
        tcpseg->setControlInfo(controlInfo);

        tcpMain->send(tcpseg,"ipv6Out");
    }
}

void TCPConnection::sendToIP(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest)
{
    tcpEV << "Sending: ";
    printSegmentBrief(tcpseg);

    if (!dest.isIPv6())
    {
        // send over IPv4
        IPControlInfo *controlInfo = new IPControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(src.get4());
        controlInfo->setDestAddr(dest.get4());
        tcpseg->setControlInfo(controlInfo);

        check_and_cast<TCP *>(simulation.getContextModule())->send(tcpseg,"ipOut");
    }
    else
    {
        // send over IPv6
        IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(src.get6());
        controlInfo->setDestAddr(dest.get6());
        tcpseg->setControlInfo(controlInfo);

        check_and_cast<TCP *>(simulation.getContextModule())->send(tcpseg,"ipv6Out");
    }
}

TCPSegment *TCPConnection::createTCPSegment(const char *name)
{
    return new TCPSegment(name);
}

void TCPConnection::signalConnectionTimeout()
{
    sendIndicationToApp(TCP_I_TIMED_OUT);
}

void TCPConnection::sendIndicationToApp(int code)
{
    tcpEV << "Notifying app: " << indicationName(code) << "\n";
    cMessage *msg = new cMessage(indicationName(code));
    msg->setKind(code);
    TCPCommand *ind = new TCPCommand();
    ind->setConnId(connId);
    msg->setControlInfo(ind);
    tcpMain->send(msg, "appOut", appGateIndex);
}

void TCPConnection::sendEstabIndicationToApp()
{
    tcpEV << "Notifying app: " << indicationName(TCP_I_ESTABLISHED) << "\n";
    cMessage *msg = new cMessage(indicationName(TCP_I_ESTABLISHED));
    msg->setKind(TCP_I_ESTABLISHED);

    TCPConnectInfo *ind = new TCPConnectInfo();
    ind->setConnId(connId);
    ind->setLocalAddr(localAddr);
    ind->setRemoteAddr(remoteAddr);
    ind->setLocalPort(localPort);
    ind->setRemotePort(remotePort);

    msg->setControlInfo(ind);
    tcpMain->send(msg, "appOut", appGateIndex);
}

void TCPConnection::sendToApp(cMessage *msg)
{
    tcpMain->send(msg, "appOut", appGateIndex);
}

void TCPConnection::initConnection(TCPOpenCommand *openCmd)
{
    // create send queue
    const char *sendQueueClass = openCmd->getSendQueueClass();
    if (!sendQueueClass || !sendQueueClass[0])
        sendQueueClass = tcpMain->par("sendQueueClass");
    sendQueue = check_and_cast<TCPSendQueue *>(createOne(sendQueueClass));
    sendQueue->setConnection(this);

    // create receive queue
    const char *receiveQueueClass = openCmd->getReceiveQueueClass();
    if (!receiveQueueClass || !receiveQueueClass[0])
        receiveQueueClass = tcpMain->par("receiveQueueClass");
    receiveQueue = check_and_cast<TCPReceiveQueue *>(createOne(receiveQueueClass));
    receiveQueue->setConnection(this);

    // create SACK retransmit queue
    rexmitQueue = new TCPSACKRexmitQueue();
    rexmitQueue->setConnection(this);

    // create algorithm
    const char *tcpAlgorithmClass = openCmd->getTcpAlgorithmClass();
    if (!tcpAlgorithmClass || !tcpAlgorithmClass[0])
        tcpAlgorithmClass = tcpMain->par("tcpAlgorithmClass");
    tcpAlgorithm = check_and_cast<TCPAlgorithm *>(createOne(tcpAlgorithmClass));
    tcpAlgorithm->setConnection(this);

    // create state block
    state = tcpAlgorithm->getStateVariables();
    configureStateVariables();
    tcpAlgorithm->initialize();
}

void TCPConnection::configureStateVariables()
{
    long advertisedWindowPar = tcpMain->par("advertisedWindow").longValue();
    if (advertisedWindowPar > TCP_MAX_WIN || advertisedWindowPar <= 0)
        throw cRuntimeError("Invalid advertisedWindow parameter: %ld", advertisedWindowPar);

    state->delayed_acks_enabled = tcpMain->par("delayedAcksEnabled"); // delayed ACKs enabled/disabled
    state->nagle_enabled = tcpMain->par("nagleEnabled"); // Nagle's algorithm enabled/disabled
    state->limited_transmit_enabled = tcpMain->par("limitedTransmitEnabled"); // Limited Transmit algorithm (RFC 3042) enabled/disabled
    state->increased_IW_enabled = tcpMain->par("increasedIWEnabled"); // increased initial window (=2*SMSS) (RFC 2581) enabled/disabled
    state->rcv_wnd = advertisedWindowPar;
    state->rcv_adv = advertisedWindowPar;
    state->maxRcvBuffer = advertisedWindowPar;
    state->snd_mss = tcpMain->par("mss").longValue(); // maximum segment size
    state->sack_support = tcpMain->par("sackSupport"); // if set, this means that current host supports SACK (RFC 2018, 2883, 3517)
    if (state->sack_support)
    {
        std::string algorithmName1 = "TCPReno";
        std::string algorithmName2 = tcpMain->par("tcpAlgorithmClass");
        if (algorithmName1!=algorithmName2) // TODO add additional checks for new SACK supporting algorithms here once they are implemented
        {
            EV << "If you want to use TCP SACK please set tcpAlgorithmClass to TCPReno" << endl;
            ASSERT(false);
        }
    }
}

void TCPConnection::selectInitialSeqNum()
{
    // set the initial send sequence number
    state->iss = (unsigned long)fmod(SIMTIME_DBL(simTime())*250000.0, 1.0+(double)(unsigned)0xffffffffUL) & 0xffffffffUL;

    state->snd_una = state->snd_nxt = state->snd_max = state->iss;

    sendQueue->init(state->iss+1); // +1 is for SYN
    rexmitQueue->init(state->iss + 1); // +1 is for SYN
}

bool TCPConnection::isSegmentAcceptable(TCPSegment *tcpseg)
{
    // check that segment entirely falls in receive window
    // RFC 793, page 69:
    // There are four cases for the acceptability test for an incoming segment:
    uint32 len = tcpseg->getPayloadLength();
    uint32 seqNo = tcpseg->getSequenceNo();

    if (len == 0)
    {
        if (state->rcv_wnd == 0)
            return seqNo == state->rcv_nxt;
        else // rcv_wnd > 0
            return seqLE(state->rcv_nxt, seqNo) && seqLess(seqNo, state->rcv_nxt + state->rcv_wnd);
    }
    else // len > 0
    {
        if (state->rcv_wnd == 0)
            return false;
        else // rcv_wnd > 0
            return (seqLE(state->rcv_nxt, seqNo) && seqLess(seqNo, state->rcv_nxt + state->rcv_wnd))
            ||
            (seqLE(state->rcv_nxt, seqNo + len - 1) && seqLess(seqNo + len - 1, state->rcv_nxt + state->rcv_wnd));
    }
}

void TCPConnection::sendSyn()
{
    if (remoteAddr.isUnspecified() || remotePort==-1)
        opp_error("Error processing command OPEN_ACTIVE: foreign socket unspecified");
    if (localPort==-1)
        opp_error("Error processing command OPEN_ACTIVE: local port unspecified");

    // create segment
    TCPSegment *tcpseg = createTCPSegment("SYN");
    tcpseg->setSequenceNo(state->iss);
    tcpseg->setSynBit(true);
    updateRcvWnd();
    tcpseg->setWindow(state->rcv_wnd);
    if (rcvWndVector)
        rcvWndVector->record(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss+1;

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
    if (rcvWndVector)
        rcvWndVector->record(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss+1;

    // write header options
    writeHeaderOptions(tcpseg);

    // send it
    sendToIP(tcpseg);
}

void TCPConnection::sendRst(uint32 seqNo)
{
    sendRst(seqNo, localAddr, remoteAddr, localPort, remotePort);
}

void TCPConnection::sendRst(uint32 seq, IPvXAddress src, IPvXAddress dest, int srcPort, int destPort)
{
    TCPSegment *tcpseg = createTCPSegment("RST");

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setSequenceNo(seq);

    // send it
    sendToIP(tcpseg, src, dest);
}

void TCPConnection::sendRstAck(uint32 seq, uint32 ack, IPvXAddress src, IPvXAddress dest, int srcPort, int destPort)
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
}

void TCPConnection::sendAck()
{
    TCPSegment *tcpseg = createTCPSegment("ACK");

    tcpseg->setAckBit(true);
    tcpseg->setSequenceNo(state->snd_nxt);
    tcpseg->setAckNo(state->rcv_nxt);
    updateRcvWnd();
    tcpseg->setWindow(state->rcv_wnd);
    if (rcvWndVector)
        rcvWndVector->record(state->rcv_wnd);

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
    updateRcvWnd();
    tcpseg->setWindow(state->rcv_wnd);
    if (rcvWndVector)
        rcvWndVector->record(state->rcv_wnd);

    // send it
    sendToIP(tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}

void TCPConnection::sendSegment(uint32 bytes)
{
    if (state->sack_enabled && state->afterRto)
    {
        // check rexmitQ and try to forward snd_nxt before sending new data
        uint32 forward = rexmitQueue->checkRexmitQueueForSackedOrRexmittedSegments(state->snd_nxt);
        state->snd_nxt = state->snd_nxt + forward;
    }

    ulong buffered = sendQueue->getBytesAvailable(state->snd_nxt);
    if (bytes > buffered) { // last segment?
        bytes = buffered;
    }

    // send one segment of 'bytes' bytes from snd_nxt, and advance snd_nxt
    TCPSegment *tcpseg = sendQueue->createSegmentWithBytes(state->snd_nxt, bytes);

    // if sack_enabled copy region of tcpseg to rexmitQueue
    if (state->sack_enabled)
        rexmitQueue->enqueueSentData(state->snd_nxt, state->snd_nxt+bytes);

    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setAckBit(true);
    updateRcvWnd();
    tcpseg->setWindow(state->rcv_wnd);
    if (rcvWndVector)
        rcvWndVector->record(state->rcv_wnd);
    // TBD when to set PSH bit?
    // TBD set URG bit if needed
    ASSERT(bytes==tcpseg->getPayloadLength());

    state->snd_nxt += bytes;

    // check if afterRto bit can be reset
    if (state->afterRto && seqGE(state->snd_nxt, state->snd_max))
        state->afterRto = false;

    if (state->send_fin && state->snd_nxt==state->snd_fin_seq)
    {
        tcpEV << "Setting FIN on segment\n";
        tcpseg->setFinBit(true);
        state->snd_nxt = state->snd_fin_seq+1;
    }

    sendToIP(tcpseg);
}

bool TCPConnection::sendData(bool fullSegmentsOnly, uint32 congestionWindow) // changed from int congestionWindow to uint32 congestionWindow 2009-08-05 by T.R.
{
    if (!state->afterRto)
        // we'll start sending from snd_max
        state->snd_nxt = state->snd_max;

    uint32 old_highRxt = 0;
    if (state->sack_enabled)
        old_highRxt = rexmitQueue->getHighestRexmittedSeqNum();

    // check how many bytes we have
    ulong buffered = sendQueue->getBytesAvailable(state->snd_nxt);
    if (buffered==0)
        return false;

    // maxWindow is minimum of snd_wnd and congestionWindow (snd_cwnd)
    ulong maxWindow = std::min(state->snd_wnd, congestionWindow);

    // effectiveWindow: number of bytes we're allowed to send now
    long effectiveWin = maxWindow - (state->snd_nxt - state->snd_una);
    if (effectiveWin <= 0)
    {
        tcpEV << "Effective window is zero (advertised window " << state->snd_wnd <<
            ", congestion window " << congestionWindow << "), cannot send.\n";
        return false;
    }

    ulong bytesToSend = effectiveWin;

    if (bytesToSend > buffered)
        bytesToSend = buffered;

    if (fullSegmentsOnly && bytesToSend < state->snd_mss && buffered > (ulong) effectiveWin) // last segment could be less then state->snd_mss
    {
        tcpEV << "Cannot send, not enough data for a full segment (SMSS=" << state->snd_mss
            << ", in buffer " << buffered << ")\n";
        return false;
    }

    // start sending 'bytesToSend' bytes
    tcpEV << "Will send " << bytesToSend << " bytes (effectiveWindow " << effectiveWin
        << ", in buffer " << buffered << " bytes)\n";

    uint32 old_snd_nxt = state->snd_nxt;
    ASSERT(bytesToSend>0);

#ifdef TCP_SENDFRAGMENTS  /* normally undefined */
    // make agressive use of the window until the last byte
    while (bytesToSend>0)
    {
        ulong bytes = std::min(bytesToSend, state->snd_mss);
        sendSegment(bytes);
        bytesToSend -= bytes;
    }
#else
    // send <MSS segments only if it's the only segment we can send now
    // FIXME this should probably obey Nagle's alg -- to be checked
    if (bytesToSend <= state->snd_mss)
    {
        sendSegment(bytesToSend);
    }
    else
    {
        // send whole segments only (nagle_enabled)
        while (bytesToSend>=state->snd_mss)
        {
            sendSegment(state->snd_mss);
            bytesToSend -= state->snd_mss;
        }
        // check how many bytes we have - last segment could be less then state->snd_mss
        buffered = sendQueue->getBytesAvailable(state->snd_nxt);
        if (bytesToSend==buffered && buffered!=0) // last segment?
            sendSegment(bytesToSend);
        else if (bytesToSend>0)
            tcpEV << bytesToSend << " bytes of space left in effectiveWindow\n";
    }
#endif

    // remember highest seq sent (snd_nxt may be set back on retransmission,
    // but we'll need snd_max to check validity of ACKs -- they must ack
    // something we really sent)
    state->snd_max = std::max (state->snd_nxt, state->snd_max);
    if (unackedVector) unackedVector->record(state->snd_max - state->snd_una);

    // notify (once is enough)
    tcpAlgorithm->ackSent();
    if (state->sack_enabled && state->lossRecovery && old_highRxt != state->highRxt)
    {
        // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
        tcpEV << "Retransmission sent during recovery, restarting REXMIT timer.\n";
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
    if (sendQueue->getBytesAvailable(state->snd_nxt)==0)
    {
        tcpEV << "Cannot send probe because send buffer is empty\n";
        return false;
    }

    uint32 old_snd_nxt = state->snd_nxt;

    tcpEV << "Sending 1 byte as probe, with seq=" << state->snd_nxt << "\n";
    sendSegment(1);

    // remember highest seq sent (snd_nxt may be set back on retransmission,
    // but we'll need snd_max to check validity of ACKs -- they must ack
    // something we really sent)
    state->snd_max = state->snd_nxt;
    if (unackedVector) unackedVector->record(state->snd_max - state->snd_una);

    // notify
    tcpAlgorithm->ackSent();
    tcpAlgorithm->dataSent(old_snd_nxt);

    return true;
}

void TCPConnection::retransmitOneSegment()
{
    // retransmit one segment at snd_una, and set snd_nxt accordingly
    state->snd_nxt = state->snd_una;

    ulong bytes = std::min(state->snd_mss, state->snd_max - state->snd_nxt);
    ASSERT(bytes!=0);

    sendSegment(bytes);

    // notify
    tcpAlgorithm->ackSent();

    if (state->sack_enabled)
    {
        // RFC 3517, page 7: "(3) Retransmit the first data segment presumed dropped -- the segment
        // starting with sequence number HighACK + 1.  To prevent repeated
        // retransmission of the same data, set HighRxt to the highest
        // sequence number in the retransmitted segment."
        state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
    }
}

void TCPConnection::retransmitData()
{
    // retransmit everything from snd_una
    state->snd_nxt = state->snd_una;

    ulong bytesToSend = state->snd_max - state->snd_nxt;
    ASSERT(bytesToSend!=0);

    // TBD - avoid to send more than allowed - check cwnd and rwnd before retransmitting data!
    while (bytesToSend>0)
    {
        ulong bytes = std::min(bytesToSend, (ulong)state->snd_mss);
        bytes = std::min(bytes, sendQueue->getBytesAvailable(state->snd_nxt));
        sendSegment(bytes);
        // Do not send packets after the FIN.
        // fixes bug that occurs in examples/inet/bulktransfer at event #64043  T=13.861159213744
        if (state->send_fin && state->snd_nxt==state->snd_fin_seq+1)
            break;
        bytesToSend -= bytes;
    }
}

void TCPConnection::readHeaderOptions(TCPSegment *tcpseg)
{
    tcpEV << "TCP Header Option(s) received:\n";

    for (uint i=0; i<tcpseg->getOptionsArraySize(); i++)
    {
        const TCPOption& option = tcpseg->getOptions(i);
        short kind = option.getKind();
        short length = option.getLength();
        tcpEV << "Received TCP option of kind " << kind << " with length " << length << "\n";
        bool ok = true;
        switch(kind)
        {
            case TCPOPTION_END_OF_OPTION_LIST: // EOL=0
                tcpEV << "Option type: EOL\n";
                if (length != 1)
                {
                    tcpEV << "ERROR: EOL option length incorrect\n";
                    ok = false;
                }
                break;
            case TCPOPTION_NO_OPERATION: // NOP=1
                tcpEV << "Option type: NOP\n";
                if (length != 1)
                {
                    tcpEV << "ERROR: NOP option length incorrect\n";
                    ok = false;
                }
                break;
            case TCPOPTION_MAXIMUM_SEGMENT_SIZE: // MSS=2
                tcpEV << "Option type: MSS\n";
                ok = processMSSOption(tcpseg, option);
                break;
            case TCPOPTION_SACK_PERMITTED: // SACK_PERMITTED=4
                tcpEV << "Option type: SACK_PERMITTED\n";
                ok = processSACKPermittedOption(tcpseg, option);
                break;
            case TCPOPTION_SACK: // SACK=5
                tcpEV << "Option type: SACK\n";
                ok = processSACKOption(tcpseg, option);
                break;
            // TODO add new TCPOptions here once they are implemented
            // TODO delegate to TCPAlgorithm as well -- it may want to recognized additional options
            default:
                tcpEV << "ERROR: Unsupported option received, kind=" << kind << "\n";
                break;
        }
        (void)ok; // unused
    }
}

bool TCPConnection::processMSSOption(TCPSegment *tcpseg, const TCPOption& option)
{
    if (option.getLength() != 4)
    {
        tcpEV << "ERROR: MSS option length incorrect\n";
        return false;
    }

    if (option.getValuesArraySize() == 0)
    {
        tcpEV << "ERROR: TCP Header Option MSS received, but no SMSS value present\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT)
    {
        tcpEV << "ERROR: TCP Header Option MSS received, but in unexpected state\n";
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
    state->snd_mss = std::min(state->snd_mss, (uint32) option.getValues(0));
    if (state->snd_mss==0)
        state->snd_mss = 536;
    tcpEV << "TCP Header Option MSS received, SMSS is set to: " << state->snd_mss << "\n";
    return true;
}

bool TCPConnection::processSACKPermittedOption(TCPSegment *tcpseg, const TCPOption& option)
{
    if (option.getLength() != 2)
    {
        tcpEV << "ERROR: SACK_PERMITTED option length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT)
    {
        tcpEV << "ERROR: TCP Header Option SACK_PERMITTED received, but in unexpected state\n";
        return false;
    }

    state->rcv_sack_perm = true;
    state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
    tcpEV << "TCP Header Option SACK_PERMITTED received, SACK (sack_enabled) is set to: " << state->sack_enabled << "\n";
    return true;
}

bool TCPConnection::processSACKOption(TCPSegment *tcpseg, const TCPOption& option)
{
    if (option.getLength() % 8 != 2)
    {
        tcpEV << "ERROR: SACK option length incorrect\n";
        return false;
    }

    if (!state->sack_enabled) // not need to check the state here?
    {
        tcpEV << "ERROR: " << (option.getLength()/2) << ". SACK(s) received, but sack_enabled is set to " << state->sack_enabled << "\n";
        return false;
    }

    // temporary variable
    int n = option.getValuesArraySize()/2;
    if (n > 0) // sacks present?
    {
        tcpEV << n << " SACK(s) received:\n";
        uint count=0;
        for (int i=0; i<n; i++)
        {
            Sack tmp;
            tmp.setStart(option.getValues(count));
            count++;
            tmp.setEnd(option.getValues(count));
            count++;

            uint32 sack_range = tmp.getEnd() - tmp.getStart();
            tcpEV << (i+1) << ". SACK:" << " [" << tmp.getStart() << ".." << tmp.getEnd() << ")\n";

            // check for D-SACK
            if (i==0 && seqLess(tmp.getEnd(), tcpseg->getAckNo()))
            {
                // RFC 2883, page 8:
                // "In order for the sender to check that the first (D)SACK block of an
                // acknowledgement in fact acknowledges duplicate data, the sender
                // should compare the sequence space in the first SACK block to the
                // cumulative ACK which is carried IN THE SAME PACKET.  If the SACK
                // sequence space is less than this cumulative ACK, it is an indication
                // that the segment identified by the SACK block has been received more
                // than once by the receiver.  An implementation MUST NOT compare the
                // sequence space in the SACK block to the TCP state variable snd.una
                // (which carries the total cumulative ACK), as this may result in the
                // wrong conclusion if ACK packets are reordered."
                tcpEV << "Received D-SACK below cumulative ACK=" << tcpseg->getAckNo() << " D-SACK:" << " [" << tmp.getStart() << ".." << tmp.getEnd() << ")\n";
            }
            else if (i==0 && seqGE(tmp.getEnd(), tcpseg->getAckNo()) && n>1)
            {
                // RFC 2883, page 8:
                // "If the sequence space in the first SACK block is greater than the
                // cumulative ACK, then the sender next compares the sequence space in
                // the first SACK block with the sequence space in the second SACK
                // block, if there is one.  This comparison can determine if the first
                // SACK block is reporting duplicate data that lies above the cumulative
                // ACK."
                Sack tmp2;
                tmp2.setStart(option.getValues(2));
                tmp2.setEnd(option.getValues(3));

                if (seqGE(tmp.getStart(), tmp2.getStart()) && seqLE(tmp.getEnd(), tmp2.getEnd()))
                {tcpEV << "Received D-SACK above cumulative ACK=" << tcpseg->getAckNo() << " D-SACK:" << " [" << tmp.getStart() << ".." << tmp.getEnd() << ") SACK:" << " [" << tmp2.getStart() << ".." << tmp2.getEnd() << ")\n";}
            }

            // splitt sack_range to smss_sized pieces
            uint32 tmp_sack_range = sack_range;
            uint32 counter = 1; // at least one piece has been received

            while (tmp_sack_range > state->snd_mss) // to check how many smss_sized pieces are covered by this single sack_range
            {
                tmp_sack_range = tmp_sack_range - state->snd_mss;
                counter++;
            }

            // find smss_sized_sack in send queue and set "sacked" bit
            uint32 mss_sized_sack = tmp.getStart();
            for (uint j=0;j<counter;j++)
            {
                if (j+1==counter) // the last part does not need to be smss_sized (nagle off)
                {
                    if (seqGE(mss_sized_sack,state->snd_una))
                        rexmitQueue->setSackedBit(mss_sized_sack,mss_sized_sack+tmp_sack_range);
                    else
                        tcpEV << "Received old sack. Sacked segment number is below snd_una\n";
                }
                else
                {
                    if (seqGE(mss_sized_sack,state->snd_una))
                        rexmitQueue->setSackedBit(mss_sized_sack,mss_sized_sack+state->snd_mss);
                    else
                        tcpEV << "Received old sack. Sacked segment number is below snd_una\n";
                }
                mss_sized_sack = mss_sized_sack + state->snd_mss;
            }
        }
        state->rcv_sacks = state->rcv_sacks + n; // total counter, no current number
        if (rcvSacksVector)
            rcvSacksVector->record(state->rcv_sacks);

        // update scoreboard
        state->sackedBytes_old = state->sackedBytes; // needed for RFC 3042 to check if last dupAck contained new sack information
        state->sackedBytes = rexmitQueue->getTotalAmountOfSackedBytes();
        if (sackedBytesVector)
            sackedBytesVector->record(state->sackedBytes);
    }
    return true;
}

TCPSegment TCPConnection::writeHeaderOptions(TCPSegment *tcpseg)
{
    TCPOption option;
    int t = 0;

    if (tcpseg->getSynBit() && (fsm.getState() == TCP_S_INIT || fsm.getState() == TCP_S_LISTEN || ((fsm.getState()==TCP_S_SYN_SENT || fsm.getState()==TCP_S_SYN_RCVD) && state->syn_rexmit_count>0))) // SYN flag set and connetion in INIT or LISTEN state (or after synRexmit timeout)
    {
        // MSS header option
        if (state->snd_mss > 0)
        {
            option.setKind(TCPOPTION_MAXIMUM_SEGMENT_SIZE); // MSS
            option.setLength(4);
            option.setValuesArraySize(1);

            // Update MSS
            option.setValues(0,state->snd_mss);
            tcpEV << "TCP Header Option MSS(=" << state->snd_mss << ") sent\n";
            tcpseg->setOptionsArraySize(tcpseg->getOptionsArraySize()+1);
            tcpseg->setOptions(t,option);
            t++;
        }

        // SACK_PERMITTED header option
        if (state->sack_support) // Is SACK supported by host?
        {
            // 2 padding bytes
            option.setKind(TCPOPTION_NO_OPERATION); // NOP
            option.setLength(1);
            option.setValuesArraySize(0);
            tcpseg->setOptionsArraySize(tcpseg->getOptionsArraySize()+2);
            tcpseg->setOptions(t,option);
            t++;
            tcpseg->setOptions(t,option);
            t++;

            option.setKind(TCPOPTION_SACK_PERMITTED);
            option.setLength(2);
            option.setValuesArraySize(0);

            // Update SACK variable
            state->snd_sack_perm = true;
            state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
            tcpEV << "TCP Header Option SACK_PERMITTED sent, SACK (sack_enabled) is set to: " << state->sack_enabled << "\n";
            tcpseg->setOptionsArraySize(tcpseg->getOptionsArraySize()+1);
            tcpseg->setOptions(t,option);
            t++;
        }

        // TODO add new TCPOptions here once they are implemented
    }
    else if (fsm.getState()==TCP_S_SYN_RCVD || fsm.getState()==TCP_S_ESTABLISHED || fsm.getState()==TCP_S_FIN_WAIT_1 || fsm.getState()==TCP_S_FIN_WAIT_2) // connetion is not in INIT or LISTEN state
    {
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
        if (state->sack_enabled && (state->snd_sack || state->snd_dsack))
        {
            // 2 padding bytes
            option.setKind(TCPOPTION_NO_OPERATION); // NOP
            option.setLength(1);
            option.setValuesArraySize(0);
            tcpseg->setOptionsArraySize(tcpseg->getOptionsArraySize()+2);
            tcpseg->setOptions(t,option);
            t++;
            tcpseg->setOptions(t,option);
            t++;

            addSacks(tcpseg);
            t++;
        }

        // TODO add new TCPOptions here once they are implemented

        // TODO delegate to TCPAlgorithm as well -- it may want to append additional options
    }

    if (tcpseg->getOptionsArraySize() != 0)
    {
        uint options_len = 0;
        for (uint i=0; i<tcpseg->getOptionsArraySize(); i++)
            options_len = options_len + tcpseg->getOptions(i).getLength();

        if (options_len <= 40) // Options length allowed? - maximum: 40 Bytes
            tcpseg->setHeaderLength(TCP_HEADER_OCTETS+options_len); // TCP_HEADER_OCTETS = 20
        else
        {
            tcpseg->setHeaderLength(TCP_HEADER_OCTETS); // TCP_HEADER_OCTETS = 20
            tcpseg->setOptionsArraySize(0); // drop all options
            tcpEV << "ERROR: Options length exceeded! Segment will be sent without options" << "\n";
        }
    }

    return *tcpseg;
}

TCPSegment TCPConnection::addSacks(TCPSegment *tcpseg)
{
    TCPOption option;
    uint options_len = 0;
    uint used_options_len = 0;
    uint m = 0; // number of sack blocks to be sent in current segment
    uint n = 0; // number of sack blocks in sacks_array before sending current segment
    bool skip_sacks_array = false; // set if dsack is subsets of a bigger sack block recently reported
    bool overlap = false; // set if recently reported sack blocks are subsets of "sacks_array[0]"

    uint32 start = state->start_seqno;
    uint32 end = state->end_seqno;

    ASSERT(start!=0 || end!=0);

    // delete old sacks (below rcv_nxt), delete duplicates and print previous status of sacks_array:
    tcpEV << "Previous status of sacks_array: \n";
    for (int a=0; a<MAX_SACK_BLOCKS; a++) // MAX_SACK_BLOCKS is set to 60
    {
        if (state->sacks_array[a].getStart()!=0 && seqLE(state->sacks_array[a].getEnd(), state->rcv_nxt))
        {
            state->sacks_array[a].setStart(0);
            state->sacks_array[a].setEnd(0);
        }
        if (state->sacks_array[a].getStart()!=0 && state->sacks_array[a].getEnd()!=0) // do not print empty entries
            tcpEV << "\t" << (a+1) << ". SACK in sacks_array:" << " [" << state->sacks_array[a].getStart() << ".." << state->sacks_array[a].getEnd() << ")\n";
        else
            break;
    }

    for (uint a=0; a<MAX_SACK_BLOCKS-1; a++)
    {
        if (state->sacks_array[a].getStart() != 0)
            m++;
        else
            break;
    }
    n = m + 1; // +1 for new the new sack block

    // 2 padding bytes are prefixed
    if (tcpseg->getOptionsArraySize()>0)
    {
        for (uint i=0; i<tcpseg->getOptionsArraySize(); i++)
            used_options_len = used_options_len + tcpseg->getOptions(i).getLength();
        if (used_options_len>30)
        {
            tcpEV << "ERROR: Failed to addSacks - at least 10 free bytes needed for SACK - used_options_len=" << used_options_len << "\n";
            //reset flags:
            skip_sacks_array = false;
            state->snd_sack  = false;
            state->snd_dsack = false;
            state->start_seqno = 0;
            state->end_seqno = 0;
            return *tcpseg;
        }
        else
        {
            n = std::min (n, (((40-used_options_len)-2)/8));
            option.setValuesArraySize(n*2);
        }
    }
    else
    {
        n = std::min (n, MAX_SACK_ENTRIES);
        option.setValuesArraySize(n*2);
    }

    // before adding a new sack move old sacks by one to the right
    for (int a=(MAX_SACK_BLOCKS-1); a>=0; a--) // MAX_SACK_BLOCKS is set to 60
        state->sacks_array[a+1] = state->sacks_array[a];

    if (state->snd_dsack) // SequenceNo < rcv_nxt
    {
        // RFC 2883, page 3:
        // "(3) The left edge of the D-SACK block specifies the first sequence
        // number of the duplicate contiguous sequence, and the right edge of
        // the D-SACK block specifies the sequence number immediately following
        // the last sequence in the duplicate contiguous sequence."
        if (seqLess(start, state->rcv_nxt) && seqLess(state->rcv_nxt, end))
            end = state->rcv_nxt;
    }
    else if (start==0 && end==0) // rcv_nxt_old != rcv_nxt
    {
        // RFC 2018, page 4:
        // "* The first SACK block (i.e., the one immediately following the
        // kind and length fields in the option) MUST specify the contiguous
        // block of data containing the segment which triggered this ACK,
        // unless that segment advanced the Acknowledgment Number field in
        // the header.  This assures that the ACK with the SACK option
        // reflects the most recent change in the data receiver's buffer
        // queue."
        start = state->sacks_array[0].getStart();
        end = state->sacks_array[0].getEnd();
    }
    else // rcv_nxt_old == rcv_nxt or end <= rcv_nxt
    {
        // RFC 2018, page 4:
        // "* The first SACK block (i.e., the one immediately following the
        // kind and length fields in the option) MUST specify the contiguous
        // block of data containing the segment which triggered this ACK,"
        start = receiveQueue->getLE(start);
        end = receiveQueue->getRE(end);
    }

    state->sacks_array[0].setStart(start);
    state->sacks_array[0].setEnd(end);

    // RFC 2883, page 3:
    // "(4) If the D-SACK block reports a duplicate contiguous sequence from
    // a (possibly larger) block of data in the receiver's data queue above
    // the cumulative acknowledgement, then the second SACK block in that
    // SACK option should specify that (possibly larger) block of data.
    //
    // (5) Following the SACK blocks described above for reporting duplicate
    // segments, additional SACK blocks can be used for reporting additional
    // blocks of data, as specified in RFC 2018."
    if (state->snd_dsack)
    {
        uint32 start_new = receiveQueue->getLE(start);
        uint32 end_new = receiveQueue->getRE(end);
        if (start_new != start || end_new != end)
        {
            skip_sacks_array = true;
            for (int a=(MAX_SACK_BLOCKS-1); a>=1; a--) // MAX_SACK_BLOCKS is set to 60
                state->sacks_array[a+1] = state->sacks_array[a];
            state->sacks_array[1].setStart(start_new); // specifies larger block of data
            state->sacks_array[1].setEnd(end_new);     // specifies larger block of data
        }
    }

    // RFC 2018, page 4:
    // "* The SACK option SHOULD be filled out by repeating the most
    // recently reported SACK blocks (based on first SACK blocks in
    // previous SACK options) that are not subsets of a SACK block
    // already included in the SACK option being constructed."

    // check if recently reported SACK blocks are subsets of "sacks_array[0]"
    for (uint a=0; a<MAX_SACK_BLOCKS-1; a++)
    {
        uint i = 1;
        bool matched = false;

        if (a==0 && skip_sacks_array)
            a = 1;

        if (state->sacks_array[a+i].getStart() == 0)
            break;

        while ((state->sacks_array[a].getStart() == state->sacks_array[a+i].getStart() ||
            state->sacks_array[a].getEnd() == state->sacks_array[a+i].getStart() ||
            state->sacks_array[a].getEnd() == state->sacks_array[a+i].getEnd())
            && a+i < MAX_SACK_BLOCKS && state->sacks_array[a].getStart()!=0) // MAX_SACK_BLOCKS is set to 60
        {
            matched = true;
            i++;
            overlap = true;
        }
        if (matched)
            state->sacks_array[a+1] = state->sacks_array[a+i];
    }

    if (!skip_sacks_array && overlap && m<4)
        n--;

    option.setKind(TCPOPTION_SACK);
    option.setLength(8*n+2);
    option.setValuesArraySize(2*n);

    // write sacks from sacks_array to options
    uint counter = 0;
    for (uint a=0; a<n; a++)
    {
        option.setValues(counter,state->sacks_array[a].getStart());
        counter++;
        option.setValues(counter,state->sacks_array[a].getEnd());
        counter++;
    }

    // independent of "n" we always need 2 padding bytes (NOP) to make: (used_options_len % 4 == 0)
    options_len = used_options_len + 8*n + 2; // 8 bytes for each SACK (n) + 2 bytes for kind&length

    if (options_len <= 40) // Options length allowed? - maximum: 40 Bytes
    {
        tcpseg->setOptionsArraySize(tcpseg->getOptionsArraySize()+1);
        tcpseg->setOptions((tcpseg->getOptionsArraySize()-1),option);

        // update number of sent sacks
        state->snd_sacks = state->snd_sacks+n;
        if (sndSacksVector)
            sndSacksVector->record(state->snd_sacks);

        uint counter = 0;
        tcpEV << n << " SACK(s) added to header:\n";
        for (uint t=0; t<(n*2); t++)
        {
            counter++;
            tcpEV << counter << ". SACK:" << " [" << option.getValues(t);
            t++;
            tcpEV << ".." << option.getValues(t) << ")";
            if (t==1)
            {
                if (state->snd_dsack)
                    tcpEV << " (D-SACK)";
                else if (seqLE(option.getValues(t),state->rcv_nxt))
                {
                    tcpEV << " (received segment filled out a gap)";
                    state->snd_dsack = true; // Note: Set snd_dsack to delete first sack from sacks_array
                }
            }
            tcpEV << "\n";
        }
    }
    else
        tcpEV << "ERROR: Option length exceeded! Segment will be sent without SACK(s)" << "\n";

    // RFC 2883, page 3:
    // "(1) A D-SACK block is only used to report a duplicate contiguous
    // sequence of data received by the receiver in the most recent packet.
    //
    // (2) Each duplicate contiguous sequence of data received is reported
    // in at most one D-SACK block.  (I.e., the receiver sends two identical
    // D-SACK blocks in subsequent packets only if the receiver receives two
    // duplicate segments.)//
    //
    // In case of d-sack: delete first sack (d-sack) and move old sacks by one to the left
    if (state->snd_dsack)
    {
        for (int a=1; a<MAX_SACK_BLOCKS; a++) // MAX_SACK_BLOCKS is set to 60
            state->sacks_array[a-1] = state->sacks_array[a];

        // delete/reset last sack to avoid duplicates
        state->sacks_array[MAX_SACK_BLOCKS-1].setStart(0);
        state->sacks_array[MAX_SACK_BLOCKS-1].setEnd(0);
    }

    // reset flags:
    skip_sacks_array = false;
    state->snd_sack  = false;
    state->snd_dsack = false;
    state->start_seqno = 0;
    state->end_seqno = 0;

    return *tcpseg;
}

void TCPConnection::updateRcvQueueVars()
{
    // update receive queue related state variables
    state->freeRcvBuffer = receiveQueue->getAmountOfFreeBytes(state->maxRcvBuffer);
    state->usedRcvBuffer = state->maxRcvBuffer - state->freeRcvBuffer;

    // update receive queue related statistics
    if (tcpRcvQueueBytesVector)
        tcpRcvQueueBytesVector->record(state->usedRcvBuffer);

    tcpEV << "receiveQ: receiveQLength=" << receiveQueue->getQueueLength() << " maxRcvBuffer=" << state->maxRcvBuffer << " usedRcvBuffer=" << state->usedRcvBuffer << " freeRcvBuffer=" << state->freeRcvBuffer << "\n";
}

void TCPConnection::updateRcvWnd()
{
    uint32 win = 0;

    // update receive queue related state variables and statistics
    updateRcvQueueVars();
    win = state->freeRcvBuffer;

    // Following lines are based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, pages 878-879]:
    // Don't advertise less than one full-sized segment to avoid SWS
    if (win < (state->maxRcvBuffer / 4) && win < state->snd_mss)
        win = 0;

    // Do not shrink window
    // (rcv_adv minus rcv_nxt) is the amount of space still available to the sender that was previously advertised
    if (win < state->rcv_adv - state->rcv_nxt)
        win = state->rcv_adv - state->rcv_nxt;

    // Observe upper limit for advertised window on this connection
    if (win > TCP_MAX_WIN) // TCP_MAX_WIN = 65535 (16 bit)
        win = TCP_MAX_WIN; // Note: The window size is limited to a 16 bit value in the TCP header.
    // Note: The order of the "Do not shrink window" and "Observe upper limit" parts has been changed to the order used in FreeBSD Release 7.1

    // update rcv_adv if needed
    if (win > 0 && seqGE(state->rcv_nxt + win, state->rcv_adv))
    {
        state->rcv_adv = state->rcv_nxt + win;
        if (rcvAdvVector)
            rcvAdvVector->record(state->rcv_adv);
    }

    state->rcv_wnd = win;
}

void TCPConnection::updateWndInfo(TCPSegment *tcpseg)
{
    // Following lines are based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 982]:
    if (tcpseg->getAckBit()
        && (seqLess(state->snd_wl1, tcpseg->getSequenceNo()) ||
        (state->snd_wl1 == tcpseg->getSequenceNo()&& seqLE(state->snd_wl2, tcpseg->getAckNo())) ||
        (state->snd_wl2 == tcpseg->getAckNo()&& tcpseg->getWindow() > state->snd_wnd)))
    {
        // send window should be updated
        tcpEV << "Updating send window from segment: new wnd=" << tcpseg->getWindow() << "\n";
        state->snd_wnd = tcpseg->getWindow();
        state->snd_wl1 = tcpseg->getSequenceNo();
        state->snd_wl2 = tcpseg->getAckNo();
        if (sndWndVector)
            sndWndVector->record(state->snd_wnd);
    }
}

bool TCPConnection::isLost(uint32 seqNum)
{
    ASSERT (state->sack_enabled);
    // RFC 3517, page 3: "This routine returns whether the given sequence number is
    // considered to be lost.  The routine returns true when either
    // DupThresh discontiguous SACKed sequences have arrived above
    // 'SeqNum' or (DupThresh * SMSS) bytes with sequence numbers greater
    // than 'SeqNum' have been SACKed.  Otherwise, the routine returns
    // false."
    bool isLost = false;

    ASSERT(seqGE(seqNum,state->snd_una)); // HighAck = snd_una

    if (rexmitQueue->getNumOfDiscontiguousSacks(seqNum) >= DUPTHRESH ||     // DUPTHRESH = 3
        rexmitQueue->getAmountOfSackedBytes(seqNum) >= (DUPTHRESH * state->snd_mss))
        isLost = true;
    else
        isLost = false;

    return isLost;
}

void TCPConnection::setPipe()
{
    ASSERT (state->sack_enabled);
    // RFC 3517, pages 1 and 2: "
    // "HighACK" is the sequence number of the highest byte of data that
    // has been cumulatively ACKed at a given point.
    //
    // "HighData" is the highest sequence number transmitted at a given
    // point.
    //
    // "HighRxt" is the highest sequence number which has been
    // retransmitted during the current loss recovery phase.
    //
    // "Pipe" is a sender's estimate of the number of bytes outstanding
    // in the network.  This is used during recovery for limiting the
    // sender's sending rate.  The pipe variable allows TCP to use a
    // fundamentally different congestion control than specified in
    // [RFC2581].  The algorithm is often referred to as the "pipe
    // algorithm"."
    // HighAck = snd_una
    // HighData = snd_max

    state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
    state->pipe = 0;

    // RFC 3517, page 3: "This routine traverses the sequence space from HighACK to HighData
    // and MUST set the "pipe" variable to an estimate of the number of
    // octets that are currently in transit between the TCP sender and
    // the TCP receiver.  After initializing pipe to zero the following
    // steps are taken for each octet 'S1' in the sequence space between
    // HighACK and HighData that has not been SACKed:"
    for (uint32 s1=state->snd_una; s1<state->snd_max; s1=s1+state->snd_mss) // Note: Would be better use byte ranges
    {
        if (rexmitQueue->getSackedBit(s1)==false)
        {
            // RFC 3517, page 3: "(a) If IsLost (S1) returns false:
            //
            //     Pipe is incremented by 1 octet.
            //
            //     The effect of this condition is that pipe is incremented for
            //     packets that have not been SACKed and have not been determined
            //     to have been lost (i.e., those segments that are still assumed
            //     to be in the network)."
            if (isLost(s1)==false)
                state->pipe++;

            // RFC 3517, pages 3 and 4: "(b) If S1 <= HighRxt:
            //
            //     Pipe is incremented by 1 octet.
            //
            //     The effect of this condition is that pipe is incremented for
            //     the retransmission of the octet.
            //
            //  Note that octets retransmitted without being considered lost are
            //  counted twice by the above mechanism."
            if (seqLE(s1,state->highRxt))
                state->pipe++;
        }
    }

    state->pipe = state->pipe * state->snd_mss; // TODO - remove this line if using byte ranges
    if (pipeVector)
        pipeVector->record(state->pipe);
}

uint32 TCPConnection::nextSeg()
{
    ASSERT (state->sack_enabled);
    // RFC 3517, page 5: "This routine uses the scoreboard data structure maintained by the
    // Update() function to determine what to transmit based on the SACK
    // information that has arrived from the data receiver (and hence
    // been marked in the scoreboard).  NextSeg () MUST return the
    // sequence number range of the next segment that is to be
    // transmitted, per the following rules:"

    state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
    uint32 seqNum = 0;
    bool found = false;

    // RFC 3517, page 5: "(1) If there exists a smallest unSACKed sequence number 'S2' that
    // meets the following three criteria for determining loss, the
    // sequence range of one segment of up to SMSS octets starting
    // with S2 MUST be returned.
    //
    // (1.a) S2 is greater than HighRxt.
    //
    // (1.b) S2 is less than the highest octet covered by any
    //       received SACK.
    //
    // (1.c) IsLost (S2) returns true."
    for (uint32 s2=state->snd_una; s2<state->snd_max; s2=s2+state->snd_mss) // Note: Would be better use byte ranges
    {
        if (rexmitQueue->getSackedBit(s2)==false)
        {
            if (seqGE(s2,state->highRxt) &&
                seqLE(s2,(rexmitQueue->getHighestSackedSeqNum())) &&
                isLost(s2))
            {
                seqNum = s2;
                found = true;
                return seqNum;
            }
        }
    }

    // RFC 3517, page 5: "(2) If no sequence number 'S2' per rule (1) exists but there
    // exists available unsent data and the receiver's advertised
    // window allows, the sequence range of one segment of up to SMSS
    // octets of previously unsent data starting with sequence number
    // HighData+1 MUST be returned."
    if (!found)
    {
        // check how many unsent bytes we have
        ulong buffered = sendQueue->getBytesAvailable(state->snd_max);
        ulong maxWindow = state->snd_wnd;
        // effectiveWindow: number of bytes we're allowed to send now
        ulong effectiveWin = maxWindow - state->pipe;
        if (buffered > 0 && effectiveWin >= state->snd_mss)
        {
            seqNum = state->snd_max; // HighData = snd_max
            found = true;
            return seqNum;
        }
    }

    // RFC 3517, pages 5 and 6: "(3) If the conditions for rules (1) and (2) fail, but there exists
    // an unSACKed sequence number 'S3' that meets the criteria for
    // detecting loss given in steps (1.a) and (1.b) above
    // (specifically excluding step (1.c)) then one segment of up to
    // SMSS octets starting with S3 MAY be returned.
    //
    // Note that rule (3) is a sort of retransmission "last resort".
    // It allows for retransmission of sequence numbers even when the
    // sender has less certainty a segment has been lost than as with
    // rule (1).  Retransmitting segments via rule (3) will help
    // sustain TCP's ACK clock and therefore can potentially help
    // avoid retransmission timeouts.  However, in sending these
    // segments the sender has two copies of the same data considered
    // to be in the network (and also in the Pipe estimate).  When an
    // ACK or SACK arrives covering this retransmitted segment, the
    // sender cannot be sure exactly how much data left the network
    // (one of the two transmissions of the packet or both
    // transmissions of the packet).  Therefore the sender may
    // underestimate Pipe by considering both segments to have left
    // the network when it is possible that only one of the two has.
    //
    // We believe that the triggering of rule (3) will be rare and
    // that the implications are likely limited to corner cases
    // relative to the entire recovery algorithm.  Therefore we leave
    // the decision of whether or not to use rule (3) to
    // implementors."
    if (!found)
    {
        for (uint32 s3=state->snd_una; s3<state->snd_max; s3=s3+state->snd_mss) // Note: Would be better use byte ranges
        {
            if (rexmitQueue->getSackedBit(s3)==false)
            {
                if (seqGE(s3,state->highRxt) &&
                    seqLE(s3,(rexmitQueue->getHighestSackedSeqNum())))
                {
                    seqNum = s3;
                    found = true;
                    return seqNum;
                }
            }
        }
    }

    // RFC 3517, page 6: "(4) If the conditions for each of (1), (2), and (3) are not met,
    // then NextSeg () MUST indicate failure, and no segment is
    // returned."
    if (!found)
        seqNum = 0;

    return seqNum;
}

void TCPConnection::sendDataDuringLossRecoveryPhase(uint32 congestionWindow)
{
    ASSERT (state->sack_enabled && state->lossRecovery);
    // RFC 3517 pages 7 and 8: "(5) In order to take advantage of potential additional available
    // cwnd, proceed to step (C) below.
    // (...)
    // (C) If cwnd - pipe >= 1 SMSS the sender SHOULD transmit one or more
    // segments as follows:
    // (...)
    // (C.5) If cwnd - pipe >= 1 SMSS, return to (C.1)"
    while (((int)congestionWindow - (int)state->pipe) >= (int)state->snd_mss) // Note: Typecast needed to avoid prohibited transmissions
    {
        // RFC 3517 pages 7 and 8: "(C.1) The scoreboard MUST be queried via NextSeg () for the
        // sequence number range of the next segment to transmit (if any),
        // and the given segment sent.  If NextSeg () returns failure (no
        // data to send) return without sending anything (i.e., terminate
        // steps C.1 -- C.5)."
        uint32 seqNum = nextSeg(); // if nextSeg() returns 0 (=failure): terminate steps C.1 -- C.5
        if (seqNum != 0)
        {
            sendSegmentDuringLossRecoveryPhase(seqNum);
            // RFC 3517 page 8: "(C.4) The estimate of the amount of data outstanding in the
            // network must be updated by incrementing pipe by the number of
            // octets transmitted in (C.1)."
            state->pipe = state->pipe + state->snd_mss;
        }
        else // nextSeg () returns failure: terminate steps C.1 -- C.5
            break;
    }
}

void TCPConnection::sendSegmentDuringLossRecoveryPhase(uint32 seqNum)
{
    ASSERT (state->sack_enabled && state->lossRecovery);
    // start sending from seqNum
    state->snd_nxt = seqNum;

    uint32 old_highRxt = rexmitQueue->getHighestRexmittedSeqNum();

    // no need to check cwnd and rwnd - has already be done before
    // no need to check nagle - sending mss bytes
    sendSegment(state->snd_mss);

    uint32 sentSeqNum = seqNum + state->snd_mss;

    // RFC 3517 page 8: "(C.2) If any of the data octets sent in (C.1) are below HighData,
    // HighRxt MUST be set to the highest sequence number of the
    // retransmitted segment."
    if (seqLE(sentSeqNum, state->snd_max)) // HighData = snd_max
    {
        ASSERT (sentSeqNum==rexmitQueue->getHighestRexmittedSeqNum());
        state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
    }
    // RFC 3517 page 8: "(C.3) If any of the data octets sent in (C.1) are above HighData,
    // HighData must be updated to reflect the transmission of
    // previously unsent data."
    else if (seqGE(sentSeqNum, state->snd_max)) // HighData = snd_max
        state->snd_max = sentSeqNum;

    if (unackedVector)
        unackedVector->record(state->snd_max - state->snd_una);

    // RFC 3517, page 9: "6   Managing the RTO Timer
    //
    // The standard TCP RTO estimator is defined in [RFC2988].  Due to the
    // fact that the SACK algorithm in this document can have an impact on
    // the behavior of the estimator, implementers may wish to consider how
    // the timer is managed.  [RFC2988] calls for the RTO timer to be
    // re-armed each time an ACK arrives that advances the cumulative ACK
    // point.  Because the algorithm presented in this document can keep the
    // ACK clock going through a fairly significant loss event,
    // (comparatively longer than the algorithm described in [RFC2581]), on
    // some networks the loss event could last longer than the RTO.  In this
    // case the RTO timer would expire prematurely and a segment that need
    // not be retransmitted would be resent.
    //
    // Therefore we give implementers the latitude to use the standard
    // [RFC2988] style RTO management or, optionally, a more careful variant
    // that re-arms the RTO timer on each retransmission that is sent during
    // recovery MAY be used.  This provides a more conservative timer than
    // specified in [RFC2988], and so may not always be an attractive
    // alternative.  However, in some cases it may prevent needless
    // retransmissions, go-back-N transmission and further reduction of the
    // congestion window."
    tcpAlgorithm->ackSent();
    if (old_highRxt != state->highRxt)
    {
        // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
        tcpEV << "Retransmission sent during recovery, restarting REXMIT timer.\n";
        tcpAlgorithm->restartRexmitTimer();
    }
    else // don't measure RTT for retransmitted packets
        tcpAlgorithm->dataSent(seqNum); // seqNum = old_snd_nxt
}

void TCPConnection::sendOneNewSegment(bool fullSegmentsOnly, uint32 congestionWindow)
{
    ASSERT (state->limited_transmit_enabled);
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
    if (!state->sack_enabled || (state->sack_enabled && state->sackedBytes_old!=state->sackedBytes))
    {
        // check how many bytes we have
        ulong buffered = sendQueue->getBytesAvailable(state->snd_max);

        if (buffered >= state->snd_mss || (!fullSegmentsOnly && buffered > 0))
        {
            ulong outstandingData = state->snd_max - state->snd_una;
            // check conditions from RFC 3042
            if (outstandingData + state->snd_mss <= state->snd_wnd &&
                outstandingData + state->snd_mss <= congestionWindow + 2*state->snd_mss)
            {
                uint32 effectiveWin = std::min (state->snd_wnd, congestionWindow) - outstandingData + 2*state->snd_mss; // RFC 3042, page 3: "(...)the sender can only send two segments beyond the congestion window (cwnd)."
                // bytes: number of bytes we're allowed to send now
                uint32 bytes = std::min(effectiveWin, state->snd_mss);
                if (bytes >= state->snd_mss || (!fullSegmentsOnly && bytes > 0))
                {
                    uint32 old_snd_nxt = state->snd_nxt;
                    // we'll start sending from snd_max
                    state->snd_nxt = state->snd_max;

                    tcpEV << "Limited Transmit algorithm enabled. Sending one new segment.\n";
                    sendSegment(bytes);

                    state->snd_max = std::max (state->snd_nxt, state->snd_max);

                    if (unackedVector)
                        unackedVector->record(state->snd_max - state->snd_una);

                    // reset snd_nxt if needed
                    if (state->afterRto)
                        state->snd_nxt = old_snd_nxt + bytes;

                    // notify
                    tcpAlgorithm->ackSent();
                    tcpAlgorithm->dataSent(old_snd_nxt);
                }
            }
        }
    }
}
