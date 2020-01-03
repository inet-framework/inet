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

#include "inet/transportlayer/tcp_nsc/TcpNsc.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#endif // ifdef WITH_IPv6

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_common/headers/tcphdr.h"
#include "inet/transportlayer/tcp_nsc/queues/TcpNscQueues.h"

#include <assert.h>
#include <dlfcn.h>
#include <netinet/in.h>

#include <sim_errno.h>

namespace inet {

namespace tcp {

Define_Module(TcpNsc);

//static member variables:
const L3Address TcpNsc::localInnerIpS("1.0.0.253");
const L3Address TcpNsc::localInnerMaskS("255.255.255.0");
const L3Address TcpNsc::localInnerGwS("1.0.0.254");
const L3Address TcpNsc::remoteFirstInnerIpS("2.0.0.1");

const char *TcpNsc::stackNameParamNameS = "stackName";
const char *TcpNsc::bufferSizeParamNameS = "stackBufferSize";

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

static std::ostream& operator<<(std::ostream& osP, const TcpNscConnection& connP)
{
    osP << "Conn={"
        << "connId=" << connP.connIdM
        << " nscsocket=" << connP.pNscSocketM
        << " sentEstablishedM=" << connP.sentEstablishedM
        << '}';
    return osP;
}

static std::ostream& operator<<(std::ostream& osP, const TcpNscConnection::SockAddr& sockAddrP)
{
    osP << sockAddrP.ipAddrM << ":" << sockAddrP.portM;
    return osP;
}

static std::ostream& operator<<(std::ostream& osP, const TcpNscConnection::SockPair& sockPairP)
{
    osP << "{loc=" << sockPairP.localM << ", rem=" << sockPairP.remoteM << "}";
    return osP;
}

TcpNsc::TcpNsc()
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
uint32_t TcpNsc::mapRemote2Nsc(L3Address const& addrP)
{
    auto i = remote2NscMapM.find(addrP);
    if (i != remote2NscMapM.end()) {
        return i->second;
    }

    // get first free remote NSC IP
    uint32_t ret = remoteFirstInnerIpS.toIpv4().getInt();
    for (auto & elem : nsc2RemoteMapM) {
        if (elem.first > ret)
            break;
        ret = elem.first + 1;
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
L3Address const& TcpNsc::mapNsc2Remote(uint32_t nscAddrP)
{
    auto i = nsc2RemoteMapM.find(nscAddrP);

    if (i != nsc2RemoteMapM.end())
        return i->second;

    ASSERT(0);
    exit(1);
}

// x == mapNsc2Remote(mapRemote2Nsc(x))

void TcpNsc::initialize(int stage)
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

        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);

        WATCH_MAP(tcpAppConnMapM);

        // load the stack
        const char *stackName = this->par(stackNameParamNameS);
        int bufferSize = this->par(bufferSizeParamNameS);
        loadStack(stackName, bufferSize);
        pStackM->if_attach(localInnerIpS.str().c_str(), localInnerMaskS.str().c_str(), 1500);
        pStackM->add_default_gateway(localInnerGwS.str().c_str());
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
        registerService(Protocol::tcp, gate("appIn"), gate("ipIn"));
        registerProtocol(Protocol::tcp, gate("ipOut"), gate("appOut"));

        if (crcMode == CRC_COMPUTED) {
#ifdef WITH_IPv4
            auto ipv4 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv4.ip"));
            if (ipv4 != nullptr)
                ipv4->registerHook(0, &crcInsertion);
#endif
#ifdef WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv6.ipv6"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, &crcInsertion);
#endif
        }
    }
    else if (stage == INITSTAGE_LAST) {
        isAliveM = true;
    }
}

TcpNsc::~TcpNsc()
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
void TcpNsc::sendEstablishedMsg(TcpNscConnection& connP)
{
    auto indication = new Indication("TCP_I_ESTABLISHED", TCP_I_ESTABLISHED);
    TcpConnectInfo *tcpConnectInfo = new TcpConnectInfo();
    tcpConnectInfo->setLocalAddr(connP.inetSockPairM.localM.ipAddrM);
    tcpConnectInfo->setRemoteAddr(connP.inetSockPairM.remoteM.ipAddrM);
    tcpConnectInfo->setLocalPort(connP.inetSockPairM.localM.portM);
    tcpConnectInfo->setRemotePort(connP.inetSockPairM.remoteM.portM);
    indication->setControlInfo(tcpConnectInfo);
    indication->addTag<TransportProtocolInd>()->setProtocol(&Protocol::tcp);
    indication->addTag<SocketInd>()->setSocketId(connP.connIdM);
    send(indication, "appOut");
    connP.sentEstablishedM = true;
}

void TcpNsc::sendAvailableIndicationMsg(TcpNscConnection& c)
{
    auto *indication = new Indication("TCP_I_AVAILABLE", TCP_I_AVAILABLE);
    TcpAvailableInfo *tcpConnectInfo = new TcpAvailableInfo();

    ASSERT(c.forkedConnId != -1);
    tcpConnectInfo->setNewSocketId(c.connIdM);
    tcpConnectInfo->setLocalAddr(c.inetSockPairM.localM.ipAddrM);
    tcpConnectInfo->setRemoteAddr(c.inetSockPairM.remoteM.ipAddrM);
    tcpConnectInfo->setLocalPort(c.inetSockPairM.localM.portM);
    tcpConnectInfo->setRemotePort(c.inetSockPairM.remoteM.portM);

    indication->setControlInfo(tcpConnectInfo);
    indication->addTag<TransportProtocolInd>()->setProtocol(&Protocol::tcp);
    indication->addTag<SocketInd>()->setSocketId(c.forkedConnId);
    send(indication, "appOut");
    c.sentEstablishedM = true;
}

void TcpNsc::changeAddresses(TcpNscConnection& connP,
        const TcpNscConnection::SockPair& inetSockPairP,
        const TcpNscConnection::SockPair& nscSockPairP)
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

void TcpNsc::handleIpInputMessage(Packet *packet)
{
    // get src/dest addresses
    TcpNscConnection::SockPair nscSockPair, inetSockPair, inetSockPairAny;

    inetSockPair.remoteM.ipAddrM = packet->getTag<L3AddressInd>()->getSrcAddress();
    inetSockPair.localM.ipAddrM = packet->getTag<L3AddressInd>()->getDestAddress();
    //int interfaceId = controlInfo->getInterfaceId();

    if (packet->getTag<NetworkProtocolInd>()->getProtocol()->getId() == Protocol::ipv6.getId()) {
        const auto& tcpHdr = packet->peekAtFront<TcpHeader>();
        // HACK: when IPv6, then correcting the TCPOPTION_MAXIMUM_SEGMENT_SIZE option
        //       with IP header size difference
        unsigned short numOptions = tcpHdr->getHeaderOptionArraySize();
        for (unsigned short i = 0; i < numOptions; i++) {
            if (tcpHdr->getHeaderOption(i)->getKind() == TCPOPTION_MAXIMUM_SEGMENT_SIZE) {
                auto newTcpHdr = staticPtrCast<TcpHeader>(tcpHdr->dupShared());
                TcpOption* option = newTcpHdr->getHeaderOptionForUpdate(i);
                TcpOptionMaxSegmentSize *mssOption = check_and_cast<TcpOptionMaxSegmentSize *>(option);
                unsigned int value = mssOption->getMaxSegmentSize();
                value -= sizeof(struct nsc_ipv6hdr) - sizeof(struct nsc_iphdr);
                mssOption->setMaxSegmentSize(value);
                packet->popAtFront<TcpHeader>();
                packet->trim();
                packet->insertAtFront(newTcpHdr);
                break;
            }
        }
    }

    auto tcpHdr = packet->peekAtFront<TcpHeader>();

    switch(tcpHdr->getCrcMode()) {
        case CRC_DECLARED_INCORRECT:
            EV_WARN << "CRC error, packet dropped\n";
            delete packet;
            return;
        case CRC_DECLARED_CORRECT: {
            // modify to calculated, for serializing
            packet->trimFront();
            const auto& newTcpHdr = packet->removeAtFront<TcpHeader>();
            newTcpHdr->setCrcMode(CRC_COMPUTED);
            newTcpHdr->setCrc(0);
            packet->insertAtFront(newTcpHdr);
            tcpHdr = newTcpHdr;
            break;
        }
        default:
            break;
    }

    // statistics:
    if (rcvSeqVector)
        rcvSeqVector->record(tcpHdr->getSequenceNo());

    if (rcvAckVector)
        rcvAckVector->record(tcpHdr->getAckNo());

    inetSockPair.remoteM.portM = tcpHdr->getSrcPort();
    inetSockPair.localM.portM = tcpHdr->getDestPort();
    nscSockPair.remoteM.portM = tcpHdr->getSrcPort();
    nscSockPair.localM.portM = tcpHdr->getDestPort();
    inetSockPairAny.localM = inetSockPair.localM;

    // process segment
    size_t ipHdrLen = sizeof(nsc_iphdr);
    size_t const maxBufferSize = 4096;
    char *data = new char[maxBufferSize];
    memset(data, 0, maxBufferSize);
    uint32_t nscSrcAddr = mapRemote2Nsc(inetSockPair.remoteM.ipAddrM);
    nscSockPair.localM.ipAddrM = localInnerIpS;
    nscSockPair.remoteM.ipAddrM.set(Ipv4Address(nscSrcAddr));

    EV_DETAIL << this << ": data arrived for interface of stack " << pStackM << "\n"
              << "  src:" << inetSockPair.remoteM.ipAddrM << ",dest:" << inetSockPair.localM.ipAddrM << "\n";

    nsc_iphdr *ih = (nsc_iphdr *)data;
    tcphdr *tcph = (tcphdr *)(data + ipHdrLen);
    // set IP header:
    ih->version = 4;
    ih->ihl = ipHdrLen / 4;
    ih->tos = 0;
    ih->id = htons(tcpHdr->getSequenceNo());
    ih->frag_off = htons(0x4000);    // don't fragment, offset = 0;
    ih->ttl = 64;
    ih->protocol = 6;    // TCP
    ih->check = 0;
    ih->saddr = htonl(nscSrcAddr);
    ih->daddr = htonl(localInnerIpS.toIpv4().getInt());

    EV_DEBUG << this << ": modified to: IP " << ih->version << " len " << ih->ihl
             << " protocol " << (unsigned int)(ih->protocol)
             << " saddr " << (ih->saddr)
             << " daddr " << (ih->daddr)
             << "\n";

    size_t totalTcpLen = maxBufferSize - ipHdrLen;
    TcpNscConnection *conn;
    conn = findConnByInetSockPair(inetSockPair);

    if (!conn) {
        conn = findConnByInetSockPair(inetSockPairAny);
    }

    const auto& bytes = packet->peekDataAsBytes();
    totalTcpLen = bytes->copyToBuffer((uint8_t *)tcph, totalTcpLen);

    if (conn) {
        conn->receiveQueueM->notifyAboutIncomingSegmentProcessing(packet);
    }

    size_t totalIpLen = ipHdrLen + totalTcpLen;
    ih->tot_len = htons(totalIpLen);
    ih->check = 0;
    ih->check = htons(TcpIpChecksum::checksum(ih, ipHdrLen));

    // receive msg from network

    pStackM->if_receive_packet(0, data, totalIpLen);

    // Attempt to read from sockets
    int changes = 0;

    for (auto & elem : tcpAppConnMapM) {
        TcpNscConnection& c = elem.second;

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
                TcpNscConnection *conn = &tcpAppConnMapM[newConnId];
                conn->tcpNscM = this;
                conn->connIdM = newConnId;
                conn->forkedConnId = c.connIdM;
                conn->pNscSocketM = sock;

                // set sockPairs:
                changeAddresses(*conn, inetSockPair, nscSockPair);

                // following code to be kept consistent with initConnection()
                conn->sendQueueM = new TcpNscSendQueue();
                conn->sendQueueM->setConnection(conn);

                conn->receiveQueueM = new TcpNscReceiveQueue();
                conn->receiveQueueM->setConnection(conn);
                EV_DETAIL << this << ": NSC: got accept!\n";

                sendAvailableIndicationMsg(*conn);
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

            while (c.forkedConnId == -1) {      // not a forked listener socket, or ACCEPT arrived after fork) {
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
    delete packet;
}

void TcpNsc::sendDataToApp(TcpNscConnection& c)
{
    Packet *dataMsg;

    while (nullptr != (dataMsg = c.receiveQueueM->extractBytesUpTo())) {
        dataMsg->setKind(TCP_I_DATA);
        dataMsg->addTag<SocketInd>()->setSocketId(c.connIdM);
        // send Msg to Application layer:
        send(dataMsg, "appOut");
    }
}

void TcpNsc::sendErrorNotificationToApp(TcpNscConnection& c, int err)
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
        auto indication = new Indication(name, code);
        TcpCommand *ind = new TcpCommand();
        indication->setControlInfo(ind);
        indication->addTag<PacketProtocolTag>()->setProtocol(&Protocol::tcp);
        indication->addTag<SocketInd>()->setSocketId(c.connIdM);
        send(indication, "appOut");
    }
}

TcpNscSendQueue *TcpNsc::createSendQueue()
{
    return new TcpNscSendQueue();
}

TcpNscReceiveQueue *TcpNsc::createReceiveQueue()
{
    return new TcpNscReceiveQueue();
}

void TcpNsc::handleAppMessage(cMessage *msgP)
{
    auto& tags = getTags(msgP);
    int connId = tags.getTag<SocketReq>()->getSocketId();

    TcpNscConnection *conn = findAppConn(connId);

    if (!conn) {
        // add into appConnMap
        conn = &tcpAppConnMapM[connId];
        conn->tcpNscM = this;
        conn->connIdM = connId;
        conn->pNscSocketM = nullptr;    // will be filled in within processAppCommand()

        // create send queue
        conn->sendQueueM = createSendQueue();
        conn->sendQueueM->setConnection(conn);

        // create receive queue
        conn->receiveQueueM = createReceiveQueue();
        conn->receiveQueueM->setConnection(conn);

        EV_INFO << this << ": TCP connection created for " << msgP << "\n";
    }

    processAppCommand(*conn, msgP);
}

void TcpNsc::handleMessage(cMessage *msgP)
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
        Packet *pk = check_and_cast<Packet *>(msgP);
        auto protocol = pk->getTag<PacketProtocolTag>()->getProtocol();
        if (protocol == &Protocol::tcp) {
            EV_TRACE << this << ": handle tcp segment: " << msgP->getName() << "\n";
            handleIpInputMessage(pk);
        }
        else if (protocol == &Protocol::icmpv4 || protocol == &Protocol::icmpv6) {
            EV_WARN << "ICMP error received -- discarding\n";    // FIXME can ICMP packets really make it up to TCP???
            delete msgP;
        }
        else
            throw cRuntimeError("Unknown protocol: %s(%d)", protocol->getName(), protocol->getId());
    }
    else {    // must be from app
        EV_DEBUG << this << ": handle msg: " << msgP->getName() << "\n";
        handleAppMessage(msgP);
    }
}

void TcpNsc::refreshDisplay() const
{
    //...
}

TcpNscConnection *TcpNsc::findAppConn(int connIdP)
{
    auto i = tcpAppConnMapM.find(connIdP);
    return i == tcpAppConnMapM.end() ? nullptr : &(i->second);
}

TcpNscConnection *TcpNsc::findConnByInetSockPair(TcpNscConnection::SockPair const& sockPairP)
{
    auto i = inetSockPair2ConnIdMapM.find(sockPairP);
    return i == inetSockPair2ConnIdMapM.end() ? nullptr : findAppConn(i->second);
}

TcpNscConnection *TcpNsc::findConnByNscSockPair(TcpNscConnection::SockPair const& sockPairP)
{
    auto i = nscSockPair2ConnIdMapM.find(sockPairP);
    return i == nscSockPair2ConnIdMapM.end() ? nullptr : findAppConn(i->second);
}

void TcpNsc::finish()
{
    isAliveM = false;
}

void TcpNsc::removeConnection(int connIdP)
{
    auto i = tcpAppConnMapM.find(connIdP);
    if (i != tcpAppConnMapM.end())
        tcpAppConnMapM.erase(i);
}

void TcpNsc::printConnBrief(TcpNscConnection& connP)
{
    EV_DEBUG << this << ": connId=" << connP.connIdM << " nscsocket=" << connP.pNscSocketM << "\n";
}

void TcpNsc::loadStack(const char *stacknameP, int bufferSizeP)
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

    EV_DETAIL << "TcpNsc " << this << " has stack " << pStackM << "\n";

    EV_DETAIL << "TcpNsc " << this << "Initializing stack, name=" << pStackM->get_name() << endl;

    pStackM->init(pStackM->get_hz());

    pStackM->buffer_size(bufferSizeP);

    EV_INFO << "TcpNsc " << this << "Stack initialized, name=" << pStackM->get_name() << endl;

    // set timer for 1.0 / pStackM->get_hz()
    pNsiTimerM = new cMessage("nsc_nsi_timer");
    scheduleAt(1.0 / (double)pStackM->get_hz(), pNsiTimerM);
}

/** Called from the stack when a packet needs to be output to the wire. */
void TcpNsc::send_callback(const void *dataP, int datalenP)
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
   void TcpNsc::interrupt()
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
void TcpNsc::wakeup()
{
    if (!isAliveM)
        return;

    EV_TRACE << this << ": wakeup() called\n";
}

void TcpNsc::gettime(unsigned int *secP, unsigned int *usecP)
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

void TcpNsc::sendToIP(const void *dataP, int lenP)
{
    L3Address src, dest;
    const nsc_iphdr *iph = (const nsc_iphdr *)dataP;

    int ipHdrLen = 4 * iph->ihl;
    ASSERT(ipHdrLen <= lenP);
    int totalLen = ntohs(iph->tot_len);
    ASSERT(totalLen == lenP);
    tcphdr const *tcph = (tcphdr const *)(((const char *)(iph)) + ipHdrLen);

    // XXX add some info (seqNo, len, etc)

    TcpNscConnection::SockPair nscSockPair;
    TcpNscConnection *conn;

    nscSockPair.localM.ipAddrM.set(Ipv4Address(ntohl(iph->saddr)));
    nscSockPair.localM.portM = ntohs(tcph->th_sport);
    nscSockPair.remoteM.ipAddrM.set(Ipv4Address(ntohl(iph->daddr)));
    nscSockPair.remoteM.portM = ntohs(tcph->th_dport);

    if (curConnM) {
        changeAddresses(*curConnM, curConnM->inetSockPairM, nscSockPair);
        conn = curConnM;
    }
    else {
        conn = findConnByNscSockPair(nscSockPair);
    }

    Packet *fp = nullptr;

    if (conn) {
        fp = conn->sendQueueM->createSegmentWithBytes(tcph, totalLen - ipHdrLen);
        src = conn->inetSockPairM.localM.ipAddrM;
        dest = conn->inetSockPairM.remoteM.ipAddrM;
    }
    else {
        const auto& bytes = makeShared<BytesChunk>((const uint8_t*)tcph, lenP - ipHdrLen);
        fp = new Packet(nullptr, bytes);
        const auto& tcpHdr = fp->popAtFront<TcpHeader>();
        fp->trimFront();
        int64_t numBytes = fp->getByteLength();
        ASSERT(numBytes == 0);
        fp->insertAtFront(tcpHdr);

        dest = mapNsc2Remote(ntohl(iph->daddr));
    }

    auto tcpHdr = fp->removeAtFront<TcpHeader>();
    tcpHdr->setCrc(0);
    ASSERT(crcMode == CRC_COMPUTED || crcMode == CRC_DECLARED_CORRECT);
    tcpHdr->setCrcMode(crcMode);
    insertTransportProtocolHeader(fp, Protocol::tcp, tcpHdr);


    b payloadLength = fp->getDataLength() - tcpHdr->getChunkLength();
    EV_TRACE << this << ": Sending: conn=" << conn << ", data: " << dataP << " of len " << lenP << " from " << src
             << " to " << dest << "\n";

    IL3AddressType *addressType = dest.getAddressType();
    fp->addTag<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    auto addresses = fp->addTag<L3AddressReq>();
    addresses->setSrcAddress(src);
    addresses->setDestAddress(dest);

    if (conn) {
        conn->receiveQueueM->notifyAboutSending(fp);
    }

    // record seq (only if we do send data) and ackno
    if (sndNxtVector && payloadLength != b(0))
        sndNxtVector->record(tcpHdr->getSequenceNo());

    if (sndAckVector)
        sndAckVector->record(tcpHdr->getAckNo());

    int connId = conn ? conn->connIdM : -1;
    EV_INFO << this << ": Send segment: conn ID=" << connId << " from " << src
            << " to " << dest << " SEQ=" << tcpHdr->getSequenceNo();
    if (tcpHdr->getSynBit())
        EV_INFO << " SYN";
    if (tcpHdr->getAckBit())
        EV_INFO << " ACK=" << tcpHdr->getAckNo();
    if (tcpHdr->getFinBit())
        EV_INFO << " FIN";
    if (tcpHdr->getRstBit())
        EV_INFO << " RST";
    if (tcpHdr->getPshBit())
        EV_INFO << " PSH";
    if (tcpHdr->getUrgBit())
        EV_INFO << " URG";
    EV_INFO << " len=" << payloadLength << "\n";

    send(fp, "ipOut");
}

void TcpNsc::processAppCommand(TcpNscConnection& connP, cMessage *msgP)
{
    printConnBrief(connP);

    // first do actions
    TcpCommand *tcpCommand = check_and_cast_nullable<TcpCommand *>(msgP->removeControlInfo());

    switch (msgP->getKind()) {
        case TCP_C_OPEN_ACTIVE:
            process_OPEN_ACTIVE(connP, tcpCommand, msgP);
            break;

        case TCP_C_OPEN_PASSIVE:
            process_OPEN_PASSIVE(connP, tcpCommand, msgP);
            break;

        case TCP_C_ACCEPT:
            process_ACCEPT(connP, check_and_cast<TcpAcceptCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_SEND:
            process_SEND(connP, check_and_cast<Packet *>(msgP));
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

void TcpNsc::process_OPEN_ACTIVE(TcpNscConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP)
{
    TcpOpenCommand *openCmd = check_and_cast<TcpOpenCommand *>(tcpCommandP);

    TcpNscConnection::SockPair inetSockPair, nscSockPair;
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
    nscSockPair.remoteM.ipAddrM.set(Ipv4Address(nscRemoteAddr));
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

void TcpNsc::process_OPEN_PASSIVE(TcpNscConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP)
{
    ASSERT(pStackM);

    TcpOpenCommand *openCmd = check_and_cast<TcpOpenCommand *>(tcpCommandP);

    if (!openCmd->getFork())
        throw cRuntimeError("TcpNsc supports Forking mode only");

    TcpNscConnection::SockPair inetSockPair, nscSockPair;
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

void TcpNsc::process_ACCEPT(TcpNscConnection& connP, TcpAcceptCommand *tcpCommandP, cMessage *msgP)
{
    connP.forkedConnId = -1;
    sendEstablishedMsg(connP);

    int err = NSC_EAGAIN;
    while (true) {
        static char buf[4096];

        int buflen = sizeof(buf);

        err = connP.pNscSocketM->read_data(buf, &buflen);

        EV_DEBUG << this << ": NSC: read: err " << err << " , buflen " << buflen << "\n";

        if (err == 0 && buflen > 0) {
            connP.receiveQueueM->enqueueNscData(buf, buflen);
            err = NSC_EAGAIN;
        }
        else
            break;
    }

    sendDataToApp(connP);
    sendErrorNotificationToApp(connP, err);
    delete tcpCommandP;
    delete msgP;
}

void TcpNsc::process_SEND(TcpNscConnection& connP, Packet *msgP)
{
    connP.send(msgP);

    connP.do_SEND();
}

void TcpNsc::do_SEND_all()
{
    for (auto & elem : tcpAppConnMapM) {
        TcpNscConnection& conn = elem.second;
        conn.do_SEND();
    }
}

void TcpNsc::process_CLOSE(TcpNscConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": process_CLOSE()\n";

    delete tcpCommandP;
    delete msgP;

    connP.close();
}

void TcpNsc::process_ABORT(TcpNscConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": process_ABORT()\n";

    delete tcpCommandP;
    delete msgP;

    connP.abort();
}

void TcpNsc::process_STATUS(TcpNscConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP)
{
    delete tcpCommandP;    // but we'll reuse msg for reply

    TcpStatusInfo *statusInfo = new TcpStatusInfo();

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
    send(msgP, "appOut");
}

} // namespace tcp

} // namespace inet

