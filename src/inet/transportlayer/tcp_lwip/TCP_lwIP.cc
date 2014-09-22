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

#include "inet/transportlayer/tcp_lwip/TCP_lwIP.h"

//#include "headers/defs.h"   // for endian macros
//#include "headers/in_systm.h"
#include "lwip/lwip_ip.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/ICMPMessage_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#endif // ifdef WITH_IPv6

#include "inet/networklayer/common/IL3AddressType.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

#include "inet/common/serializer/tcp/headers/tcphdr.h"
#include "lwip/lwip_tcp.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/transportlayer/tcp_lwip/TcpLwipConnection.h"
#include "inet/transportlayer/tcp_lwip/queues/TcpLwipByteStreamQueues.h"
#include "inet/transportlayer/tcp_lwip/queues/TcpLwipMsgBasedQueues.h"
#include "inet/transportlayer/tcp_lwip/queues/TcpLwipVirtualDataQueues.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/common/serializer/tcp/TCPSerializer.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

namespace tcp {

using namespace serializer;

Define_Module(TCP_lwIP);

TCP_lwIP::TCP_lwIP()
    :
    pLwipFastTimerM(NULL),
    pLwipTcpLayerM(NULL),
    isAliveM(false),
    pCurTcpSegM(NULL)
{
    netIf.gw.addr = L3Address();
    netIf.flags = 0;
    netIf.input = NULL;
    netIf.ip_addr.addr = L3Address();
    netIf.linkoutput = NULL;
    netIf.mtu = 1500;
    netIf.name[0] = 'T';
    netIf.name[1] = 'C';
    netIf.netmask.addr = L3Address();
    netIf.next = 0;
    netIf.num = 0;
    netIf.output = NULL;
    netIf.state = NULL;
}

void TCP_lwIP::initialize(int stage)
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

        recordStatisticsM = par("recordStats");

        pLwipTcpLayerM = new LwipTcpLayer(*this);
        pLwipFastTimerM = new cMessage("lwip_fast_timer");
        EV_INFO << "TCP_lwIP " << this << " has stack " << pLwipTcpLayerM << "\n";
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

TCP_lwIP::~TCP_lwIP()
{
    EV_TRACE << this << ": destructor\n";
    isAliveM = false;

    while (!tcpAppConnMapM.empty()) {
        TcpAppConnMap::iterator i = tcpAppConnMapM.begin();
        delete i->second;
        tcpAppConnMapM.erase(i);
    }

    if (pLwipFastTimerM)
        cancelAndDelete(pLwipFastTimerM);

    if (pLwipTcpLayerM)
        delete pLwipTcpLayerM;
}

// send a TCP_I_ESTABLISHED msg to Application Layer
void TCP_lwIP::sendEstablishedMsg(TcpLwipConnection& connP)
{
    connP.sendEstablishedMsg();
}

void TCP_lwIP::handleIpInputMessage(TCPSegment *tcpsegP)
{
    L3Address srcAddr, destAddr;
    int interfaceId = -1;

    cObject *ctrl = tcpsegP->removeControlInfo();
    if (!ctrl)
        error("(%s)%s arrived without control info", tcpsegP->getClassName(), tcpsegP->getName());

    INetworkProtocolControlInfo *controlInfo = check_and_cast<INetworkProtocolControlInfo *>(ctrl);
    srcAddr = controlInfo->getSourceAddress();
    destAddr = controlInfo->getDestinationAddress();
    interfaceId = controlInfo->getInterfaceId();
    delete ctrl;

    // process segment
    size_t ipHdrLen = sizeof(ip_hdr);
    size_t const maxBufferSize = 4096;
    char *data = new char[maxBufferSize];
    memset(data, 0, maxBufferSize);

    ip_hdr *ih = (ip_hdr *)data;
    tcphdr *tcph = (tcphdr *)(data + ipHdrLen);

    // set the modified lwip IP header:
    ih->_hl = ipHdrLen / 4;
    ASSERT((ih->_hl) * 4 == ipHdrLen);
    ih->_chksum = 0;
    ih->src.addr = srcAddr;
    ih->dest.addr = destAddr;

    size_t totalTcpLen = maxBufferSize - ipHdrLen;

    totalTcpLen = TCPSerializer().serialize(tcpsegP, (unsigned char *)tcph, totalTcpLen);

    // calculate TCP checksum
    tcph->th_sum = 0;
    tcph->th_sum = TCPSerializer().checksum(tcph, totalTcpLen, srcAddr, destAddr);

    size_t totalIpLen = ipHdrLen + totalTcpLen;
    ih->_chksum = 0;

    // search unfilled local addr in pcb-s for this connection.
    TcpAppConnMap::iterator i;
    L3Address laddr = ih->dest.addr;
    L3Address raddr = ih->src.addr;
    u16_t lport = tcpsegP->getDestPort();
    u16_t rport = tcpsegP->getSrcPort();

    if (tcpsegP->getSynBit() && tcpsegP->getAckBit()) {
        for (i = tcpAppConnMapM.begin(); i != tcpAppConnMapM.end(); i++) {
            LwipTcpLayer::tcp_pcb *pcb = i->second->pcbM;
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

    ASSERT(pCurTcpSegM == NULL);
    pCurTcpSegM = tcpsegP;
    // receive msg from network
    pLwipTcpLayerM->if_receive_packet(interfaceId, data, totalIpLen);
    // lwip call back the notifyAboutIncomingSegmentProcessing() for store incoming messages
    pCurTcpSegM = NULL;

    // LwipTcpLayer will call the tcp_event_recv() / tcp_event_err() and/or send a packet to sender

    delete[] data;
    delete tcpsegP;
}

void TCP_lwIP::notifyAboutIncomingSegmentProcessing(LwipTcpLayer::tcp_pcb *pcb, uint32 seqNo,
        const void *dataptr, int len)
{
    TcpLwipConnection *conn = (pcb != NULL) ? (TcpLwipConnection *)(pcb->callback_arg) : NULL;
    if (conn) {
        conn->receiveQueueM->notifyAboutIncomingSegmentProcessing(pCurTcpSegM, seqNo, dataptr, len);
    }
    else {
        if (pCurTcpSegM->getPayloadLength())
            throw cRuntimeError("conn is null, and received packet has data");

        EV_WARN << "notifyAboutIncomingSegmentProcessing: conn is null\n";
    }
}

void TCP_lwIP::lwip_free_pcb_event(LwipTcpLayer::tcp_pcb *pcb)
{
    TcpLwipConnection *conn = (TcpLwipConnection *)(pcb->callback_arg);
    if (conn != NULL) {
        if (conn->pcbM == pcb) {
            // conn->sendIndicationToApp(TCP_I_????); // TODO send some indication when need
            removeConnection(*conn);
        }
    }
}

err_t TCP_lwIP::lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
        LwipTcpLayer::lwip_event event, struct pbuf *p, u16_t size, err_t err)
{
    TcpLwipConnection *conn = (TcpLwipConnection *)arg;
    ASSERT(conn != NULL);

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

err_t TCP_lwIP::tcp_event_accept(TcpLwipConnection& conn, LwipTcpLayer::tcp_pcb *pcb, err_t err)
{
    int newConnId = ev.getUniqueNumber();
    TcpLwipConnection *newConn = new TcpLwipConnection(conn, newConnId, pcb);
    // add into appConnMap
    tcpAppConnMapM[newConnId] = newConn;

    newConn->sendEstablishedMsg();

    EV_DETAIL << this << ": TCP_lwIP: got accept!\n";
    conn.do_SEND();
    return err;
}

err_t TCP_lwIP::tcp_event_sent(TcpLwipConnection& conn, u16_t size)
{
    conn.do_SEND();
    return ERR_OK;
}

err_t TCP_lwIP::tcp_event_recv(TcpLwipConnection& conn, struct pbuf *p, err_t err)
{
    if (p == NULL) {
        // Received FIN:
        EV_DETAIL << this << ": tcp_event_recv(" << conn.connIdM
                  << ", pbuf[NULL], " << (int)err << "):FIN\n";
        TcpStatusInd ind = (conn.pcbM->state == LwipTcpLayer::TIME_WAIT) ? TCP_I_CLOSED : TCP_I_PEER_CLOSED;
        EV_INFO << "Connection " << conn.connIdM << ((ind == TCP_I_CLOSED) ? " closed" : "closed by peer") << endl;
        conn.sendIndicationToApp(ind);
        // TODO is it good?
        pLwipTcpLayerM->tcp_recved(conn.pcbM, 0);
    }
    else {
        EV_DETAIL << this << ": tcp_event_recv(" << conn.connIdM << ", pbuf[" << p->len << ", "
                  << p->tot_len << "], " << (int)err << ")\n";
        conn.receiveQueueM->enqueueTcpLayerData(p->payload, p->tot_len);
        pLwipTcpLayerM->tcp_recved(conn.pcbM, p->tot_len);
        pbuf_free(p);
    }

    while (cPacket *dataMsg = conn.receiveQueueM->extractBytesUpTo()) {
        TCPConnectInfo *tcpConnectInfo = new TCPConnectInfo();
        tcpConnectInfo->setConnId(conn.connIdM);
        tcpConnectInfo->setLocalAddr(conn.pcbM->local_ip.addr);
        tcpConnectInfo->setRemoteAddr(conn.pcbM->remote_ip.addr);
        tcpConnectInfo->setLocalPort(conn.pcbM->local_port);
        tcpConnectInfo->setRemotePort(conn.pcbM->remote_port);
        dataMsg->setControlInfo(tcpConnectInfo);
        // send Msg to Application layer:
        send(dataMsg, "appOut", conn.appGateIndexM);
    }

    conn.do_SEND();
    return err;
}

err_t TCP_lwIP::tcp_event_conn(TcpLwipConnection& conn, err_t err)
{
    conn.sendEstablishedMsg();
    conn.do_SEND();
    return err;
}

void TCP_lwIP::removeConnection(TcpLwipConnection& conn)
{
    conn.pcbM->callback_arg = NULL;
    conn.pcbM = NULL;
    tcpAppConnMapM.erase(conn.connIdM);
    delete &conn;
}

err_t TCP_lwIP::tcp_event_err(TcpLwipConnection& conn, err_t err)
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

err_t TCP_lwIP::tcp_event_poll(TcpLwipConnection& conn)
{
    conn.do_SEND();
    return ERR_OK;
}

struct netif *TCP_lwIP::ip_route(L3Address const& ipAddr)
{
    return &netIf;
}

void TCP_lwIP::handleAppMessage(cMessage *msgP)
{
    TCPCommand *controlInfo = check_and_cast<TCPCommand *>(msgP->getControlInfo());
    int connId = controlInfo->getConnId();

    TcpLwipConnection *conn = findAppConn(connId);

    if (!conn) {
        TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(controlInfo);

        TCPDataTransferMode dataTransferMode = (TCPDataTransferMode)(openCmd->getDataTransferMode());

        // add into appConnMap
        conn = new TcpLwipConnection(*this, connId, msgP->getArrivalGate()->getIndex(), dataTransferMode);
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

void TCP_lwIP::handleMessage(cMessage *msgP)
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
            // must be a TCPSegment
            TCPSegment *tcpseg = check_and_cast<TCPSegment *>(msgP);
            EV_TRACE << this << ": handle tcp segment: " << msgP->getName() << "\n";
            handleIpInputMessage(tcpseg);
        }
    }
    else {    // must be from app
        EV_TRACE << this << ": handle msg: " << msgP->getName() << "\n";
        handleAppMessage(msgP);
    }

    if (!pLwipFastTimerM->isScheduled()) {    // lwip fast timer
        if (NULL != pLwipTcpLayerM->tcp_active_pcbs || NULL != pLwipTcpLayerM->tcp_tw_pcbs)
            scheduleAt(roundTime(simTime() + 0.250, 4), pLwipFastTimerM);
    }

    if (ev.isGUI())
        updateDisplayString();
}

void TCP_lwIP::updateDisplayString()
{
    if (ev.isDisabled()) {
        // in express mode, we don't bother to update the display
        // (std::map's iteration is not very fast if map is large)
        getDisplayString().setTagArg("t", 0, "");
        return;
    }

    int numINIT = 0, numCLOSED = 0, numLISTEN = 0, numSYN_SENT = 0, numSYN_RCVD = 0,
        numESTABLISHED = 0, numCLOSE_WAIT = 0, numLAST_ACK = 0, numFIN_WAIT_1 = 0,
        numFIN_WAIT_2 = 0, numCLOSING = 0, numTIME_WAIT = 0;

    for (TcpAppConnMap::iterator i = tcpAppConnMapM.begin(); i != tcpAppConnMapM.end(); ++i) {
        LwipTcpLayer::tcp_pcb *pcb = (*i).second->pcbM;

        if (NULL == pcb) {
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

TcpLwipConnection *TCP_lwIP::findAppConn(int connIdP)
{
    TcpAppConnMap::iterator i = tcpAppConnMapM.find(connIdP);
    return i == tcpAppConnMapM.end() ? NULL : (i->second);
}

void TCP_lwIP::finish()
{
    isAliveM = false;
}

void TCP_lwIP::printConnBrief(TcpLwipConnection& connP)
{
    EV_TRACE << this << ": connId=" << connP.connIdM << " appGateIndex=" << connP.appGateIndexM;
}

void TCP_lwIP::ip_output(LwipTcpLayer::tcp_pcb *pcb, L3Address const& srcP,
        L3Address const& destP, void *dataP, int lenP)
{
    TcpLwipConnection *conn = (pcb != NULL) ? (TcpLwipConnection *)(pcb->callback_arg) : NULL;

    TCPSegment *tcpseg;

    if (conn) {
        tcpseg = conn->sendQueueM->createSegmentWithBytes(dataP, lenP);
    }
    else {
        tcpseg = new TCPSegment("tcp-segment");

        TCPSerializer().parse((const unsigned char *)dataP, lenP, tcpseg, true);
        ASSERT(tcpseg->getPayloadLength() == 0);
    }

    ASSERT(tcpseg);

    EV_TRACE << this << ": Sending: conn=" << conn << ", data: " << dataP << " of len " << lenP
             << " from " << srcP << " to " << destP << "\n";

    IL3AddressType *addressType = destP.getAddressType();
    INetworkProtocolControlInfo *controlInfo = addressType->createNetworkProtocolControlInfo();
    controlInfo->setTransportProtocol(IP_PROT_TCP);
    controlInfo->setSourceAddress(srcP);
    controlInfo->setDestinationAddress(destP);
    tcpseg->setControlInfo(check_and_cast<cObject *>(controlInfo));

    if (conn) {
        conn->notifyAboutSending(*tcpseg);
    }

    EV_INFO << this << ": Send segment: conn ID=" << conn->connIdM << " from " << srcP
            << " to " << destP << " SEQ=" << tcpseg->getSequenceNo();
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

void TCP_lwIP::processAppCommand(TcpLwipConnection& connP, cMessage *msgP)
{
    printConnBrief(connP);

    // first do actions
    TCPCommand *tcpCommand = check_and_cast<TCPCommand *>(msgP->removeControlInfo());

    switch (msgP->getKind()) {
        case TCP_C_OPEN_ACTIVE:
            process_OPEN_ACTIVE(connP, check_and_cast<TCPOpenCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_OPEN_PASSIVE:
            process_OPEN_PASSIVE(connP, check_and_cast<TCPOpenCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_SEND:
            process_SEND(connP, check_and_cast<TCPSendCommand *>(tcpCommand),
                check_and_cast<cPacket *>(msgP));
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
}

void TCP_lwIP::process_OPEN_ACTIVE(TcpLwipConnection& connP, TCPOpenCommand *tcpCommandP,
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

void TCP_lwIP::process_OPEN_PASSIVE(TcpLwipConnection& connP, TCPOpenCommand *tcpCommandP,
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

void TCP_lwIP::process_SEND(TcpLwipConnection& connP, TCPSendCommand *tcpCommandP, cPacket *msgP)
{
    EV_INFO << this << ": processing SEND command, len=" << msgP->getByteLength() << endl;

    delete tcpCommandP;

    connP.send(msgP);
}

void TCP_lwIP::process_CLOSE(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing CLOSE(" << connP.connIdM << ") command\n";

    delete tcpCommandP;
    delete msgP;

    connP.close();
}

void TCP_lwIP::process_ABORT(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing ABORT(" << connP.connIdM << ") command\n";

    delete tcpCommandP;
    delete msgP;

    connP.abort();
}

void TCP_lwIP::process_STATUS(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing STATUS(" << connP.connIdM << ") command\n";

    delete tcpCommandP;    // but we'll reuse msg for reply

    TCPStatusInfo *statusInfo = new TCPStatusInfo();
    connP.fillStatusInfo(*statusInfo);
    msgP->setControlInfo(statusInfo);
    msgP->setKind(TCP_I_STATUS);
    send(msgP, "appOut", connP.appGateIndexM);
}

TcpLwipSendQueue *TCP_lwIP::createSendQueue(TCPDataTransferMode transferModeP)
{
    switch (transferModeP) {
        case TCP_TRANSFER_BYTECOUNT:
            return new TcpLwipVirtualDataSendQueue();

        case TCP_TRANSFER_OBJECT:
            return new TcpLwipMsgBasedSendQueue();

        case TCP_TRANSFER_BYTESTREAM:
            return new TcpLwipByteStreamSendQueue();

        default:
            throw cRuntimeError("Invalid TCP data transfer mode: %d", transferModeP);
    }
}

TcpLwipReceiveQueue *TCP_lwIP::createReceiveQueue(TCPDataTransferMode transferModeP)
{
    switch (transferModeP) {
        case TCP_TRANSFER_BYTECOUNT:
            return new TcpLwipVirtualDataReceiveQueue();

        case TCP_TRANSFER_OBJECT:
            return new TcpLwipMsgBasedReceiveQueue();

        case TCP_TRANSFER_BYTESTREAM:
            return new TcpLwipByteStreamReceiveQueue();

        default:
            throw cRuntimeError("Invalid TCP data transfer mode: %d", transferModeP);
    }
}

bool TCP_lwIP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

} // namespace tcp

} // namespace inet

