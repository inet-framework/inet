//
// Copyright (C) 2004 Andras Varga
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

#include "inet/transportlayer/tcp/TCP.h"

#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/tcp/TCPConnection.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/ICMPMessage_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#endif // ifdef WITH_IPv6

#include "inet/transportlayer/tcp/queues/TCPByteStreamRcvQueue.h"
#include "inet/transportlayer/tcp/queues/TCPByteStreamSendQueue.h"
#include "inet/transportlayer/tcp/queues/TCPMsgBasedRcvQueue.h"
#include "inet/transportlayer/tcp/queues/TCPMsgBasedSendQueue.h"
#include "inet/transportlayer/tcp/queues/TCPVirtualDataRcvQueue.h"
#include "inet/transportlayer/tcp/queues/TCPVirtualDataSendQueue.h"

namespace inet {

namespace tcp {

Define_Module(TCP);

#define EPHEMERAL_PORTRANGE_START    1024
#define EPHEMERAL_PORTRANGE_END      5000

static std::ostream& operator<<(std::ostream& os, const TCP::SockPair& sp)
{
    os << "loc=" << sp.localAddr << ":" << sp.localPort << " "
       << "rem=" << sp.remoteAddr << ":" << sp.remotePort;
    return os;
}

static std::ostream& operator<<(std::ostream& os, const TCP::AppConnKey& app)
{
    os << "connId=" << app.connId << " appGateIndex=" << app.appGateIndex;
    return os;
}

static std::ostream& operator<<(std::ostream& os, const TCPConnection& conn)
{
    os << "connId=" << conn.connId << " " << TCPConnection::stateName(conn.getFsmState())
       << " state={" << const_cast<TCPConnection&>(conn).getState()->info() << "}";
    return os;
}

void TCP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *q;
        q = par("sendQueueClass");
        if (*q != '\0')
            throw cRuntimeError("Don't use obsolete sendQueueClass = \"%s\" parameter", q);

        q = par("receiveQueueClass");
        if (*q != '\0')
            throw cRuntimeError("Don't use obsolete receiveQueueClass = \"%s\" parameter", q);

        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
        WATCH(lastEphemeralPort);

        WATCH_PTRMAP(tcpConnMap);
        WATCH_PTRMAP(tcpAppConnMap);

        recordStatistics = par("recordStats");
        useDataNotification = par("useDataNotification");
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        cModule *host = findContainingNode(this);
        NodeStatus *nodeStatus = check_and_cast_nullable<NodeStatus *>(host ? host->getSubmodule("status") : nullptr);
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_TCP);
    }
}

TCP::~TCP()
{
    while (!tcpAppConnMap.empty()) {
        auto i = tcpAppConnMap.begin();
        delete i->second;
        tcpAppConnMap.erase(i);
    }
}

void TCP::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Model error: self msg '%s' received when isOperational is false", msg->getName());
        EV_ERROR << "TCP is turned off, dropping '" << msg->getName() << "' message\n";
        delete msg;
    }
    else if (msg->isSelfMessage()) {
        TCPConnection *conn = (TCPConnection *)msg->getContextPointer();
        bool ret = conn->processTimer(msg);
        if (!ret)
            removeConnection(conn);
    }
    else if (msg->arrivedOn("ipIn")) {
        if (false
#ifdef WITH_IPv4
            || dynamic_cast<ICMPMessage *>(msg)
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
            || dynamic_cast<ICMPv6Message *>(msg)
#endif // ifdef WITH_IPv6
            )
        {
            EV_DETAIL << "ICMP error received -- discarding\n";    // FIXME can ICMP packets really make it up to TCP???
            delete msg;
        }
        else {
            // must be a TCPSegment
            TCPSegment *tcpseg = check_and_cast<TCPSegment *>(msg);

            // get src/dest addresses
            L3Address srcAddr, destAddr;

            cObject *ctrl = tcpseg->removeControlInfo();
            if (!ctrl)
                throw cRuntimeError("(%s)%s arrived without control info", tcpseg->getClassName(), tcpseg->getName());

            INetworkProtocolControlInfo *controlInfo = check_and_cast<INetworkProtocolControlInfo *>(ctrl);
            srcAddr = controlInfo->getSourceAddress();
            destAddr = controlInfo->getDestinationAddress();
            //interfaceId = controlInfo->getInterfaceId();
            delete ctrl;

            // process segment
            TCPConnection *conn = findConnForSegment(tcpseg, srcAddr, destAddr);
            if (conn) {
                bool ret = conn->processTCPSegment(tcpseg, srcAddr, destAddr);
                if (!ret)
                    removeConnection(conn);
            }
            else {
                segmentArrivalWhileClosed(tcpseg, srcAddr, destAddr);
            }
        }
    }
    else {    // must be from app
        TCPCommand *controlInfo = check_and_cast<TCPCommand *>(msg->getControlInfo());
        int appGateIndex = msg->getArrivalGate()->getIndex();
        int connId = controlInfo->getConnId();

        TCPConnection *conn = findConnForApp(appGateIndex, connId);

        if (!conn) {
            conn = createConnection(appGateIndex, connId);

            // add into appConnMap here; it'll be added to connMap during processing
            // the OPEN command in TCPConnection's processAppCommand().
            AppConnKey key;
            key.appGateIndex = appGateIndex;
            key.connId = connId;
            tcpAppConnMap[key] = conn;

            EV_INFO << "TCP connection created for " << msg << "\n";
        }
        bool ret = conn->processAppCommand(msg);
        if (!ret)
            removeConnection(conn);
    }

    if (hasGUI())
        updateDisplayString();
}

TCPConnection *TCP::createConnection(int appGateIndex, int connId)
{
    return new TCPConnection(this, appGateIndex, connId);
}

void TCP::segmentArrivalWhileClosed(TCPSegment *tcpseg, L3Address srcAddr, L3Address destAddr)
{
    TCPConnection *tmp = new TCPConnection();
    tmp->segmentArrivalWhileClosed(tcpseg, srcAddr, destAddr);
    delete tmp;
    delete tcpseg;
}

void TCP::updateDisplayString()
{
#if OMNETPP_VERSION < 0x0500
    if (getEnvir()->isDisabled()) {
#else
    if (getEnvir()->isExpressMode()) {
#endif
        // in express mode, we don't bother to update the display
        // (std::map's iteration is not very fast if map is large)
        getDisplayString().setTagArg("t", 0, "");
        return;
    }

    //char buf[40];
    //sprintf(buf,"%d conns", tcpAppConnMap.size());
    //getDisplayString().setTagArg("t",0,buf);
    if (isOperational)
        getDisplayString().removeTag("i2");
    else
        getDisplayString().setTagArg("i2", 0, "status/cross");

    int numINIT = 0, numCLOSED = 0, numLISTEN = 0, numSYN_SENT = 0, numSYN_RCVD = 0,
        numESTABLISHED = 0, numCLOSE_WAIT = 0, numLAST_ACK = 0, numFIN_WAIT_1 = 0,
        numFIN_WAIT_2 = 0, numCLOSING = 0, numTIME_WAIT = 0;

    for (auto i = tcpAppConnMap.begin(); i != tcpAppConnMap.end(); ++i) {
        int state = (*i).second->getFsmState();

        switch (state) {
            case TCP_S_INIT:
                numINIT++;
                break;

            case TCP_S_CLOSED:
                numCLOSED++;
                break;

            case TCP_S_LISTEN:
                numLISTEN++;
                break;

            case TCP_S_SYN_SENT:
                numSYN_SENT++;
                break;

            case TCP_S_SYN_RCVD:
                numSYN_RCVD++;
                break;

            case TCP_S_ESTABLISHED:
                numESTABLISHED++;
                break;

            case TCP_S_CLOSE_WAIT:
                numCLOSE_WAIT++;
                break;

            case TCP_S_LAST_ACK:
                numLAST_ACK++;
                break;

            case TCP_S_FIN_WAIT_1:
                numFIN_WAIT_1++;
                break;

            case TCP_S_FIN_WAIT_2:
                numFIN_WAIT_2++;
                break;

            case TCP_S_CLOSING:
                numCLOSING++;
                break;

            case TCP_S_TIME_WAIT:
                numTIME_WAIT++;
                break;
        }
    }

    char buf2[200];
    buf2[0] = '\0';

    if (numINIT > 0)
        sprintf(buf2 + strlen(buf2), "init:%d ", numINIT);
    if (numCLOSED > 0)
        sprintf(buf2 + strlen(buf2), "closed:%d ", numCLOSED);
    if (numLISTEN > 0)
        sprintf(buf2 + strlen(buf2), "listen:%d ", numLISTEN);
    if (numSYN_SENT > 0)
        sprintf(buf2 + strlen(buf2), "syn_sent:%d ", numSYN_SENT);
    if (numSYN_RCVD > 0)
        sprintf(buf2 + strlen(buf2), "syn_rcvd:%d ", numSYN_RCVD);
    if (numESTABLISHED > 0)
        sprintf(buf2 + strlen(buf2), "estab:%d ", numESTABLISHED);
    if (numCLOSE_WAIT > 0)
        sprintf(buf2 + strlen(buf2), "close_wait:%d ", numCLOSE_WAIT);
    if (numLAST_ACK > 0)
        sprintf(buf2 + strlen(buf2), "last_ack:%d ", numLAST_ACK);
    if (numFIN_WAIT_1 > 0)
        sprintf(buf2 + strlen(buf2), "fin_wait_1:%d ", numFIN_WAIT_1);
    if (numFIN_WAIT_2 > 0)
        sprintf(buf2 + strlen(buf2), "fin_wait_2:%d ", numFIN_WAIT_2);
    if (numCLOSING > 0)
        sprintf(buf2 + strlen(buf2), "closing:%d ", numCLOSING);
    if (numTIME_WAIT > 0)
        sprintf(buf2 + strlen(buf2), "time_wait:%d ", numTIME_WAIT);

    getDisplayString().setTagArg("t", 0, buf2);
}

TCPConnection *TCP::findConnForSegment(TCPSegment *tcpseg, L3Address srcAddr, L3Address destAddr)
{
    SockPair key;
    key.localAddr = destAddr;
    key.remoteAddr = srcAddr;
    key.localPort = tcpseg->getDestPort();
    key.remotePort = tcpseg->getSrcPort();
    SockPair save = key;

    // try with fully qualified SockPair
    auto i = tcpConnMap.find(key);
    if (i != tcpConnMap.end())
        return i->second;

    // try with localAddr missing (only localPort specified in passive/active open)
    key.localAddr = L3Address();
    i = tcpConnMap.find(key);

    if (i != tcpConnMap.end())
        return i->second;

    // try fully qualified local socket + blank remote socket (for incoming SYN)
    key = save;
    key.remoteAddr = L3Address();
    key.remotePort = -1;
    i = tcpConnMap.find(key);

    if (i != tcpConnMap.end())
        return i->second;

    // try with blank remote socket, and localAddr missing (for incoming SYN)
    key.localAddr = L3Address();
    i = tcpConnMap.find(key);

    if (i != tcpConnMap.end())
        return i->second;

    // given up
    return nullptr;
}

TCPConnection *TCP::findConnForApp(int appGateIndex, int connId)
{
    AppConnKey key;
    key.appGateIndex = appGateIndex;
    key.connId = connId;

    auto i = tcpAppConnMap.find(key);
    return i == tcpAppConnMap.end() ? nullptr : i->second;
}

ushort TCP::getEphemeralPort()
{
    // start at the last allocated port number + 1, and search for an unused one
    ushort searchUntil = lastEphemeralPort++;
    if (lastEphemeralPort == EPHEMERAL_PORTRANGE_END) // wrap
        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;

    while (usedEphemeralPorts.find(lastEphemeralPort) != usedEphemeralPorts.end()) {
        if (lastEphemeralPort == searchUntil) // got back to starting point?
            throw cRuntimeError("Ephemeral port range %d..%d exhausted, all ports occupied", EPHEMERAL_PORTRANGE_START, EPHEMERAL_PORTRANGE_END);

        lastEphemeralPort++;

        if (lastEphemeralPort == EPHEMERAL_PORTRANGE_END) // wrap
            lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
    }

    // found a free one, return it
    return lastEphemeralPort;
}

void TCP::addSockPair(TCPConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort)
{
    // update addresses/ports in TCPConnection
    SockPair key;
    key.localAddr = conn->localAddr = localAddr;
    key.remoteAddr = conn->remoteAddr = remoteAddr;
    key.localPort = conn->localPort = localPort;
    key.remotePort = conn->remotePort = remotePort;

    // make sure connection is unique
    auto it = tcpConnMap.find(key);
    if (it != tcpConnMap.end()) {
        // throw "address already in use" error
        if (remoteAddr.isUnspecified() && remotePort == -1)
            throw cRuntimeError("Address already in use: there is already a connection listening on %s:%d",
                    localAddr.str().c_str(), localPort);
        else
            throw cRuntimeError("Address already in use: there is already a connection %s:%d to %s:%d",
                    localAddr.str().c_str(), localPort, remoteAddr.str().c_str(), remotePort);
    }

    // then insert it into tcpConnMap
    tcpConnMap[key] = conn;

    // mark port as used
    if (localPort >= EPHEMERAL_PORTRANGE_START && localPort < EPHEMERAL_PORTRANGE_END)
        usedEphemeralPorts.insert(localPort);
}

void TCP::updateSockPair(TCPConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort)
{
    // find with existing address/port pair...
    SockPair key;
    key.localAddr = conn->localAddr;
    key.remoteAddr = conn->remoteAddr;
    key.localPort = conn->localPort;
    key.remotePort = conn->remotePort;
    auto it = tcpConnMap.find(key);

    ASSERT(it != tcpConnMap.end() && it->second == conn);

    // ...and remove from the old place in tcpConnMap
    tcpConnMap.erase(it);

    // then update addresses/ports, and re-insert it with new key into tcpConnMap
    key.localAddr = conn->localAddr = localAddr;
    key.remoteAddr = conn->remoteAddr = remoteAddr;
    ASSERT(conn->localPort == localPort);
    key.remotePort = conn->remotePort = remotePort;
    tcpConnMap[key] = conn;

    // localPort doesn't change (see ASSERT above), so there's no need to update usedEphemeralPorts[].
}

void TCP::addForkedConnection(TCPConnection *conn, TCPConnection *newConn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort)
{
    // update conn's socket pair, and register newConn (which'll keep LISTENing)
    updateSockPair(conn, localAddr, remoteAddr, localPort, remotePort);
    addSockPair(newConn, newConn->localAddr, newConn->remoteAddr, newConn->localPort, newConn->remotePort);

    // conn will get a new connId...
    AppConnKey key;
    key.appGateIndex = conn->appGateIndex;
    key.connId = conn->connId;
    tcpAppConnMap.erase(key);
    key.connId = conn->connId = getEnvir()->getUniqueNumber();
    tcpAppConnMap[key] = conn;

    // ...and newConn will live on with the old connId
    key.appGateIndex = newConn->appGateIndex;
    key.connId = newConn->connId;
    tcpAppConnMap[key] = newConn;
}

void TCP::removeConnection(TCPConnection *conn)
{
    EV_INFO << "Deleting TCP connection\n";

    AppConnKey key;
    key.appGateIndex = conn->appGateIndex;
    key.connId = conn->connId;
    tcpAppConnMap.erase(key);

    SockPair key2;
    key2.localAddr = conn->localAddr;
    key2.remoteAddr = conn->remoteAddr;
    key2.localPort = conn->localPort;
    key2.remotePort = conn->remotePort;
    tcpConnMap.erase(key2);

    // IMPORTANT: usedEphemeralPorts.erase(conn->localPort) is NOT GOOD because it
    // deletes ALL occurrences of the port from the multiset.
    auto it = usedEphemeralPorts.find(conn->localPort);

    if (it != usedEphemeralPorts.end())
        usedEphemeralPorts.erase(it);

    delete conn;
}

void TCP::finish()
{
    EV_INFO << getFullPath() << ": finishing with " << tcpConnMap.size() << " connections open.\n";
}

TCPSendQueue *TCP::createSendQueue(TCPDataTransferMode transferModeP)
{
    switch (transferModeP) {
        case TCP_TRANSFER_BYTECOUNT:
            return new TCPVirtualDataSendQueue();

        case TCP_TRANSFER_OBJECT:
            return new TCPMsgBasedSendQueue();

        case TCP_TRANSFER_BYTESTREAM:
            return new TCPByteStreamSendQueue();

        default:
            throw cRuntimeError("Invalid TCP data transfer mode: %d", transferModeP);
    }
}

TCPReceiveQueue *TCP::createReceiveQueue(TCPDataTransferMode transferModeP)
{
    switch (transferModeP) {
        case TCP_TRANSFER_BYTECOUNT:
            return new TCPVirtualDataRcvQueue();

        case TCP_TRANSFER_OBJECT:
            return new TCPMsgBasedRcvQueue();

        case TCP_TRANSFER_BYTESTREAM:
            return new TCPByteStreamRcvQueue();

        default:
            throw cRuntimeError("Invalid TCP data transfer mode: %d", transferModeP);
    }
}

bool TCP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_TRANSPORT_LAYER) {
            //FIXME implementation
            isOperational = true;
            updateDisplayString();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_TRANSPORT_LAYER) {
            //FIXME close connections???
            reset();
            isOperational = false;
            updateDisplayString();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            //FIXME implementation
            reset();
            isOperational = false;
            updateDisplayString();
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

void TCP::reset()
{
    for (auto it = tcpAppConnMap.begin(); it != tcpAppConnMap.end(); ++it)
        delete it->second;
    tcpAppConnMap.clear();
    tcpConnMap.clear();
    usedEphemeralPorts.clear();
    lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
}

} // namespace tcp

} // namespace inet
