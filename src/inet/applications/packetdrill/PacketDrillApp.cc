//
// Copyright (C) 2015 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <stdlib.h>
#include <stdio.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "PacketDrillUtils.h"
#include "PacketDrillApp.h"
#include "PacketDrillInfo_m.h"
#include "inet/transportlayer/udp/UDPPacket_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram_m.h"

Define_Module(PacketDrillApp);

namespace inet {

#define MSGKIND_START  0
#define MSGKIND_EVENT  1

PacketDrillApp::PacketDrillApp()
{
    localPort = 1000;
    remotePort = 2000;
    protocol = 0;
    idInbound = 0;
    idOutbound = 0;
    relSequenceIn = 0;
    relSequenceOut = 0;
    peerTS = 0;
    peerWindow = 0;
    localAddress = L3Address("127.0.0.1");
    remoteAddress = L3Address("127.0.0.1");
}

void PacketDrillApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // parameters
        msgArrived = false;
        recvFromSet = false;
        listenSet = false;
        acceptSet = false;
        establishedPending = false;
        receivedPackets = new cPacketQueue("receiveQueue");
        outboundPackets = new cPacketQueue("outboundPackets");
        expectedMessageSize = 0;
        eventCounter = 0;
        numEvents = 0;
        eventTimer = new cMessage("event timer");
        eventTimer->setKind(MSGKIND_EVENT);
        simStartTime = simTime();
        simRelTime = simTime();
    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
        pd = new PacketDrill(this);
        config = new PacketDrillConfig();
        scriptFile = (const char *)(par("scriptFile"));
        script = new PacketDrillScript(scriptFile);
        localAddress = L3Address(par("localAddress"));
        remoteAddress = L3Address(par("remoteAddress"));
        localPort = par("localPort");
        remotePort = par("remotePort");

        cMessage* timeMsg = new cMessage("PacketDrillAppTimer");
        timeMsg->setKind(MSGKIND_START);
        scheduleAt((simtime_t)par("startTime"), timeMsg);
    }
}


void PacketDrillApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    } else {
        if (msg->getArrivalGate()->isName("tunIn")) {
            if (outboundPackets->getLength() == 0) {
#if OMNETPP_VERSION < 0x500
                cMessage *nextMsg = getSimulation()->getScheduler()->getNextEvent();
#else
                cEvent *nextMsg = getSimulation()->getScheduler()->guessNextEvent();
#endif
                if ((simTime() + par("latency")) < nextMsg->getArrivalTime()) {
                    delete (PacketDrillInfo *)msg->getContextPointer();
                    delete msg;
                    throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
                } else {
                    PacketDrillInfo *info = new PacketDrillInfo();
                    info->setLiveTime(getSimulation()->getSimTime());
                    msg->setContextPointer(info);
                    receivedPackets->insert(PK(msg));
                }
            } else {
                IPv4Datagram *datagram = check_and_cast<IPv4Datagram *>(outboundPackets->pop());
                IPv4Datagram *live = check_and_cast<IPv4Datagram*>(msg);
                PacketDrillInfo *info = (PacketDrillInfo *)datagram->getContextPointer();
                if (verifyTime((enum eventTime_t) info->getTimeType(), info->getScriptTime(),
                        info->getScriptTimeEnd(), info->getOffset(), getSimulation()->getSimTime(), "outbound packet")
                        == STATUS_ERR) {
                    delete info;
                    delete msg;
                    throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
                }
                if (!compareDatagram(datagram, live)) {
                    delete (PacketDrillInfo *)msg->getContextPointer();
                    delete msg;
                    throw cTerminationException("Packetdrill error: Datagrams are not the same");
                }
                delete info;
                if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                    eventCounter++;
                    scheduleEvent();
                }
                delete (PacketDrillInfo *)msg->getContextPointer();
                delete msg;
                delete datagram;
            }
        } else if (msg->getArrivalGate()->isName("udpIn")) {
            PacketDrillEvent *event = (PacketDrillEvent *)(script->getEventList()->get(eventCounter));
            if (verifyTime((enum eventTime_t) event->getTimeType(), event->getEventTime(), event->getEventTimeEnd(),
                    event->getEventOffset(), getSimulation()->getSimTime(), "inbound packet") == STATUS_ERR) {
                delete msg;
                return;
            }
            if (recvFromSet) {
                recvFromSet = false;
                msgArrived = false;
                if (!(PK(msg)->getByteLength() == expectedMessageSize)) {
                    delete msg;
                    throw cTerminationException("Packetdrill error: Received data has unexpected size");
                }
                if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                    eventCounter++;
                    scheduleEvent();
                }
                delete msg;
            } else {
                PacketDrillInfo* info = new PacketDrillInfo();
                info->setLiveTime(getSimulation()->getSimTime());
                msg->setContextPointer(info);
                receivedPackets->insert(PK(msg));
                msgArrived = true;
                if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                    eventCounter++;
                    scheduleEvent();
                }
            }
        } else if (msg->getArrivalGate()->isName("tcpIn")) {
            switch (msg->getKind()) {
                case TCP_I_ESTABLISHED:
                    if (listenSet) {
                        if (acceptSet) {
                            tcpSocket.setState(TCPSocket::CONNECTED);
                            tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getConnId();
                            listenSet = false;
                            acceptSet = false;
                        } else {
                            tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getConnId();
                            establishedPending = true;
                        }
                    } else {
                        tcpSocket.setState(TCPSocket::CONNECTED);
                        tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getConnId();
                    }
                    delete msg;
                    break;
                case TCP_I_CLOSED:
                    delete msg;
                    break;
                case TCP_I_DATA_NOTIFICATION:
                    if (recvFromSet)
                    {
                        cMessage *msg = new cMessage("data request");
                        msg->setKind(TCP_C_READ);
                        TCPCommand *cmd = new TCPCommand();
                        cmd->setConnId(tcpConnId);
                        msg->setControlInfo(cmd);
                        send(msg, "tcpOut");
                        recvFromSet = false;
                        // send a receive request to TCP
                    }
                    msgArrived = true;
                    delete msg;
                    break;
                default: EV_INFO << "Message kind not supported (yet)";
            }

        } else {
            delete msg;
            throw cRuntimeError("Unknown gate");
        }
    }
}

void PacketDrillApp::adjustTimes(PacketDrillEvent *event)
{
    simtime_t offset, offsetLastEvent;
    if (event->getTimeType() == ANY_TIME ||
        event->getTimeType() == RELATIVE_TIME ||
        event->getTimeType() == RELATIVE_RANGE_TIME) {
        offset = getSimulation()->getSimTime() - simStartTime;
        cQueue *eventList = script->getEventList();
        offsetLastEvent = ((PacketDrillEvent *)(eventList->get(eventCounter - 1)))->getEventTime() - simStartTime;
        offset = (offset.dbl() > offsetLastEvent.dbl()) ? offset : offsetLastEvent;
        event->setEventOffset(offset);
        event->setEventTime(event->getEventTime() + offset + simStartTime);
        if (event->getTimeType() == RELATIVE_RANGE_TIME) {
            event->setEventTimeEnd(event->getEventTimeEnd() + offset + simStartTime);
        }
    } else if (event->getTimeType() == ABSOLUTE_TIME) {
        event->setEventTime(event->getEventTime() + simStartTime);
    } else
        throw cRuntimeError("Unknown time type");
}

void PacketDrillApp::scheduleEvent()
{
    cQueue *eventList = script->getEventList();
    eventList->setName("eventQueue");
    PacketDrillEvent *event = (PacketDrillEvent *)(eventList->get(eventCounter));
    event->setEventNumber(eventCounter);
    adjustTimes(event);
    cancelEvent(eventTimer);
    eventTimer->setContextPointer(event);
    scheduleAt(event->getEventTime(), eventTimer);
}

void PacketDrillApp::runEvent(PacketDrillEvent* event)
{
    char str[128];
    if (event->getType() == PACKET_EVENT) {
        IPv4Datagram *ip = check_and_cast<IPv4Datagram *>(event->getPacket()->getInetPacket());
        if (event->getPacket()->getDirection() == DIRECTION_INBOUND) { // < injected packet, will go through the stack bottom up.
            if (protocol == IP_PROT_TCP) {
                TCPSegment* tcp = check_and_cast<TCPSegment*>(ip->decapsulate());
                tcp->setAckNo(tcp->getAckNo() + relSequenceOut);
                if (tcp->getHeaderOptionArraySize() > 0) {
                    for (unsigned int i = 0; i < tcp->getHeaderOptionArraySize(); i++) {
                        if (tcp->getHeaderOption(i)->getKind() == TCPOPT_TIMESTAMP) {
                            TCPOptionTimestamp *option = new TCPOptionTimestamp();
                            option->setEchoedTimestamp(peerTS);
                            tcp->setHeaderOption(i, option);
                        }
                    }
                }
                ip->encapsulate(tcp);
                snprintf(str, sizeof(str), "inbound %d", eventCounter);
                ip->setName(str);
            }
            send(ip, "tunOut");
        } else if (event->getPacket()->getDirection() == DIRECTION_OUTBOUND) { // >
            if (receivedPackets->getLength() > 0) {
                IPv4Datagram *live = check_and_cast<IPv4Datagram *>(receivedPackets->pop());
                if (ip && live) {
                    PacketDrillInfo *liveInfo = (PacketDrillInfo *)live->getContextPointer();
                    if (verifyTime((enum eventTime_t) event->getTimeType(), event->getEventTime(),
                            event->getEventTimeEnd(), event->getEventOffset(), liveInfo->getLiveTime(),
                            "outbound packet") == STATUS_ERR) {
                        delete liveInfo;
                        delete live;
                        delete ip;
                        throw cTerminationException("Packetdrill error: Timing error");
                    }
                    delete liveInfo;
                    if (!compareDatagram(ip, live)) {
                        delete liveInfo;
                        throw cTerminationException("Packetdrill error: Datagrams are not the same");
                    }
                    if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                        eventCounter++;
                        scheduleEvent();
                    }
                }
                delete live;
                delete ip;
            } else {
                PacketDrillInfo *info = new PacketDrillInfo("outbound");
                info->setScriptTime(event->getEventTime());
                info->setScriptTimeEnd(event->getEventTimeEnd());
                info->setOffset(event->getEventOffset());
                info->setTimeType(event->getTimeType());
                ip->setContextPointer(info);
                snprintf(str, sizeof(str), "outbound %d", eventCounter);
                ip->setName(str);
                outboundPackets->insert(ip);
            }
        } else
            throw cRuntimeError("Invalid direction");
    } else if (event->getType() == SYSCALL_EVENT) {
        EV_INFO << "syscallEvent: time_type = " << event->getTimeType() << " event time = " << event->getEventTime()
                << " end event time = " << event->getEventTimeEnd() << endl;
        runSystemCallEvent(event, event->getSyscall());
    }
}

void PacketDrillApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_START: {
            simStartTime = getSimulation()->getSimTime();
            simRelTime = simStartTime;
            if (script->parseScriptAndSetConfig(config, NULL))
                throw cRuntimeError("Error parsing the script");
            numEvents = script->getEventList()->getLength();
            scheduleEvent();
            delete msg;
            break;
        }

        case MSGKIND_EVENT: {
            PacketDrillEvent *event = (PacketDrillEvent *)msg->getContextPointer();
            runEvent(event);
            if ((!recvFromSet && outboundPackets->getLength() == 0) &&
                (!eventTimer->isScheduled() && eventCounter < numEvents - 1)) {
                eventCounter++;
                scheduleEvent();
            }
            break;
        }

        default:
            throw cRuntimeError("Unknown message kind");
    }
}

void PacketDrillApp::runSystemCallEvent(PacketDrillEvent* event, struct syscall_spec *syscall)
{
    char *error = NULL;
    const char *name = syscall->name;
    cQueue *args = new cQueue("systemCallArgs");
    int result = 0;

    // Evaluate script symbolic expressions to get live numeric args for system calls.

    if (pd->evaluateExpressionList(syscall->arguments, args, &error)) {
        free(error);
        return;
    }
    if (!strcmp(name, "socket")) {
        syscallSocket(syscall, args, &error);
    } else if (!strcmp(name, "bind")) {
        syscallBind(syscall, args, &error);
    } else if (!strcmp(name, "listen")) {
        syscallListen(syscall, args, &error);
    } else if (!strcmp(name, "write")) {
        syscallWrite(syscall, args, &error);
    } else if (!strcmp(name, "read")) {
        syscallRead((PacketDrillEvent*) event, syscall, args, &error);
    } else if (!strcmp(name, "sendto")) {
        syscallSendTo(syscall, args, &error);
    } else if (!strcmp(name, "recvfrom")) {
        syscallRecvFrom((PacketDrillEvent*)event, syscall, args, &error);
    } else if (!strcmp(name, "close")) {
        syscallClose(syscall, args, &error);
    } else if (!strcmp(name, "connect")) {
        syscallConnect(syscall, args, &error);
    } else if (!strcmp(name, "accept")) {
        syscallAccept(syscall, args, &error);
    } else {
        EV_INFO << "System call %s not known (yet)." << name;
    }

    delete(args);
    delete(syscall->arguments);

    if (result == STATUS_ERR) {
        EV_ERROR << event->getLineNumber() << ": runtime error in " << syscall->name << " call: " << error << endl;
        free(error);
    }
    return;
}

int PacketDrillApp::syscallSocket(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int type;
    PacketDrillExpression *exp;

    if (args->getLength() != 3) {
        return STATUS_ERR;
    }
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS)) {
        return STATUS_ERR;
    }
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || exp->getS32(&type, error)) {
        return STATUS_ERR;
    }
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || exp->getS32(&protocol, error)) {
        return STATUS_ERR;
    }

    switch (protocol) {
        case IP_PROT_UDP:
            udpSocket.setOutputGate(gate("udpOut"));
            udpSocket.bind(localPort);
            break;

        case IP_PROT_TCP:
            tcpSocket.readDataTransferModePar(*this);
            tcpSocket.setOutputGate(gate("tcpOut"));
            tcpSocket.bind(localPort);
        break;
        default:
            throw cRuntimeError("Protocol type not supported for this system call");
    }

    return STATUS_OK;
}

int PacketDrillApp::syscallBind(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;
    PacketDrillExpression *exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP:
            break;

        case IP_PROT_TCP:
            if (tcpSocket.getState() == TCPSocket::NOT_BOUND) {
                tcpSocket.bind(localAddress, localPort);
            }
            break;
        default:
            throw cRuntimeError("Protocol type not supported for this system call");
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallListen(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, backlog;
    PacketDrillExpression *exp;

    if (args->getLength() != 2)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || exp->getS32(&backlog, error))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP:
            break;

        case IP_PROT_TCP:
            listenSet = true;
            tcpSocket.listenOnce();
        break;
        default:
            throw cRuntimeError("Protocol type not supported for this system call");
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallAccept(struct syscall_spec *syscall, cQueue *args, char **error)
{
    if (!listenSet)
        return STATUS_ERR;

    if (establishedPending) {
        establishedPending = false;
    } else {
        acceptSet = true;
    }

    return STATUS_OK;
}

int PacketDrillApp::syscallWrite(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count;
    PacketDrillExpression* exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(2);
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;

    switch (protocol)
    {
        case IP_PROT_TCP: {
            cPacket *payload = new cPacket("Write");
            payload->setByteLength(syscall->result->getNum());
            tcpSocket.send(payload);
            break;
        }
        default: EV_INFO << "Protocol not supported for this socket call";
    }

    return STATUS_OK;
}


int PacketDrillApp::syscallConnect(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;
    PacketDrillExpression *exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP:
            break;

        case IP_PROT_TCP:
            tcpSocket.connect(remoteAddress, remotePort);
        break;
        default:
            throw cRuntimeError("Protocol type not supported for this system call");
    }

    return STATUS_OK;
}


int PacketDrillApp::syscallSendTo(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count, flags;
    PacketDrillExpression *exp;

    if (args->getLength() != 6)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(3);
    if (!exp || exp->getS32(&flags, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(4);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(5);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    cPacket *payload = new cPacket("SendTo");
    payload->setByteLength(count);

    switch (protocol) {
        case IP_PROT_UDP:
            udpSocket.sendTo(payload, remoteAddress, remotePort);
            break;

        default:
            throw cRuntimeError("Protocol type not supported for this system call");
    }

    return STATUS_OK;
}

int PacketDrillApp::syscallRead(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count;
    PacketDrillExpression* exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(2);
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;

    if ((expectedMessageSize = syscall->result->getNum()) > 0) {
        if (msgArrived) {
            switch (protocol) {
                case IP_PROT_TCP: {
                    cMessage *msg = new cMessage("dataRequest");
                    msg->setKind(TCP_C_READ);
                    TCPCommand *tcpcmd = new TCPCommand();
                    tcpcmd->setConnId(tcpConnId);
                    msg->setControlInfo(tcpcmd);
                    send(msg, "tcpOut");
                    break;
                }
                default: EV_INFO << "Protoocl not supported for this system call.";
            }
            msgArrived = false;
            expectedMessageSize = syscall->result->getNum();
            recvFromSet = true;
            // send a receive request to TCP
        }
        else
        {
            recvFromSet = true;
        }
    }
    else
    {
        if (msgArrived)
        {
            outboundPackets->pop();
            msgArrived = false;
        }
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallRecvFrom(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **err)
{
    int script_fd, count, flags;
    PacketDrillExpression *exp;

    if (args->getLength() != 6)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, err))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || exp->getS32(&count, err))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(3);
    if (!exp || exp->getS32(&flags, err))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(4);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(5);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    if (msgArrived) {
        cMessage *msg = (cMessage*)(receivedPackets->pop());
        msgArrived = false;
        recvFromSet = false;
        if (!(PK(msg)->getByteLength() == syscall->result->getNum())) {
            delete msg;
            throw cTerminationException("Packetdrill error: Wrong payload length");
        }
        PacketDrillInfo *info = (PacketDrillInfo *)msg->getContextPointer();
        if (verifyTime((enum eventTime_t) event->getTimeType(), event->getEventTime(), event->getEventTimeEnd(),
                event->getEventOffset(), info->getLiveTime(), "inbound packet") == STATUS_ERR) {
            delete info;
            delete msg;
            return false;
        }
        delete info;
        delete msg;
    } else {
        expectedMessageSize = syscall->result->getNum();
        recvFromSet = true;
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallClose(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;

    if (args->getLength() != 1)
        return STATUS_ERR;
    PacketDrillExpression *exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP: {
            EV_DETAIL << "close UDP socket\n";
            udpSocket.close();
            break;
        }

        case IP_PROT_TCP: {
            cMessage *msg = new cMessage("close");
            msg->setKind(TCP_C_CLOSE);
            TCPCommand *cmd = new TCPCommand();
            cmd->setConnId(tcpConnId);
            msg->setControlInfo(cmd);
            send(msg, "tcpOut");
            break;
        }
        default:
            EV_INFO << "Protocol " << protocol << " is not supported for this system call\n";
    }

    return STATUS_OK;
}

void PacketDrillApp::finish()
{
    delete receivedPackets;
    delete outboundPackets;
    EV_INFO << "PacketDrillApp finished\n";
}

PacketDrillApp::~PacketDrillApp()
{
    delete eventTimer;
    delete script;
}

// Verify that something happened at the expected time.

int PacketDrillApp::verifyTime(enum eventTime_t timeType, simtime_t scriptTime, simtime_t scriptTimeEnd, simtime_t offset,
        simtime_t liveTime, const char *description)
{
    simtime_t expectedTime = scriptTime;
    simtime_t expectedTimeEnd = scriptTimeEnd;
    simtime_t actualTime = liveTime;
    simtime_t tolerance = SimTime(config->getToleranceUsecs(), SIMTIME_US);

    if (timeType == ANY_TIME) {
        return STATUS_OK;
    }

    if (timeType == ABSOLUTE_RANGE_TIME || timeType == RELATIVE_RANGE_TIME) {
        if (actualTime < (expectedTime - tolerance) || actualTime > (expectedTimeEnd + tolerance)) {
            if (timeType == ABSOLUTE_RANGE_TIME) {
                EV_INFO << "timing error: expected " << description << " in time range " << scriptTime << " ~ "
                        << scriptTimeEnd << " sec, but happened at " << actualTime << " sec" << endl;
            } else if (timeType == RELATIVE_RANGE_TIME) {
                EV_INFO << "timing error: expected " << description << " in relative time range +"
                        << scriptTime - offset << " ~ " << scriptTimeEnd - offset << " sec, but happened at +"
                        << actualTime - offset << " sec" << endl;
            }
            return STATUS_ERR;
        } else {
            return STATUS_OK;
        }
    }

    if ((actualTime < (expectedTime - tolerance)) || (actualTime > (expectedTime + tolerance))) {
        EV_INFO << "timing error: expected " << description << " at " << scriptTime << " sec, but happened at "
                << actualTime << " sec" << endl;
        return STATUS_ERR;
    } else {
        return STATUS_OK;
    }
}

bool PacketDrillApp::compareDatagram(IPv4Datagram *storedDatagram, IPv4Datagram *liveDatagram)
{
    if (!(storedDatagram->getSrcAddress() == liveDatagram->getSrcAddress())) {
        return false;
    }
    if (!(storedDatagram->getDestAddress() == liveDatagram->getDestAddress())) {
        return false;
    }
    if (!(storedDatagram->getTransportProtocol() == liveDatagram->getTransportProtocol())) {
        return false;
    }
    if (!(storedDatagram->getTimeToLive() == liveDatagram->getTimeToLive())) {
        return false;
    }
    if (!(storedDatagram->getIdentification() == liveDatagram->getIdentification())){
        return false;
    }
    if (!(storedDatagram->getMoreFragments() == liveDatagram->getMoreFragments())) {
        return false;
    }
    if (!(storedDatagram->getDontFragment() == liveDatagram->getDontFragment())) {
        return false;
    }
    if (!(storedDatagram->getFragmentOffset() == liveDatagram->getFragmentOffset())) {
        return false;
    }
    if (!(storedDatagram->getTypeOfService() == liveDatagram->getTypeOfService())) {
        return false;
    }
    if (!(storedDatagram->getHeaderLength() == liveDatagram->getHeaderLength())) {
        return false;
    }
    switch (storedDatagram->getTransportProtocol()) {
        case IP_PROT_UDP: {
            UDPPacket *storedUdp = check_and_cast<UDPPacket *>(storedDatagram->getEncapsulatedPacket());
            UDPPacket *liveUdp = check_and_cast<UDPPacket *>(liveDatagram->getEncapsulatedPacket());
            if (!(compareUdpPacket(storedUdp, liveUdp))) {
                return false;
            }
            break;
        }

        case IP_PROT_TCP: {
            TCPSegment *storedTcp = check_and_cast<TCPSegment *>(storedDatagram->getEncapsulatedPacket());
            TCPSegment *liveTcp = check_and_cast<TCPSegment *>(liveDatagram->getEncapsulatedPacket());
            if (storedTcp->getSynBit()) { // SYN was sent. Store the sequence number for comparisons
                relSequenceOut = liveTcp->getSequenceNo();
            }
            if (storedTcp->getSynBit() && storedTcp->getAckBit()) {
                peerWindow = liveTcp->getWindow();
            }
            if (!(compareTcpPacket(storedTcp, liveTcp))) {
                return false;
            }
            break;
        }

        default: EV_INFO << "Transport protocol %d is not supported yet" << storedDatagram->getTransportProtocol();
    }
    return true;
}

bool PacketDrillApp::compareUdpPacket(UDPPacket *storedUdp, UDPPacket *liveUdp)
{
    return (storedUdp->getSourcePort() == liveUdp->getSourcePort()) &&
        (storedUdp->getDestinationPort() == liveUdp->getDestinationPort());
}

bool PacketDrillApp::compareTcpPacket(TCPSegment *storedTcp, TCPSegment *liveTcp)
{
    if (!(storedTcp->getSrcPort() == liveTcp->getSrcPort())) {
        return false;
    }
    if (!(storedTcp->getDestPort() == liveTcp->getDestPort())) {
        return false;
    }
    if (!(storedTcp->getSequenceNo() + relSequenceOut == liveTcp->getSequenceNo())) {
        return false;
    }
    if (!(storedTcp->getAckNo() == liveTcp->getAckNo())) {
        return false;
    }
    if (!(storedTcp->getUrgBit() == liveTcp->getUrgBit()) || !(storedTcp->getAckBit() == liveTcp->getAckBit()) ||
        !(storedTcp->getPshBit() == liveTcp->getPshBit()) || !(storedTcp->getRstBit() == liveTcp->getRstBit()) ||
        !(storedTcp->getSynBit() == liveTcp->getSynBit()) || !(storedTcp->getFinBit() == liveTcp->getFinBit())) {
        return false;
    }
    if (!(storedTcp->getUrgentPointer() == liveTcp->getUrgentPointer())) {
        return false;
    }

    if (storedTcp->getHeaderOptionArraySize() > 0 || liveTcp->getHeaderOptionArraySize()) {
     EV_DETAIL << "Options present";
        if (storedTcp->getHeaderOptionArraySize() == 0) {
            return true;
        }
        if (storedTcp->getHeaderOptionArraySize() != liveTcp->getHeaderOptionArraySize()) {
            TCPOption *liveOption;
            for (unsigned int i = 0; i < liveTcp->getHeaderOptionArraySize(); i++) {
                liveOption = liveTcp->getHeaderOption(i);
            }
            return false;
        } else {
            TCPOption *storedOption, *liveOption;
            for (unsigned int i = 0; i < storedTcp->getHeaderOptionArraySize(); i++) {
                storedOption = storedTcp->getHeaderOption(i);
                liveOption = liveTcp->getHeaderOption(i);
                if (storedOption->getKind() == liveOption->getKind()) {
                    switch (storedOption->getKind()) {
                        case TCPOPT_EOL:
                        case TCPOPT_NOP:
                            if (!(storedOption->getLength() == liveOption->getLength())) {
                                return false;
                            }
                            break;
                        case TCPOPT_SACK_PERMITTED:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 2)) {
                                return false;
                            }
                            break;
                        case TCPOPT_WINDOW:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 3 &&
                                check_and_cast<TCPOptionWindowScale *>(storedOption)->getWindowScale()
                                 == check_and_cast<TCPOptionWindowScale *>(liveOption)->getWindowScale())) {
                                return false;
                            }
                            break;
                        case TCPOPT_SACK:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() > 2 && (storedOption->getLength() % 8) == 2 &&
                                check_and_cast<TCPOptionSack *>(storedOption)->getSackItemArraySize()
                                == check_and_cast<TCPOptionSack *>(liveOption)->getSackItemArraySize())) {
                                return false;
                            }
                            break;
                        case TCPOPT_TIMESTAMP:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 10 &&
                                check_and_cast<TCPOptionTimestamp *>(storedOption)->getSenderTimestamp()
                                == check_and_cast<TCPOptionTimestamp *>(liveOption)->getSenderTimestamp())) {
                                return false;
                            }
                            break;
                        default: EV_INFO << "Option not supported";
                    }
                } else {
                    EV_INFO << "Wrong sequence or option kind not present";
                    return false;
                }
            }
        }

    }
    return true;
}

} // namespace INET
