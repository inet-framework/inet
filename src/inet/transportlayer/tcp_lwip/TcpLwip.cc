//
// Copyright (C) 2010 Zoltan Bojthe
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

#include "inet/transportlayer/tcp_lwip/TcpLwip.h"

//#include "headers/defs.h"   // for endian macros
//#include "headers/in_systm.h"
#include "lwip/lwip_ip.h"
#include "lwip/lwip_tcp.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#endif // ifdef WITH_IPv6

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_common/headers/tcphdr.h"
#include "inet/transportlayer/tcp_lwip/TcpLwipConnection.h"
#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"

namespace inet {
namespace tcp {

Define_Module(TcpLwip);

TcpLwip::TcpLwip()
    :
    pLwipFastTimerM(nullptr),
    pLwipTcpLayerM(nullptr),
    isAliveM(false),
    pCurTcpSegM(nullptr)
{
    netIf.gw.addr = L3Address();
    netIf.flags = 0;
    netIf.input = nullptr;
    netIf.ip_addr.addr = L3Address();
    netIf.linkoutput = nullptr;
    netIf.mtu = 1500;
    netIf.name[0] = 'T';
    netIf.name[1] = 'C';
    netIf.netmask.addr = L3Address();
    netIf.next = nullptr;
    netIf.num = 0;
    netIf.output = nullptr;
    netIf.state = nullptr;
}

void TcpLwip::initialize(int stage)
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

        pLwipTcpLayerM = new LwipTcpLayer(*this);
        pLwipFastTimerM = new cMessage("lwip_fast_timer");
        EV_INFO << "TcpLwip " << this << " has stack " << pLwipTcpLayerM << "\n";
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

TcpLwip::~TcpLwip()
{
    EV_TRACE << this << ": destructor\n";
    isAliveM = false;

#if OMNETPP_BUILDNUM < 1505   //OMNETPP_VERSION < 0x0600    // 6.0 pre9
    while (!tcpAppConnMapM.empty()) {
        auto i = tcpAppConnMapM.begin();
        auto& pcb = i->second->pcbM;
        if (pcb) {
            pcb->callback_arg = nullptr;
            getLwipTcpLayer()->tcp_pcb_purge(pcb);
            memp_free(MEMP_TCP_PCB, pcb);
            pcb = nullptr;
        }
        i->second->deleteModule();
        tcpAppConnMapM.erase(i);
    }
#endif

    if (pLwipFastTimerM)
        cancelAndDelete(pLwipFastTimerM);

    if (pLwipTcpLayerM)
        delete pLwipTcpLayerM;
}

void TcpLwip::handleIpInputMessage(Packet *packet)
{
    L3Address srcAddr, destAddr;
    int interfaceId = -1;

    auto tcpsegP = packet->peekAtFront<TcpHeader>();
    srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
    destAddr = packet->getTag<L3AddressInd>()->getDestAddress();
    interfaceId = (packet->getTag<InterfaceInd>())->getInterfaceId();

    switch(tcpsegP->getCrcMode()) {
        case CRC_DECLARED_INCORRECT:
            EV_WARN << "CRC error, packet dropped\n";
            delete packet;
            return;
        case CRC_DECLARED_CORRECT: {
            // modify to calculated, for serializing
            packet->trimFront();
            const auto& newTcpsegP = packet->removeAtFront<TcpHeader>();
            newTcpsegP->setCrcMode(CRC_COMPUTED);
            newTcpsegP->setCrc(0);
            packet->insertAtFront(newTcpsegP);
            tcpsegP = newTcpsegP;
            break;
        }
        default:
            break;
    }

    // process segment
    size_t ipHdrLen = sizeof(ip_hdr);
    size_t const maxBufferSize = 4096;
    char *data = new char[maxBufferSize];
    memset(data, 0, maxBufferSize);

    ip_hdr *ih = (ip_hdr *)(data);
    //tcphdr *tcph = (tcphdr *)(data + ipHdrLen);

    // set the modified lwip IP header:
    ih->_hl = ipHdrLen / 4;
    ASSERT((ih->_hl) * 4 == ipHdrLen);
    ih->_chksum = 0;
    ih->src.addr = srcAddr;
    ih->dest.addr = destAddr;

    size_t totalTcpLen = maxBufferSize - ipHdrLen;

    const auto& bytes = packet->peekDataAsBytes();
    totalTcpLen = bytes->copyToBuffer((uint8_t *)data + ipHdrLen, totalTcpLen);

    size_t totalIpLen = ipHdrLen + totalTcpLen;
    ih->_chksum = 0;

    // search unfilled local addr in pcb-s for this connection.
    L3Address laddr = ih->dest.addr;
    L3Address raddr = ih->src.addr;
    u16_t lport = tcpsegP->getDestPort();
    u16_t rport = tcpsegP->getSrcPort();

    if (tcpsegP->getSynBit() && tcpsegP->getAckBit()) {
        for (auto & elem : tcpAppConnMapM) {
            LwipTcpLayer::tcp_pcb *pcb = elem.second->pcbM;
            if (pcb) {
                if ((pcb->state == LwipTcpLayer::SYN_SENT)
                    && (pcb->local_ip.addr.isUnspecified())
                    && (pcb->local_port == lport)
                    && (pcb->remote_ip.addr == raddr)
                    && (pcb->remote_port == rport)
                    )
                {
                    pcb->local_ip.addr = laddr;
                }
            }
        }
    }

    ASSERT(pCurTcpSegM == nullptr);
    pCurTcpSegM = packet;
    // receive msg from network
    pLwipTcpLayerM->if_receive_packet(interfaceId, data, totalIpLen);
    // lwip call back the notifyAboutIncomingSegmentProcessing() for store incoming messages
    pCurTcpSegM = nullptr;

    // LwipTcpLayer will call the tcp_event_recv() / tcp_event_err() and/or send a packet to sender

    delete[] data;
    delete packet;
}

void TcpLwip::notifyAboutIncomingSegmentProcessing(LwipTcpLayer::tcp_pcb *pcb, uint32 seqNo,
        const void *dataptr, int len)
{
    TcpLwipConnection *conn = (pcb != nullptr) ? static_cast<TcpLwipConnection *>(pcb->callback_arg) : nullptr;
    if (conn) {
        conn->receiveQueueM->notifyAboutIncomingSegmentProcessing(pCurTcpSegM, seqNo, dataptr, len);
    }
    else {
        const auto& tcpHdr = pCurTcpSegM->peekAtFront<TcpHeader>();
        if (B(pCurTcpSegM->getByteLength()) > tcpHdr->getHeaderLength())
            throw cRuntimeError("conn is null, and received packet has data");

        EV_WARN << "notifyAboutIncomingSegmentProcessing: conn is null\n";
    }
}

void TcpLwip::lwip_free_pcb_event(LwipTcpLayer::tcp_pcb *pcb)
{
    TcpLwipConnection *conn = static_cast<TcpLwipConnection *>(pcb->callback_arg);
    if (conn != nullptr && conn->pcbM == pcb) {
        // conn->sendIndicationToApp(TCP_I_????); // TODO send some indication when need
        removeConnection(*conn);
    }
}

err_t TcpLwip::lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
        LwipTcpLayer::lwip_event event, struct pbuf *p, u16_t size, err_t err)
{
    TcpLwipConnection *conn = static_cast<TcpLwipConnection *>(arg);
    ASSERT(conn != nullptr);

    switch (event) {
        case LwipTcpLayer::LWIP_EVENT_ACCEPT:
            err = tcp_event_accept(*conn, pcb, err);
            break;

        case LwipTcpLayer::LWIP_EVENT_SENT:
            ASSERT(conn->pcbM == pcb);
            err = tcp_event_sent(*conn, size);
            break;

        case LwipTcpLayer::LWIP_EVENT_RECV:
            ASSERT(conn->pcbM == pcb);
            err = tcp_event_recv(*conn, p, err);
            break;

        case LwipTcpLayer::LWIP_EVENT_CONNECTED:
            ASSERT(conn->pcbM == pcb);
            err = tcp_event_conn(*conn, err);
            break;

        case LwipTcpLayer::LWIP_EVENT_POLL:
            // it's called also when conn->pcbM point to a LISTEN pcb, and pcb point to a SYN_RCVD
            if (conn->pcbM == pcb)
                err = tcp_event_poll(*conn);
            break;

        case LwipTcpLayer::LWIP_EVENT_ERR:
            err = tcp_event_err(*conn, err);
            break;

        default:
            throw cRuntimeError("Invalid lwip_event: %d", event);
            break;
    }

    return err;
}

err_t TcpLwip::tcp_event_accept(TcpLwipConnection& conn, LwipTcpLayer::tcp_pcb *pcb, err_t err)
{
    int newConnId = getEnvir()->getUniqueNumber();

    auto moduleType = cModuleType::get("inet.transportlayer.tcp_lwip.TcpLwipConnection");
    char submoduleName[24];
    sprintf(submoduleName, "conn-%d", newConnId);
    auto newConn = check_and_cast<TcpLwipConnection *>(moduleType->create(submoduleName, this));
    newConn->finalizeParameters();
    newConn->buildInside();
    newConn->initConnection(conn, newConnId, pcb);
    newConn->callInitialize();

    // add into appConnMap
    tcpAppConnMapM[newConnId] = newConn;

    newConn->sendAvailableIndicationToApp(conn.connIdM);

    EV_DETAIL << this << ": TcpLwip: got accept!\n";
    return err;
}

err_t TcpLwip::tcp_event_sent(TcpLwipConnection& conn, u16_t size)
{
    conn.do_SEND();
    return ERR_OK;
}

err_t TcpLwip::tcp_event_recv(TcpLwipConnection& conn, struct pbuf *p, err_t err)
{
    if (p == nullptr) {
        // Received FIN:
        EV_DETAIL << this << ": tcp_event_recv(" << conn.connIdM
                  << ", pbuf[nullptr], " << (int)err << "):FIN\n";
        TcpStatusInd ind = (conn.pcbM->state == LwipTcpLayer::TIME_WAIT) ? TCP_I_CLOSED : TCP_I_PEER_CLOSED;
        EV_INFO << "Connection " << conn.connIdM << ((ind == TCP_I_CLOSED) ? " closed" : "closed by peer") << endl;
        conn.sendIndicationToApp(ind);
        // TODO is it good?
        pLwipTcpLayerM->tcp_recved(conn.pcbM, 0);
    }
    else {
        EV_DETAIL << this << ": tcp_event_recv(" << conn.connIdM << ", pbuf[" << p->len << ", "
                  << p->tot_len << "], " << (int)err << ")\n";
        for (auto c = p; c; c = c->next)
            conn.receiveQueueM->enqueueTcpLayerData(c->payload, c->len);
        pLwipTcpLayerM->tcp_recved(conn.pcbM, p->tot_len);
        pbuf_free(p);
    }

    conn.sendUpData();
    conn.do_SEND();
    return err;
}

err_t TcpLwip::tcp_event_conn(TcpLwipConnection& conn, err_t err)
{
    conn.sendEstablishedMsg();
    conn.do_SEND();
    return err;
}

void TcpLwip::removeConnection(TcpLwipConnection& conn)
{
    conn.pcbM->callback_arg = nullptr;
    conn.pcbM = nullptr;
    tcpAppConnMapM.erase(conn.connIdM);
    conn.deleteModule();
}

err_t TcpLwip::tcp_event_err(TcpLwipConnection& conn, err_t err)
{
    switch (err) {
        case ERR_ABRT:
            EV_INFO << "Connection " << conn.connIdM << " aborted, closed\n";
            conn.sendIndicationToApp(TCP_I_CLOSED);
            removeConnection(conn);
            break;

        case ERR_RST:
            EV_INFO << "Connection " << conn.connIdM << " reset\n";
            conn.sendIndicationToApp(TCP_I_CONNECTION_RESET);
            removeConnection(conn);
            break;

        default:
            throw cRuntimeError("Invalid LWIP error code: %d", err);
    }

    return err;
}

err_t TcpLwip::tcp_event_poll(TcpLwipConnection& conn)
{
    conn.do_SEND();
    return ERR_OK;
}

struct netif *TcpLwip::ip_route(L3Address const& ipAddr)
{
    return &netIf;
}

void TcpLwip::handleAppMessage(cMessage *msgP)
{
    auto& tags = getTags(msgP);
    int connId = tags.getTag<SocketReq>()->getSocketId();

    TcpLwipConnection *conn = findAppConn(connId);

    if (!conn) {
        // add into appConnMap

        auto moduleType = cModuleType::get("inet.transportlayer.tcp_lwip.TcpLwipConnection");
        char submoduleName[24];
        sprintf(submoduleName, "conn-%d", connId);
        conn = check_and_cast<TcpLwipConnection *>(moduleType->create(submoduleName, this));
        conn->finalizeParameters();
        conn->buildInside();
        conn->initConnection(*this, connId);
        conn->callInitialize();

        tcpAppConnMapM[connId] = conn;

        EV_INFO << this << ": TCP connection created for " << msgP << "\n";
    }

    processAppCommand(*conn, msgP);
}

simtime_t roundTime(const simtime_t& timeP, int secSlicesP)
{
    int64_t scale = timeP.getScale() / secSlicesP;
    simtime_t ret = timeP;
    ret /= scale;
    ret *= scale;
    return ret;
}

void TcpLwip::handleMessage(cMessage *msgP)
{
    if (msgP->isSelfMessage()) {
        // timer expired
        if (msgP == pLwipFastTimerM) {    // lwip fast timer
            EV_TRACE << "Call tcp_fasttmr()\n";
            pLwipTcpLayerM->tcp_fasttmr();
            if (simTime() == roundTime(simTime(), 2)) {
                EV_TRACE << "Call tcp_slowtmr()\n";
                pLwipTcpLayerM->tcp_slowtmr();
            }
        }
        else {
            throw cRuntimeError("Unknown self message");
        }
    }
    else if (msgP->arrivedOn("ipIn")) {
        // must be a Packet
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
        EV_TRACE << this << ": handle msg: " << msgP->getName() << "\n";
        handleAppMessage(msgP);
    }

    if (!pLwipFastTimerM->isScheduled()) {    // lwip fast timer
        if (nullptr != pLwipTcpLayerM->tcp_active_pcbs || nullptr != pLwipTcpLayerM->tcp_tw_pcbs)
            scheduleAt(roundTime(simTime() + 0.250, 4), pLwipFastTimerM);
    }
}

void TcpLwip::refreshDisplay() const
{
    if (getEnvir()->isExpressMode()) {
        // in express mode, we don't bother to update the display
        // (std::map's iteration is not very fast if map is large)
        getDisplayString().setTagArg("t", 0, "");
        return;
    }

    int numINIT = 0, numCLOSED = 0, numLISTEN = 0, numSYN_SENT = 0, numSYN_RCVD = 0,
        numESTABLISHED = 0, numCLOSE_WAIT = 0, numLAST_ACK = 0, numFIN_WAIT_1 = 0,
        numFIN_WAIT_2 = 0, numCLOSING = 0, numTIME_WAIT = 0;

    for (auto & elem : tcpAppConnMapM) {
        LwipTcpLayer::tcp_pcb *pcb = (elem).second->pcbM;

        if (nullptr == pcb) {
            numINIT++;
        }
        else {
            enum LwipTcpLayer::tcp_state state = pcb->state;

            switch (state) {
                case LwipTcpLayer::CLOSED:
                    numCLOSED++;
                    break;

                case LwipTcpLayer::LISTEN:
                    numLISTEN++;
                    break;

                case LwipTcpLayer::SYN_SENT:
                    numSYN_SENT++;
                    break;

                case LwipTcpLayer::SYN_RCVD:
                    numSYN_RCVD++;
                    break;

                case LwipTcpLayer::ESTABLISHED:
                    numESTABLISHED++;
                    break;

                case LwipTcpLayer::CLOSE_WAIT:
                    numCLOSE_WAIT++;
                    break;

                case LwipTcpLayer::LAST_ACK:
                    numLAST_ACK++;
                    break;

                case LwipTcpLayer::FIN_WAIT_1:
                    numFIN_WAIT_1++;
                    break;

                case LwipTcpLayer::FIN_WAIT_2:
                    numFIN_WAIT_2++;
                    break;

                case LwipTcpLayer::CLOSING:
                    numCLOSING++;
                    break;

                case LwipTcpLayer::TIME_WAIT:
                    numTIME_WAIT++;
                    break;
            }
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

TcpLwipConnection *TcpLwip::findAppConn(int connIdP)
{
    auto i = tcpAppConnMapM.find(connIdP);
    return i == tcpAppConnMapM.end() ? nullptr : (i->second);
}

void TcpLwip::finish()
{
    isAliveM = false;
}

void TcpLwip::printConnBrief(TcpLwipConnection& connP)
{
    EV_TRACE << this << ": connId=" << connP.connIdM;
}

void TcpLwip::ip_output(LwipTcpLayer::tcp_pcb *pcb, L3Address const& srcP, L3Address const& destP, void *dataP, int lenP)
{
    TcpLwipConnection *conn = (pcb != nullptr) ? static_cast<TcpLwipConnection *>(pcb->callback_arg) : nullptr;

    Packet *packet = nullptr;

    if (conn) {
        packet = conn->sendQueueM->createSegmentWithBytes(dataP, lenP);
    }
    else {
        const auto& bytes = makeShared<BytesChunk>((const uint8_t*)dataP, lenP);
        packet = new Packet(nullptr, bytes);
        const auto& tcpHdr = packet->popAtFront<TcpHeader>();
        packet->trimFront();
        int64_t numBytes = packet->getByteLength();
        ASSERT(numBytes == 0);
        packet->insertAtFront(tcpHdr);
    }

    ASSERT(packet);

    auto tcpHdr = packet->removeAtFront<TcpHeader>();
    tcpHdr->setCrc(0);
    tcpHdr->setCrcMode(crcMode);
    insertTransportProtocolHeader(packet, Protocol::tcp, tcpHdr);

    EV_TRACE << this << ": Sending: conn=" << conn << ", data: " << dataP << " of len " << lenP
             << " from " << srcP << " to " << destP << "\n";

    IL3AddressType *addressType = destP.getAddressType();

    packet->addTag<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    auto addresses = packet->addTag<L3AddressReq>();
    addresses->setSrcAddress(srcP);
    addresses->setDestAddress(destP);
    if (conn) {
        conn->notifyAboutSending(*tcpHdr);
    }

    EV_INFO << this << ": Send segment:";
    if (conn)
        EV_INFO << "conn ID=" << conn->connIdM;
    EV_INFO << " from " << srcP << " to " << destP << " SEQ=" << tcpHdr->getSequenceNo();
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
    EV_INFO << " len=" << B(packet->getDataLength()) - tcpHdr->getHeaderLength() << "\n";

    send(packet, "ipOut");
}

void TcpLwip::processAppCommand(TcpLwipConnection& connP, cMessage *msgP)
{
    printConnBrief(connP);

    // first do actions
    TcpCommand *tcpCommand = check_and_cast_nullable<TcpCommand *>(msgP->removeControlInfo());

    switch (msgP->getKind()) {
        case TCP_C_OPEN_ACTIVE:
            process_OPEN_ACTIVE(connP, check_and_cast<TcpOpenCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_OPEN_PASSIVE:
            process_OPEN_PASSIVE(connP, check_and_cast<TcpOpenCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_ACCEPT:
            process_ACCEPT(connP, check_and_cast<TcpAcceptCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_SEND:
            process_SEND(connP, check_and_cast<Packet *>(msgP));
            break;

        case TCP_C_CLOSE:
            ASSERT(tcpCommand);
            process_CLOSE(connP, tcpCommand, msgP);
            break;

        case TCP_C_ABORT:
            ASSERT(tcpCommand);
            process_ABORT(connP, tcpCommand, msgP);
            break;

        case TCP_C_STATUS:
            ASSERT(tcpCommand);
            process_STATUS(connP, tcpCommand, msgP);
            break;

        default:
            throw cRuntimeError("Wrong command from app: %d", msgP->getKind());
    }
}

void TcpLwip::process_OPEN_ACTIVE(TcpLwipConnection& connP, TcpOpenCommand *tcpCommandP,
        cMessage *msgP)
{
    if (tcpCommandP->getRemoteAddr().isUnspecified() || tcpCommandP->getRemotePort() == -1)
        throw cRuntimeError("Error processing command OPEN_ACTIVE: remote address and port must be specified");

    ASSERT(pLwipTcpLayerM);

    int localPort = tcpCommandP->getLocalPort();
    if (localPort == -1)
        localPort = 0;

    EV_INFO << this << ": OPEN: "
            << tcpCommandP->getLocalAddr() << ":" << localPort << " --> "
            << tcpCommandP->getRemoteAddr() << ":" << tcpCommandP->getRemotePort() << "\n";
    connP.connect(tcpCommandP->getLocalAddr(), localPort,
            tcpCommandP->getRemoteAddr(), tcpCommandP->getRemotePort());

    delete tcpCommandP;
    delete msgP;
}

void TcpLwip::process_OPEN_PASSIVE(TcpLwipConnection& connP, TcpOpenCommand *tcpCommandP,
        cMessage *msgP)
{
    ASSERT(pLwipTcpLayerM);

    ASSERT(tcpCommandP->getFork() == true);

    if (tcpCommandP->getLocalPort() == -1)
        throw cRuntimeError("Error processing command OPEN_PASSIVE: local port must be specified");

    EV_INFO << this << "Starting to listen on: " << tcpCommandP->getLocalAddr() << ":"
            << tcpCommandP->getLocalPort() << "\n";

    /*
       process passive open request
     */

    connP.listen(tcpCommandP->getLocalAddr(), tcpCommandP->getLocalPort());

    delete tcpCommandP;
    delete msgP;
}

void TcpLwip::process_ACCEPT(TcpLwipConnection& connP, TcpAcceptCommand *tcpCommand, cMessage *msg)
{
    connP.accept();
    delete tcpCommand;
    delete msg;
}

void TcpLwip::process_SEND(TcpLwipConnection& connP, Packet *msgP)
{
    EV_INFO << this << ": processing SEND command, len=" << msgP->getByteLength() << endl;

    connP.send(msgP);
}

void TcpLwip::process_CLOSE(TcpLwipConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing CLOSE(" << connP.connIdM << ") command\n";

    delete tcpCommandP;
    delete msgP;

    connP.close();
}

void TcpLwip::process_ABORT(TcpLwipConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing ABORT(" << connP.connIdM << ") command\n";

    delete tcpCommandP;
    delete msgP;

    connP.abort();
}

void TcpLwip::process_STATUS(TcpLwipConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing STATUS(" << connP.connIdM << ") command\n";

    delete tcpCommandP;    // but we'll reuse msg for reply

    TcpStatusInfo *statusInfo = new TcpStatusInfo();
    connP.fillStatusInfo(*statusInfo);
    msgP->setControlInfo(statusInfo);
    msgP->setKind(TCP_I_STATUS);
    send(msgP, "appOut");
}

TcpLwipSendQueue *TcpLwip::createSendQueue()
{
    return new TcpLwipSendQueue();
}

TcpLwipReceiveQueue *TcpLwip::createReceiveQueue()
{
    return new TcpLwipReceiveQueue();
}

} // namespace tcp
} // namespace inet

