//
// Copyright (C) 2004 Andras Varga
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "TCPConnection.h"
#include "TCPSegment_m.h"
#include "TCPInterfacePacket_m.h"
#include "IPInterfacePacket.h"


TCPConnection::TCPConnection(int _connId, cSimpleModule *_mod)
{
    connId = _connId;
    module = _mod;
}

bool TCPConnection::processTimer(cMessage *msg)
{
    // first do actions
    switch (fsm.state())
    {
        case TCP_S_INIT:         processTimerInInit(msg); break;
        case TCP_S_CLOSED:       processTimerInClosed(msg); break;
        case TCP_S_LISTEN:       processTimerInListen(msg); break;
        case TCP_S_SYN_SENT:     processTimerInSynRcvd(msg); break;
        case TCP_S_SYN_RCVD:     processTimerInSynSent(msg); break;
        case TCP_S_ESTABLISHED:  processTimerInEstablished(msg); break;
        case TCP_S_CLOSE_WAIT:   processTimerInCloseWait(msg); break;
        case TCP_S_LAST_ACK:     processTimerInLastAck(msg); break;
        case TCP_S_FIN_WAIT_1:   processTimerInFinWait1(msg); break;
        case TCP_S_FIN_WAIT_2:   processTimerInFinWait2(msg); break;
        case TCP_S_CLOSING:      processTimerInClosing(msg); break;
        case TCP_S_TIME_WAIT:    processTimerInTimeWait(msg); break;
        default: opp_error("wrong TCP state");
    }

    // then state transitions
    TCPEvent event = analyseTimerEvent(msg);
    return performStateTransition(event);
}

bool TCPConnection::processTCPSegment(IPInterfacePacket *wrappedTcpSeg)
{
    // unwrap TCP segment
    TCPSegment *tcpseg = (TCPSegment *) wrappedTcpSeg->decapsulate();
    if (localAddr.isNull())
    {
        localAddr = wrappedTcpSeg->destAddr();
        remoteAddr = wrappedTcpSeg->srcAddr();
    }
    else
    {
        ASSERT(localAddr==wrappedTcpSeg->destAddr());
        ASSERT(remoteAddr==wrappedTcpSeg->srcAddr());
    }
    delete wrappedTcpSeg;

    // first do actions
    switch (fsm.state())
    {
        case TCP_S_INIT:         processTCPSegmentInInit(tcpseg); break;
        case TCP_S_CLOSED:       processTCPSegmentInClosed(tcpseg); break;
        case TCP_S_LISTEN:       processTCPSegmentInListen(tcpseg); break;
        case TCP_S_SYN_SENT:     processTCPSegmentInSynRcvd(tcpseg); break;
        case TCP_S_SYN_RCVD:     processTCPSegmentInSynSent(tcpseg); break;
        case TCP_S_ESTABLISHED:  processTCPSegmentInEstablished(tcpseg); break;
        case TCP_S_CLOSE_WAIT:   processTCPSegmentInCloseWait(tcpseg); break;
        case TCP_S_LAST_ACK:     processTCPSegmentInLastAck(tcpseg); break;
        case TCP_S_FIN_WAIT_1:   processTCPSegmentInFinWait1(tcpseg); break;
        case TCP_S_FIN_WAIT_2:   processTCPSegmentInFinWait2(tcpseg); break;
        case TCP_S_CLOSING:      processTCPSegmentInClosing(tcpseg); break;
        case TCP_S_TIME_WAIT:    processTCPSegmentInTimeWait(tcpseg); break;
        default: opp_error("wrong TCP state");
    }

    // then state transitions
    TCPEvent event = analyseTCPSegmentEvent(tcpseg);
    return performStateTransition(event);
}

bool TCPConnection::processAppCommand(TCPInterfacePacket *tcpIfPacket)
{
    // first do actions
    switch (fsm.state())
    {
        case TCP_S_INIT:         processAppCommandInInit(tcpIfPacket); break;
        case TCP_S_CLOSED:       processAppCommandInClosed(tcpIfPacket); break;
        case TCP_S_LISTEN:       processAppCommandInListen(tcpIfPacket); break;
        case TCP_S_SYN_SENT:     processAppCommandInSynRcvd(tcpIfPacket); break;
        case TCP_S_SYN_RCVD:     processAppCommandInSynSent(tcpIfPacket); break;
        case TCP_S_ESTABLISHED:  processAppCommandInEstablished(tcpIfPacket); break;
        case TCP_S_CLOSE_WAIT:   processAppCommandInCloseWait(tcpIfPacket); break;
        case TCP_S_LAST_ACK:     processAppCommandInLastAck(tcpIfPacket); break;
        case TCP_S_FIN_WAIT_1:   processAppCommandInFinWait1(tcpIfPacket); break;
        case TCP_S_FIN_WAIT_2:   processAppCommandInFinWait2(tcpIfPacket); break;
        case TCP_S_CLOSING:      processAppCommandInClosing(tcpIfPacket); break;
        case TCP_S_TIME_WAIT:    processAppCommandInTimeWait(tcpIfPacket); break;
        default: opp_error("wrong TCP state");
    }

    // then state transitions
    TCPEvent event = analyseAppCommandEvent(tcpIfPacket);
    return performStateTransition(event);
}

void TCPConnection::sendToIP(TCPSegment *tcpseg)
{
    IPInterfacePacket *ipIfPacket = new IPInterfacePacket();
    ipIfPacket->encapsulate(tcpseg);
    ipIfPacket->setProtocol(IP_PROT_TCP);
    ipIfPacket->setSrcAddr(localAddr);
    ipIfPacket->setDestAddr(remoteAddr);

    module->send(tcpseg,"to_ip");
}

void TCPConnection::sendToApp(TCPInterfacePacket *tcpIfPacket)
{
    module->send(tcpIfPacket,"to_appl");
}

//==========
// State transition processing

TCPEvent TCPConnection::analyseTCPSegmentEvent(TCPSegment *tcpseg)
{
    TCPEvent event;
    //...
    return event;
}

TCPEvent TCPConnection::analyseTimerEvent(cMessage *msg)
{
    TCPEvent event;
    //...
    return event;
}

TCPEvent TCPConnection::analyseAppCommandEvent(TCPInterfacePacket *tcpIfPacket)
{
    TCPEvent event;
    //...
    return event;
}

bool TCPConnection::performStateTransition(const TCPEvent& event)
{
    // fsm switch ...
    return fsm.state() == TCP_S_CLOSED;
}

//==========
// Processing events in different states

void TCPConnection::processTCPSegmentInInit(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInInit(cMessage *event) {}
void TCPConnection::processAppCommandInInit(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInClosed(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInClosed(cMessage *event) {}
void TCPConnection::processAppCommandInClosed(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInListen(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInListen(cMessage *event) {}
void TCPConnection::processAppCommandInListen(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInSynRcvd(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInSynRcvd(cMessage *event) {}
void TCPConnection::processAppCommandInSynRcvd(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInSynSent(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInSynSent(cMessage *event) {}
void TCPConnection::processAppCommandInSynSent(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInEstablished(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInEstablished(cMessage *event) {}
void TCPConnection::processAppCommandInEstablished(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInCloseWait(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInCloseWait(cMessage *event) {}
void TCPConnection::processAppCommandInCloseWait(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInLastAck(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInLastAck(cMessage *event) {}
void TCPConnection::processAppCommandInLastAck(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInFinWait1(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInFinWait1(cMessage *event) {}
void TCPConnection::processAppCommandInFinWait1(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInFinWait2(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInFinWait2(cMessage *event) {}
void TCPConnection::processAppCommandInFinWait2(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInClosing(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInClosing(cMessage *event) {}
void TCPConnection::processAppCommandInClosing(TCPInterfacePacket *tcpIfPacket) {}

void TCPConnection::processTCPSegmentInTimeWait(TCPSegment *tcpseg) {}
void TCPConnection::processTimerInTimeWait(cMessage *event) {}
void TCPConnection::processAppCommandInTimeWait(TCPInterfacePacket *tcpIfPacket) {}


