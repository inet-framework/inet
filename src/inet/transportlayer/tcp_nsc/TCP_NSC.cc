//
// Copyright (C) 2006 Sam Jansen, Andras Varga
//               2009 Zoltan Bojthe
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

#include "inet/transportlayer/tcp_nsc/TCP_NSC.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/ICMPMessage_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#endif // ifdef WITH_IPv6

#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"

#include "inet/common/serializer/tcp/headers/tcphdr.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/transportlayer/tcp_nsc/queues/TCP_NSC_Queues.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/common/serializer/tcp/TCPSerializer.h"

#include <assert.h>
#include <dlfcn.h>
#include <netinet/in.h>

#include "inet/transportlayer/tcp_nsc/queues/TCP_NSC_VirtualDataQueues.h"
#include "inet/transportlayer/tcp_nsc/queues/TCP_NSC_ByteStreamQueues.h"

#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/INETUtils.h"

#include <sim_errno.h>

namespace inet {

using namespace serializer;

namespace tcp {

Define_Module(TCP_NSC);

//static member variables:
const L3Address TCP_NSC::localInnerIpS("1.0.0.253");
const L3Address TCP_NSC::localInnerMaskS("255.255.255.0");
const L3Address TCP_NSC::localInnerGwS("1.0.0.254");
const L3Address TCP_NSC::remoteFirstInnerIpS("2.0.0.1");

const char *TCP_NSC::stackNameParamNameS = "stackName";
const char *TCP_NSC::bufferSizeParamNameS = "stackBufferSize";

static const unsigned short PORT_UNDEF = -1;

struct nsc_iphdr
{
#if BYTE_ORDER == LITTLE_ENDIAN
    unsigned int ihl : 4;
    unsigned int version : 4;
#elif BYTE_ORDER == BIG_ENDIAN
    unsigned int version : 4;
    unsigned int ihl : 4;
#else // if BYTE_ORDER == LITTLE_ENDIAN
# error "Please check BYTE_ORDER declaration"
#endif // if BYTE_ORDER == LITTLE_ENDIAN
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
    /*The options start here. */
} __attribute__((packed));

struct nsc_ipv6hdr
{
#if BYTE_ORDER == LITTLE_ENDIAN
    uint32_t flow : 20;
    uint32_t ds : 8;
    uint32_t version : 4;
#elif BYTE_ORDER == BIG_ENDIAN
    uint32_t version : 4;
    uint32_t ds : 8;
    uint32_t flow : 20;
#else // if BYTE_ORDER == LITTLE_ENDIAN
# error "Please check BYTE_ORDER declaration"
#endif // if BYTE_ORDER == LITTLE_ENDIAN
    uint16_t len;
    uint8_t next_header;
    uint8_t hop_limit;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr[4];
    uint32_t daddr[4];
} __attribute__((packed));

static std::ostream& operator<<(std::ostream& osP, const TCP_NSC_Connection& connP)
{
    osP << "Conn={"
        << "connId=" << connP.connIdM
        << " appGateIndex=" << connP.appGateIndexM
        << " nscsocket=" << connP.pNscSocketM
        << " sentEstablishedM=" << connP.sentEstablishedM
        << '}';
    return osP;
}

static std::ostream& operator<<(std::ostream& osP, const TCP_NSC_Connection::SockAddr& sockAddrP)
{
    osP << sockAddrP.ipAddrM << ":" << sockAddrP.portM;
    return osP;
}

static std::ostream& operator<<(std::ostream& osP, const TCP_NSC_Connection::SockPair& sockPairP)
{
    osP << "{loc=" << sockPairP.localM << ", rem=" << sockPairP.remoteM << "}";
    return osP;
}

TCP_NSC::TCP_NSC()
    : pStackM(nullptr),
    pNsiTimerM(nullptr),
    isAliveM(false),
    curAddrCounterM(0),
    curConnM(nullptr),

    // statistics:
    sndNxtVector(nullptr),
    sndAckVector(nullptr),
    rcvSeqVector(nullptr),
    rcvAckVector(nullptr)
{
    // statistics:
    if (true) {    // (getTcpMain()->recordStatistics)
        sndNxtVector = new cOutVector("sent seq");
        sndAckVector = new cOutVector("sent ack");
        rcvSeqVector = new cOutVector("rcvd seq");
        rcvAckVector = new cOutVector("rcvd ack");
    }
}

// return mapped remote ip in host byte order
// if addrP not exists in map, it's create a new nsc addr
uint32_t TCP_NSC::mapRemote2Nsc(L3Address const& addrP)
{
    auto i = remote2NscMapM.find(addrP);
    if (i != remote2NscMapM.end()) {
        return i->second;
    }

    // get first free remote NSC IP
    uint32_t ret = remoteFirstInnerIpS.toIPv4().getInt();
    for (auto j = nsc2RemoteMapM.begin(); j != nsc2RemoteMapM.end(); j++) {
        if (j->first > ret)
            break;
        ret = j->first + 1;
    }

    // add new pair to maps
    remote2NscMapM[addrP] = ret;
    nsc2RemoteMapM[ret] = addrP;
    ASSERT(remote2NscMapM.find(addrP) != remote2NscMapM.end());
    ASSERT(nsc2RemoteMapM.find(ret) != nsc2RemoteMapM.end());
    return ret;
}

// return original remote ip from remote NSC IP
// assert if  IP not exists in map
// nscAddrP in host byte order!
L3Address const& TCP_NSC::mapNsc2Remote(uint32_t nscAddrP)
{
    auto i = nsc2RemoteMapM.find(nscAddrP);

    if (i != nsc2RemoteMapM.end())
        return i->second;

    ASSERT(0);
    exit(1);
}

// x == mapNsc2Remote(mapRemote2Nsc(x))

void TCP_NSC::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    EV_TRACE << this << ": initialize stage " << stage << endl;

    if (stage == INITSTAGE_LOCAL) {
        const char *q;
        q = par("sendQueueClass");

        if (*q != '\0')
            throw cRuntimeError("Don't use obsolete sendQueueClass = \"%s\" parameter", q);

        q = par("receiveQueueClass");

        if (*q != '\0')
            throw cRuntimeError("Don't use obsolete receiveQueueClass = \"%s\" parameter", q);

        WATCH_MAP(tcpAppConnMapM);

        // load the stack
        const char *stackName = this->par(stackNameParamNameS).stringValue();
        int bufferSize = (int)(this->par(bufferSizeParamNameS).longValue());
        loadStack(stackName, bufferSize);
        pStackM->if_attach(localInnerIpS.str().c_str(), localInnerMaskS.str().c_str(), 1500);
        pStackM->add_default_gateway(localInnerGwS.str().c_str());
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_TCP);
    }
    else if (stage == INITSTAGE_LAST) {
        isAliveM = true;
    }
}

TCP_NSC::~TCP_NSC()
{
    EV_TRACE << this << ": destructor\n";
    isAliveM = false;

    while (!tcpAppConnMapM.empty()) {
        auto i = tcpAppConnMapM.begin();
        delete (*i).second.pNscSocketM;
        tcpAppConnMapM.erase(i);
    }

    // statistics
    delete sndNxtVector;
    delete sndAckVector;
    delete rcvSeqVector;
    delete rcvAckVector;
}

// send a TCP_I_ESTABLISHED msg to Application Layer
void TCP_NSC::sendEstablishedMsg(TCP_NSC_Connection& connP)
{
    if (connP.sentEstablishedM)
        return;

    cMessage *msg = new cMessage("TCP_I_ESTABLISHED");
    msg->setKind(TCP_I_ESTABLISHED);
    TCPConnectInfo *tcpConnectInfo = new TCPConnectInfo();
    tcpConnectInfo->setConnId(connP.connIdM);
    tcpConnectInfo->setLocalAddr(connP.inetSockPairM.localM.ipAddrM);
    tcpConnectInfo->setRemoteAddr(connP.inetSockPairM.remoteM.ipAddrM);
    tcpConnectInfo->setLocalPort(connP.inetSockPairM.localM.portM);
    tcpConnectInfo->setRemotePort(connP.inetSockPairM.remoteM.portM);
    msg->setControlInfo(tcpConnectInfo);
    send(msg, "appOut", connP.appGateIndexM);
    connP.sentEstablishedM = true;
}

void TCP_NSC::changeAddresses(TCP_NSC_Connection& connP,
        const TCP_NSC_Connection::SockPair& inetSockPairP,
        const TCP_NSC_Connection::SockPair& nscSockPairP)
{
    if (!(connP.inetSockPairM == inetSockPairP)) {
        EV_DEBUG << "conn:" << connP << " change inetMap from " << connP.inetSockPairM << " to " << inetSockPairP << "\n";
        inetSockPair2ConnIdMapM.erase(connP.inetSockPairM);
        connP.inetSockPairM = inetSockPairP;
        inetSockPair2ConnIdMapM[connP.inetSockPairM] = connP.connIdM;
    }

    if (!(connP.nscSockPairM == nscSockPairP)) {
        EV_DEBUG << "conn:" << connP << " change nscMap from " << connP.nscSockPairM << " to " << nscSockPairP << "\n";
        // remove old from map:
        nscSockPair2ConnIdMapM.erase(connP.nscSockPairM);
        // change addresses:
        connP.nscSockPairM = nscSockPairP;
        // and add to map:
        nscSockPair2ConnIdMapM[connP.nscSockPairM] = connP.connIdM;
    }
}

void TCP_NSC::handleIpInputMessage(TCPSegment *tcpsegP)
{
    // get src/dest addresses
    TCP_NSC_Connection::SockPair nscSockPair, inetSockPair, inetSockPairAny;

    cObject *ctrl = tcpsegP->removeControlInfo();
    if (!ctrl)
        throw cRuntimeError("(%s)%s arrived without control info", tcpsegP->getClassName(), tcpsegP->getName());

    INetworkProtocolControlInfo *controlInfo = check_and_cast<INetworkProtocolControlInfo *>(ctrl);
    inetSockPair.remoteM.ipAddrM = controlInfo->getSourceAddress();
    inetSockPair.localM.ipAddrM = controlInfo->getDestinationAddress();
    //int interfaceId = controlInfo->getInterfaceId();

    if (dynamic_cast<IPv6ControlInfo *>(ctrl) != nullptr) {
        {
            // HACK: when IPv6, then correcting the TCPOPTION_MAXIMUM_SEGMENT_SIZE option
            //       with IP header size difference
            unsigned short numOptions = tcpsegP->getHeaderOptionArraySize();
            for (unsigned short i = 0; i < numOptions; i++) {
                TCPOption* option = tcpsegP->getHeaderOption(i);
                if (option->getKind() == TCPOPTION_MAXIMUM_SEGMENT_SIZE) {
                    TCPOptionMaxSegmentSize *mssOption = dynamic_cast<TCPOptionMaxSegmentSize *>(option);
                    if (mssOption) {
                        unsigned int value = mssOption->getMaxSegmentSize();
                        value -= sizeof(struct nsc_ipv6hdr) - sizeof(struct nsc_iphdr);
                        mssOption->setMaxSegmentSize(value);
                    }
                }
            }
        }
    }
    delete ctrl;

    // statistics:
    if (rcvSeqVector)
        rcvSeqVector->record(tcpsegP->getSequenceNo());

    if (rcvAckVector)
        rcvAckVector->record(tcpsegP->getAckNo());

    inetSockPair.remoteM.portM = tcpsegP->getSrcPort();
    inetSockPair.localM.portM = tcpsegP->getDestPort();
    nscSockPair.remoteM.portM = tcpsegP->getSrcPort();
    nscSockPair.localM.portM = tcpsegP->getDestPort();
    inetSockPairAny.localM = inetSockPair.localM;

    // process segment
    size_t ipHdrLen = sizeof(nsc_iphdr);
    size_t const maxBufferSize = 4096;
    char *data = new char[maxBufferSize];
    memset(data, 0, maxBufferSize);
    uint32_t nscSrcAddr = mapRemote2Nsc(inetSockPair.remoteM.ipAddrM);
    nscSockPair.localM.ipAddrM = localInnerIpS;
    nscSockPair.remoteM.ipAddrM.set(IPv4Address(nscSrcAddr));

    EV_DETAIL << this << ": data arrived for interface of stack " << pStackM << "\n"
              << "  src:" << inetSockPair.remoteM.ipAddrM << ",dest:" << inetSockPair.localM.ipAddrM << "\n";

    nsc_iphdr *ih = (nsc_iphdr *)data;
    tcphdr *tcph = (tcphdr *)(data + ipHdrLen);
    // set IP header:
    ih->version = 4;
    ih->ihl = ipHdrLen / 4;
    ih->tos = 0;
    ih->id = htons(tcpsegP->getSequenceNo());
    ih->frag_off = htons(0x4000);    // don't fragment, offset = 0;
    ih->ttl = 64;
    ih->protocol = 6;    // TCP
    ih->check = 0;
    ih->saddr = htonl(nscSrcAddr);
    ih->daddr = htonl(localInnerIpS.toIPv4().getInt());

    EV_DEBUG << this << ": modified to: IP " << ih->version << " len " << ih->ihl
             << " protocol " << (unsigned int)(ih->protocol)
             << " saddr " << (ih->saddr)
             << " daddr " << (ih->daddr)
             << "\n";

    size_t totalTcpLen = maxBufferSize - ipHdrLen;
    TCP_NSC_Connection *conn;
    conn = findConnByInetSockPair(inetSockPair);

    if (!conn) {
        conn = findConnByInetSockPair(inetSockPairAny);
    }

    { // local scope for 'b' and 'c'
        Buffer b(tcph, totalTcpLen);
        Context c;
        c.l3AddressesPtr = &ih->saddr;
        c.l3AddressesLength = 8;
        TCPSerializer().serializePacket(tcpsegP, b, c);
        totalTcpLen = b.getPos();
    }

    if (conn) {
        conn->receiveQueueM->notifyAboutIncomingSegmentProcessing(tcpsegP);
    }

    size_t totalIpLen = ipHdrLen + totalTcpLen;
    ih->tot_len = htons(totalIpLen);
    ih->check = 0;
    ih->check = htons(TCPIPchecksum::checksum(ih, ipHdrLen));

    // receive msg from network

    pStackM->if_receive_packet(0, data, totalIpLen);

    // Attempt to read from sockets
    int changes = 0;

    for (auto j = tcpAppConnMapM.begin(); j != tcpAppConnMapM.end(); ++j) {
        TCP_NSC_Connection& c = j->second;

        if (c.pNscSocketM && c.isListenerM) {
            // accepting socket
            EV_DETAIL << this << ": NSC: attempting to accept:\n";

            INetStreamSocket *sock = nullptr;
            int err;

            err = c.pNscSocketM->accept(&sock);

            EV_DETAIL << this << ": accept returned " << err << " , sock is " << sock
                      << " socket" << c.pNscSocketM << "\n";

            if (sock) {
                ASSERT(changes == 0);
                ASSERT(c.inetSockPairM.localM.portM == inetSockPair.localM.portM);
                ++changes;

                int newConnId = getEnvir()->getUniqueNumber();
                // add into appConnMap
                TCP_NSC_Connection *conn = &tcpAppConnMapM[newConnId];
                conn->tcpNscM = this;
                conn->connIdM = newConnId;
                conn->appGateIndexM = c.appGateIndexM;
                conn->pNscSocketM = sock;

                // set sockPairs:
                changeAddresses(*conn, inetSockPair, nscSockPair);

                // following code to be kept consistent with initConnection()
                const char *sendQueueClass = c.sendQueueM->getClassName();
                conn->sendQueueM = check_and_cast<TCP_NSC_SendQueue *>(inet::utils::createOne(sendQueueClass));
                conn->sendQueueM->setConnection(conn);

                const char *receiveQueueClass = c.receiveQueueM->getClassName();
                conn->receiveQueueM = check_and_cast<TCP_NSC_ReceiveQueue *>(inet::utils::createOne(receiveQueueClass));
                conn->receiveQueueM->setConnection(conn);
                EV_DETAIL << this << ": NSC: got accept!\n";

                sendEstablishedMsg(*conn);
            }
        }
        else if (c.pNscSocketM && !c.disconnectCalledM && c.pNscSocketM->is_connected()) {    // not listener
            bool hasData = false;
            int err = NSC_EAGAIN;
            EV_DEBUG << this << ": NSC: attempting to read from socket " << c.pNscSocketM << "\n";

            if ((!c.sentEstablishedM) && c.pNscSocketM->is_connected()) {
                ASSERT(changes == 0);
                hasData = true;
                changeAddresses(c, inetSockPair, nscSockPair);
                sendEstablishedMsg(c);
            }

            while (true) {
                static char buf[4096];

                int buflen = sizeof(buf);

                err = c.pNscSocketM->read_data(buf, &buflen);

                EV_DEBUG << this << ": NSC: read: err " << err << " , buflen " << buflen << "\n";

                if (err == 0 && buflen > 0) {
                    ASSERT(changes == 0);
                    if (!hasData)
                        changeAddresses(c, inetSockPair, nscSockPair);

                    hasData = true;
                    c.receiveQueueM->enqueueNscData(buf, buflen);
                    err = NSC_EAGAIN;
                }
                else
                    break;
            }

            if (hasData) {
                sendDataToApp(c);
                ++changes;
                changeAddresses(c, inetSockPair, nscSockPair);
            }
            sendErrorNotificationToApp(c, err);
        }
    }

    /*
       ...
       NSC: process segment (data,len); should call removeConnection() if socket has
       closed and completely done

       XXX: probably need to poll sockets to see if they are closed.
       ...
     */

    delete[] data;
    delete tcpsegP;
}

void TCP_NSC::sendDataToApp(TCP_NSC_Connection& c)
{
    cPacket *dataMsg;

    while (nullptr != (dataMsg = c.receiveQueueM->extractBytesUpTo())) {
        TCPConnectInfo *tcpConnectInfo = new TCPConnectInfo();
        tcpConnectInfo->setConnId(c.connIdM);
        tcpConnectInfo->setLocalAddr(c.inetSockPairM.localM.ipAddrM);
        tcpConnectInfo->setRemoteAddr(c.inetSockPairM.remoteM.ipAddrM);
        tcpConnectInfo->setLocalPort(c.inetSockPairM.localM.portM);
        tcpConnectInfo->setRemotePort(c.inetSockPairM.remoteM.portM);
        dataMsg->setControlInfo(tcpConnectInfo);
        // send Msg to Application layer:
        send(dataMsg, "appOut", c.appGateIndexM);
    }
}

void TCP_NSC::sendErrorNotificationToApp(TCP_NSC_Connection& c, int err)
{
    int code = -1;
    const char *name;
    switch (err) {
        case 0:
            code = TCP_I_PEER_CLOSED;
            name = "PEER_CLOSED";
            break;

        case NSC_ECONNREFUSED:
            code = TCP_I_CONNECTION_REFUSED;
            name = "CONNECTION_REFUSED";
            break;

        case NSC_ECONNRESET:
            code = TCP_I_CONNECTION_RESET;
            name = "CONNECTION_RESET";
            break;

        case NSC_ETIMEDOUT:
            code = TCP_I_TIMED_OUT;
            name = "TIMED_OUT";
            break;

        case NSC_EAGAIN:
            code = -1;
            name = "";
            break;

        default:
            throw cRuntimeError("Unknown NSC error returned by read_data(): %d", err);
            break;
    }
    if (code != -1) {
        cMessage *msg = new cMessage(name);
        msg->setKind(code);
        TCPCommand *ind = new TCPCommand();
                    ind->setConnId(c.connIdM);
        msg->setControlInfo(ind);
                    send(msg, "appOut", c.appGateIndexM);
    }
}

TCP_NSC_SendQueue *TCP_NSC::createSendQueue(TCPDataTransferMode transferModeP)
{
    switch (transferModeP) {
        case TCP_TRANSFER_BYTECOUNT:
            return new TCP_NSC_VirtualDataSendQueue();

        case TCP_TRANSFER_BYTESTREAM:
            return new TCP_NSC_ByteStreamSendQueue();

        case TCP_TRANSFER_OBJECT:    //return new TCP_NSC_MsgBasedSendQueue();
        default:
            throw cRuntimeError("Invalid TCP data transfer mode: %d at %s", transferModeP, this->getFullPath().c_str());
    }
}

TCP_NSC_ReceiveQueue *TCP_NSC::createReceiveQueue(TCPDataTransferMode transferModeP)
{
    switch (transferModeP) {
        case TCP_TRANSFER_BYTECOUNT:
            return new TCP_NSC_VirtualDataReceiveQueue();

        case TCP_TRANSFER_BYTESTREAM:
            return new TCP_NSC_ByteStreamReceiveQueue();

        case TCP_TRANSFER_OBJECT:    //return new TCP_NSC_MsgBasedReceiveQueue();
        default:
            throw cRuntimeError("Invalid TCP data transfer mode: %d at %s", transferModeP, this->getFullPath().c_str());
    }
}

void TCP_NSC::handleAppMessage(cMessage *msgP)
{
    TCPCommand *controlInfo = check_and_cast<TCPCommand *>(msgP->getControlInfo());
    int connId = controlInfo->getConnId();

    TCP_NSC_Connection *conn = findAppConn(connId);

    if (!conn) {
        TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(controlInfo);

        // add into appConnMap
        conn = &tcpAppConnMapM[connId];
        conn->tcpNscM = this;
        conn->connIdM = connId;
        conn->appGateIndexM = msgP->getArrivalGate()->getIndex();
        conn->pNscSocketM = nullptr;    // will be filled in within processAppCommand()

        TCPDataTransferMode transferMode = (TCPDataTransferMode)(openCmd->getDataTransferMode());
        // create send queue
        conn->sendQueueM = createSendQueue(transferMode);
        conn->sendQueueM->setConnection(conn);

        // create receive queue
        conn->receiveQueueM = createReceiveQueue(transferMode);
        conn->receiveQueueM->setConnection(conn);

        EV_INFO << this << ": TCP connection created for " << msgP << "\n";
    }

    processAppCommand(*conn, msgP);
}

void TCP_NSC::handleMessage(cMessage *msgP)
{
    if (msgP->isSelfMessage()) {
        // timer expired
        /*
           ...
           NSC timer processing
           ...
           Timers are ordinary cMessage objects that are started by
           scheduleAt(simTime()+timeout, msg), and can be cancelled
           via cancelEvent(msg); when they expire (fire) they are delivered
           to the module via handleMessage(), i.e. they end up here.
         */
        if (msgP == pNsiTimerM) {    // nsc_nsi_timer
            do_SEND_all();

            pStackM->increment_ticks();

            pStackM->timer_interrupt();

            scheduleAt(msgP->getArrivalTime() + 1.0 / (double)pStackM->get_hz(), msgP);
        }
    }
    else if (msgP->arrivedOn("ipIn")) {
        if (false
#ifdef WITH_IPv4
            || dynamic_cast<ICMPMessage *>(msgP)
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
            || dynamic_cast<ICMPv6Message *>(msgP)
#endif // ifdef WITH_IPv6
            )
        {
            EV_WARN << "ICMP error received -- discarding\n";    // FIXME can ICMP packets really make it up to TCP???
            delete msgP;
        }
        else {
            EV_DEBUG << this << ": handle msg: " << msgP->getName() << "\n";
            // must be a TCPSegment
            TCPSegment *tcpseg = check_and_cast<TCPSegment *>(msgP);
            handleIpInputMessage(tcpseg);
        }
    }
    else {    // must be from app
        EV_DEBUG << this << ": handle msg: " << msgP->getName() << "\n";
        handleAppMessage(msgP);
    }

    if (hasGUI())
        updateDisplayString();
}

void TCP_NSC::updateDisplayString()
{
    //...
}

TCP_NSC_Connection *TCP_NSC::findAppConn(int connIdP)
{
    auto i = tcpAppConnMapM.find(connIdP);
    return i == tcpAppConnMapM.end() ? nullptr : &(i->second);
}

TCP_NSC_Connection *TCP_NSC::findConnByInetSockPair(TCP_NSC_Connection::SockPair const& sockPairP)
{
    auto i = inetSockPair2ConnIdMapM.find(sockPairP);
    return i == inetSockPair2ConnIdMapM.end() ? nullptr : findAppConn(i->second);
}

TCP_NSC_Connection *TCP_NSC::findConnByNscSockPair(TCP_NSC_Connection::SockPair const& sockPairP)
{
    auto i = nscSockPair2ConnIdMapM.find(sockPairP);
    return i == nscSockPair2ConnIdMapM.end() ? nullptr : findAppConn(i->second);
}

void TCP_NSC::finish()
{
    isAliveM = false;
}

void TCP_NSC::removeConnection(int connIdP)
{
    auto i = tcpAppConnMapM.find(connIdP);
    if (i != tcpAppConnMapM.end())
        tcpAppConnMapM.erase(i);
}

void TCP_NSC::printConnBrief(TCP_NSC_Connection& connP)
{
    EV_DEBUG << this << ": connId=" << connP.connIdM << " appGateIndex=" << connP.appGateIndexM
             << " nscsocket=" << connP.pNscSocketM << "\n";
}

void TCP_NSC::loadStack(const char *stacknameP, int bufferSizeP)
{
    void *handle = nullptr;
    FCreateStack create = nullptr;

    EV_DETAIL << this << ": Loading stack " << stacknameP << "\n";

    handle = dlopen(stacknameP, RTLD_NOW);

    if (!handle) {
        throw cRuntimeError("The loading of '%s' NSC stack is unsuccessful: %s. Check the LD_LIBRARY_PATH or stackname!", stacknameP, dlerror());
    }

    create = (FCreateStack)dlsym(handle, "nsc_create_stack");

    if (!create) {
        throw cRuntimeError("The '%s' NSC stack creation unsuccessful: %s", stacknameP, dlerror());
    }

    pStackM = create(this, this, nullptr);

    EV_DETAIL << "TCP_NSC " << this << " has stack " << pStackM << "\n";

    EV_DETAIL << "TCP_NSC " << this << "Initializing stack, name=" << pStackM->get_name() << endl;

    pStackM->init(pStackM->get_hz());

    pStackM->buffer_size(bufferSizeP);

    EV_INFO << "TCP_NSC " << this << "Stack initialized, name=" << pStackM->get_name() << endl;

    // set timer for 1.0 / pStackM->get_hz()
    pNsiTimerM = new cMessage("nsc_nsi_timer");
    scheduleAt(1.0 / (double)pStackM->get_hz(), pNsiTimerM);
}

/** Called from the stack when a packet needs to be output to the wire. */
void TCP_NSC::send_callback(const void *dataP, int datalenP)
{
    if (!isAliveM)
        return;

    EV_DETAIL << this << ": NSC: send_callback(" << dataP << ", " << datalenP << ") called\n";

    sendToIP(dataP, datalenP);

    // comment from nsc.cc:
    //   New method: if_send_finish. This should be called by the nic
    //   driver code when there is space in it's queue. Unfortunately
    //   we don't know whether that is the case from here, so we just
    //   call it immediately for now. **THIS IS INCORRECT**. It should
    //   only be called when a space becomes free in the nic queue.
    pStackM->if_send_finish(0);    // XXX: hardcoded inerface id
}

/*
   void TCP_NSC::interrupt()
   {
   }
 */

/** This is called when something interesting happened to a socket. Perhaps
 * a socket can now be written to or read from or something. We cannot
 * directly do things like read or write from the socket, because this is
 * not what happens in the real FreeBSD: it normally puts the application or
 * thread blocking on the socket in question on the run queue.
 *
 * To simulate this behaviour we wait a very small amount of time (this is
 * sort of simulating processing delay) then call wakeup_expire. The
 * important thing is that we return here and actually do the reading/writing
 * at a later time.
 */
void TCP_NSC::wakeup()
{
    if (!isAliveM)
        return;

    EV_TRACE << this << ": wakeup() called\n";
}

void TCP_NSC::gettime(unsigned int *secP, unsigned int *usecP)
{
#ifdef USE_DOUBLE_SIMTIME
    double t = simTime().dbl();
    *sec = (unsigned int)(t);
    *usec = (unsigned int)((t - *sec) * 1000000 + 0.5);
#else // ifdef USE_DOUBLE_SIMTIME
    simtime_t t = simTime();
    int64 raw = t.raw();
    int64 scale = t.getScale();
    int64 secs = raw / scale;
    int64 usecs = (raw - (secs * scale));

    //usecs = usecs * 1000000 / scale;
    if (scale > 1000000) // scale always 10^n
        usecs /= (scale / 1000000);
    else
        usecs *= (1000000 / scale);

    *secP = secs;
    *usecP = usecs;
#endif // ifdef USE_DOUBLE_SIMTIME

    EV_TRACE << this << ": gettime(" << *secP << "," << *usecP << ") called\n";
}

void TCP_NSC::sendToIP(const void *dataP, int lenP)
{
    L3Address src, dest;
    const nsc_iphdr *iph = (const nsc_iphdr *)dataP;

    int ipHdrLen = 4 * iph->ihl;
    ASSERT(ipHdrLen <= lenP);
    int totalLen = ntohs(iph->tot_len);
    ASSERT(totalLen == lenP);
    tcphdr const *tcph = (tcphdr const *)(((const char *)(iph)) + ipHdrLen);

    // XXX add some info (seqNo, len, etc)

    TCP_NSC_Connection::SockPair nscSockPair;
    TCP_NSC_Connection *conn;

    nscSockPair.localM.ipAddrM.set(IPv4Address(ntohl(iph->saddr)));
    nscSockPair.localM.portM = ntohs(tcph->th_sport);
    nscSockPair.remoteM.ipAddrM.set(IPv4Address(ntohl(iph->daddr)));
    nscSockPair.remoteM.portM = ntohs(tcph->th_dport);

    if (curConnM) {
        changeAddresses(*curConnM, curConnM->inetSockPairM, nscSockPair);
        conn = curConnM;
    }
    else {
        conn = findConnByNscSockPair(nscSockPair);
    }

    TCPSegment *tcpseg;

    if (conn) {
        tcpseg = conn->sendQueueM->createSegmentWithBytes(tcph, totalLen - ipHdrLen);
        src = conn->inetSockPairM.localM.ipAddrM;
        dest = conn->inetSockPairM.remoteM.ipAddrM;
    }
    else {
        tcpseg = TCPSerializer().deserialize((const unsigned char *)tcph, totalLen - ipHdrLen, true);
        dest = mapNsc2Remote(ntohl(iph->daddr));
    }

    ASSERT(tcpseg);

    EV_TRACE << this << ": Sending: conn=" << conn << ", data: " << dataP << " of len " << lenP << " from " << src
             << " to " << dest << "\n";

    IL3AddressType *addressType = dest.getAddressType();
    INetworkProtocolControlInfo *controlInfo = addressType->createNetworkProtocolControlInfo();
    controlInfo->setTransportProtocol(IP_PROT_TCP);
    controlInfo->setSourceAddress(src);
    controlInfo->setDestinationAddress(dest);
    tcpseg->setControlInfo(check_and_cast<cObject *>(controlInfo));

    if (conn) {
        conn->receiveQueueM->notifyAboutSending(tcpseg);
    }

    // record seq (only if we do send data) and ackno
    if (sndNxtVector && tcpseg->getPayloadLength() != 0)
        sndNxtVector->record(tcpseg->getSequenceNo());

    if (sndAckVector)
        sndAckVector->record(tcpseg->getAckNo());

    int connId = conn ? conn->connIdM : -1;
    EV_INFO << this << ": Send segment: conn ID=" << connId << " from " << src
            << " to " << dest << " SEQ=" << tcpseg->getSequenceNo();
    if (tcpseg->getSynBit())
        EV_INFO << " SYN";
    if (tcpseg->getAckBit())
        EV_INFO << " ACK=" << tcpseg->getAckNo();
    if (tcpseg->getFinBit())
        EV_INFO << " FIN";
    if (tcpseg->getRstBit())
        EV_INFO << " RST";
    if (tcpseg->getPshBit())
        EV_INFO << " PSH";
    if (tcpseg->getUrgBit())
        EV_INFO << " URG";
    EV_INFO << " len=" << tcpseg->getPayloadLength() << "\n";

    send(tcpseg, "ipOut");
}

void TCP_NSC::processAppCommand(TCP_NSC_Connection& connP, cMessage *msgP)
{
    printConnBrief(connP);

    // first do actions
    TCPCommand *tcpCommand = (TCPCommand *)(msgP->removeControlInfo());

    switch (msgP->getKind()) {
        case TCP_C_OPEN_ACTIVE:
            process_OPEN_ACTIVE(connP, tcpCommand, msgP);
            break;

        case TCP_C_OPEN_PASSIVE:
            process_OPEN_PASSIVE(connP, tcpCommand, msgP);
            break;

        case TCP_C_SEND:
            process_SEND(connP, tcpCommand, check_and_cast<cPacket *>(msgP));
            break;

        case TCP_C_CLOSE:
            process_CLOSE(connP, tcpCommand, msgP);
            break;

        case TCP_C_ABORT:
            process_ABORT(connP, tcpCommand, msgP);
            break;

        case TCP_C_STATUS:
            process_STATUS(connP, tcpCommand, msgP);
            break;

        default:
            throw cRuntimeError("Wrong command from app: %d", msgP->getKind());
    }

    /*
       ...
       NSC: should call removeConnection(connP.connIdM) if socket has been destroyed
       ...
     */
}

void TCP_NSC::process_OPEN_ACTIVE(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(tcpCommandP);

    TCP_NSC_Connection::SockPair inetSockPair, nscSockPair;
    inetSockPair.localM.ipAddrM = openCmd->getLocalAddr();
    inetSockPair.remoteM.ipAddrM = openCmd->getRemoteAddr();
    inetSockPair.localM.portM = openCmd->getLocalPort();
    inetSockPair.remoteM.portM = openCmd->getRemotePort();

    if (inetSockPair.remoteM.ipAddrM.isUnspecified() || inetSockPair.remoteM.portM == PORT_UNDEF)
        throw cRuntimeError("Error processing command OPEN_ACTIVE: remote address and port must be specified");

    EV_INFO << this << ": OPEN: "
            << inetSockPair.localM.ipAddrM << ":" << inetSockPair.localM.portM << " --> "
            << inetSockPair.remoteM.ipAddrM << ":" << inetSockPair.remoteM.portM << "\n";

    // insert to map:
    uint32_t nscRemoteAddr = mapRemote2Nsc(inetSockPair.remoteM.ipAddrM);

    ASSERT(pStackM);

    nscSockPair.localM.portM = inetSockPair.localM.portM;
    if (nscSockPair.localM.portM == PORT_UNDEF)
        nscSockPair.localM.portM = 0; // NSC uses 0 to mean "not specified"
    nscSockPair.remoteM.ipAddrM.set(IPv4Address(nscRemoteAddr));
    nscSockPair.remoteM.portM = inetSockPair.remoteM.portM;

    changeAddresses(connP, inetSockPair, nscSockPair);

    ASSERT(!curConnM);
    curConnM = &connP;
    connP.connect(*pStackM, inetSockPair, nscSockPair);
    curConnM = nullptr;

    // and add to map:
    // TODO sendToIp already set the addresses.
    //changeAddresses(connP, inetSockPair, nscSockPair);

    delete tcpCommandP;
    delete msgP;
}

void TCP_NSC::process_OPEN_PASSIVE(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    ASSERT(pStackM);

    TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(tcpCommandP);

    if (!openCmd->getFork())
        throw cRuntimeError("TCP_NSC supports Forking mode only");

    TCP_NSC_Connection::SockPair inetSockPair, nscSockPair;
    inetSockPair.localM.ipAddrM = openCmd->getLocalAddr();
    inetSockPair.remoteM.ipAddrM = openCmd->getRemoteAddr();
    inetSockPair.localM.portM = openCmd->getLocalPort();
    inetSockPair.remoteM.portM = openCmd->getRemotePort();

    uint32_t nscRemoteAddr = inetSockPair.remoteM.ipAddrM.isUnspecified()
        ? ntohl(INADDR_ANY)
        : mapRemote2Nsc(inetSockPair.remoteM.ipAddrM);    // Don't remove! It's insert remoteAddr into MAP.

    (void)nscRemoteAddr;    // Eliminate "unused variable" warning.

    if (inetSockPair.localM.portM == PORT_UNDEF)
        throw cRuntimeError("Error processing command OPEN_PASSIVE: local port must be specified");

    EV_INFO << this << "Starting to listen on: " << inetSockPair.localM.ipAddrM << ":" << inetSockPair.localM.portM << "\n";

    /*
       ...
       NSC: process passive open request
       ...
     */
    nscSockPair.localM.portM = inetSockPair.localM.portM;

    changeAddresses(connP, inetSockPair, nscSockPair);

    ASSERT(!curConnM);
    curConnM = &connP;
    connP.listen(*pStackM, inetSockPair, nscSockPair);
    curConnM = nullptr;

    changeAddresses(connP, inetSockPair, nscSockPair);

    delete openCmd;
    delete msgP;
}

void TCP_NSC::process_SEND(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cPacket *msgP)
{
    TCPSendCommand *sendCommand = check_and_cast<TCPSendCommand *>(tcpCommandP);
    delete sendCommand;

    connP.send(msgP);

    connP.do_SEND();
}

void TCP_NSC::do_SEND_all()
{
    for (auto j = tcpAppConnMapM.begin(); j != tcpAppConnMapM.end(); ++j) {
        TCP_NSC_Connection& conn = j->second;
        conn.do_SEND();
    }
}

void TCP_NSC::process_CLOSE(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": process_CLOSE()\n";

    delete tcpCommandP;
    delete msgP;

    connP.close();
}

void TCP_NSC::process_ABORT(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": process_ABORT()\n";

    delete tcpCommandP;
    delete msgP;

    connP.abort();
}

void TCP_NSC::process_STATUS(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    delete tcpCommandP;    // but we'll reuse msg for reply

    TCPStatusInfo *statusInfo = new TCPStatusInfo();

    /*
       ...
       NSC: fill in status if possible

         Still more work to do here. Some of this needs to be implemented on
         the NSC side, which is fairly trivial, just updating the get_var
         function for each stack.
       ...
     */

    char result[512];

    ASSERT(connP.pNscSocketM);

    connP.pNscSocketM->get_var("cwnd_", result, sizeof(result));
    statusInfo->setSnd_wnd(atoi(result));

    statusInfo->setLocalAddr(connP.inetSockPairM.localM.ipAddrM);
    statusInfo->setRemoteAddr(connP.inetSockPairM.remoteM.ipAddrM);
    statusInfo->setLocalPort(connP.inetSockPairM.localM.portM);
    statusInfo->setRemotePort(connP.inetSockPairM.remoteM.portM);
    //connP.pNscSocketM->get_var("ssthresh_", result, sizeof(result));
    //connP.pNscSocketM->get_var("rxtcur_", result, sizeof(result));

/* other TCP model:
    statusInfo->setState(fsm.getState());
    statusInfo->setStateName(stateName(fsm.getState()));


    statusInfo->setSnd_mss(state->snd_mss);
    statusInfo->setSnd_una(state->snd_una);
    statusInfo->setSnd_nxt(state->snd_nxt);
    statusInfo->setSnd_max(state->snd_max);
    statusInfo->setSnd_wnd(state->snd_wnd);
    statusInfo->setSnd_up(state->snd_up);
    statusInfo->setSnd_wl1(state->snd_wl1);
    statusInfo->setSnd_wl2(state->snd_wl2);
    statusInfo->setIss(state->iss);
    statusInfo->setRcv_nxt(state->rcv_nxt);
    statusInfo->setRcv_wnd(state->rcv_wnd);
    statusInfo->setRcv_up(state->rcv_up);
    statusInfo->setIrs(state->irs);
    statusInfo->setFin_ack_rcvd(state->fin_ack_rcvd);
 */

    msgP->setControlInfo(statusInfo);
    msgP->setKind(TCP_I_STATUS);
    send(msgP, "appOut", connP.appGateIndexM);
}

bool TCP_NSC::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

} // namespace tcp

} // namespace inet

