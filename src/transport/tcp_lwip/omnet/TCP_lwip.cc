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

#include "TCP_lwip.h"

#include "headers/defs.h"   // for endian macros
#include "headers/in_systm.h"
#include "headers/ip.h"

#include "IPControlInfo.h"
#include "IPv6ControlInfo.h"
#include "headers/tcp.h"
#include "lwip/tcp.h"
#include "TCPCommand_m.h"
#include "TCPIPchecksum.h"
#include "TcpLwipConnection.h"
#include "TcpLwipQueues.h"
#include "TCPSegment.h"
#include "TCPSerializer.h"

#include <assert.h>
#include <dlfcn.h>


Define_Module(TCP_lwip);

bool TCP_lwip::testingS;
bool TCP_lwip::logverboseS;

// macro for normal ev<< logging (note: deliberately no parens in macro def)
// FIXME
//#define tcpEV (((ev.disable_tracing) || (TCP_lwip::testingS)) ? ev : std::cout)
#define tcpEV ev
//#define tcpEV std::cout

TCP_lwip::TCP_lwip()
  :
    pLwipFastTimerM(NULL),
    pLwipTcpLayerM(NULL),
    isAliveM(false),
	pCurTcpSegM(NULL)
{
    netIf.gw.addr = 0;
    netIf.flags = 0;
    netIf.input = NULL;
    netIf.ip_addr.addr = 0;
    netIf.linkoutput = NULL;
    netIf.mtu = 1500;
    netIf.name[0] = 'T';
    netIf.name[1] = 'C';
    netIf.netmask.addr = 0;
    netIf.next = 0;
    netIf.num = 0;
    netIf.output = NULL;
    netIf.state = NULL;
}

void TCP_lwip::initialize()
{
    tcpEV << this << ": initialize\n";
    WATCH_MAP(tcpAppConnMapM);

    cModule *netw = simulation.getSystemModule();
    testingS = netw->hasPar("testing") && netw->par("testing").boolValue();
    logverboseS = !testingS && netw->hasPar("logverbose") && netw->par("logverbose").boolValue();

    loadStack();
//    pLwipTcpLayerM->if_attach(localInnerIpS.str().c_str(), localInnerMaskS.str().c_str(), 1500);
//    pLwipTcpLayerM->add_default_gateway(localInnerGwS.str().c_str());

    isAliveM = true;
}

TCP_lwip::~TCP_lwip()
{
    tcpEV << this << ": destructor\n";
    isAliveM = false;
    while (!tcpAppConnMapM.empty())
    {
        TcpAppConnMap::iterator i = tcpAppConnMapM.begin();
        tcpAppConnMapM.erase(i);
    }
}

// send a TCP_I_ESTABLISHED msg to Application Layer
void TCP_lwip::sendEstablishedMsg(TcpLwipConnection &connP)
{
    connP.sendEstablishedMsg();
}

void TCP_lwip::handleIpInputMessage(TCPSegment* tcpsegP)
{
    IPvXAddress srcAddr, destAddr;
    int interfaceId = -1;

    // get src/dest addresses
    if (dynamic_cast<IPControlInfo *>(tcpsegP->getControlInfo())!=NULL)
    {
        IPControlInfo *controlInfo = (IPControlInfo *)tcpsegP->removeControlInfo();
        srcAddr = controlInfo->getSrcAddr();
        destAddr = controlInfo->getDestAddr();
        interfaceId = controlInfo->getInterfaceId();
        delete controlInfo;
    }
    else if (dynamic_cast<IPv6ControlInfo *>(tcpsegP->getControlInfo())!=NULL)
    {
        error("(%s)%s : TCP_lwip doesn't work on IPv6", tcpsegP->getClassName(), tcpsegP->getName());
    }
    else
    {
        error("(%s)%s arrived without control info", tcpsegP->getClassName(), tcpsegP->getName());
    }

    // process segment
    size_t ipHdrLen = sizeof(ip_hdr);
    size_t const maxBufferSize = 4096;
    char *data = new char[maxBufferSize];
    memset(data, 0, maxBufferSize);

    ip *ih = (ip *)data;
    tcphdr *tcph = (tcphdr *)(data + ipHdrLen);
    // set IP header:
    ih->ip_v = 4;
    ih->ip_hl = ipHdrLen/4;
    ih->ip_tos = 0;
    ih->ip_id = htons(tcpsegP->getSequenceNo());
    ih->ip_off = htons(0x4000);   // don't fragment, offset = 0;
    ih->ip_ttl = 64;
    ih->ip_p = 6;       // TCP
    ih->ip_sum = 0;
    ih->ip_src.s_addr = htonl(srcAddr.get4().getInt());
    ih->ip_dst.s_addr = htonl(destAddr.get4().getInt());

    size_t totalTcpLen = maxBufferSize - ipHdrLen;

    totalTcpLen = TCPSerializer().serialize(tcpsegP, (unsigned char *)tcph, totalTcpLen);

    // calculate TCP checksum
    tcph->th_sum = 0;
    tcph->th_sum = TCPSerializer().checksum(tcph, totalTcpLen, srcAddr, destAddr);

    size_t totalIpLen = ipHdrLen + totalTcpLen;
    ih->ip_len = htons(totalIpLen);
    ih->ip_sum = 0;
    ih->ip_sum = TCPIPchecksum::checksum(ih, ipHdrLen);

    // search unfilled local addr in pcb-s for this connection.
    TcpAppConnMap::iterator i;
    u32_t laddr = ih->ip_dst.s_addr;
    u32_t raddr = ih->ip_src.s_addr;
    u16_t lport = tcpsegP->getDestPort();
    u16_t rport = tcpsegP->getSrcPort();

    if(tcpsegP->getSynBit() && tcpsegP->getAckBit() )
    {
        for(i = tcpAppConnMapM.begin(); i != tcpAppConnMapM.end(); i++)
        {
            LwipTcpLayer::tcp_pcb *pcb = i->second->pcbM;
            if(pcb)
            {
                if(  (pcb->state == LwipTcpLayer::SYN_SENT)
                     && (pcb->local_ip.addr == 0)
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
    pCurTcpSegM = NULL;

    // LwipTcpLayer will call the tcp_event_recv() / tcp_event_err() and/or send a packet to sender

    // TODO we must save info from tcpseg to conn->receiveQueue if packet accepted.

    delete [] data;
    delete tcpsegP;
}

void TCP_lwip::notifyAboutIncomingSegmentProcessing(LwipTcpLayer::tcp_pcb *pcb, uint32 seqNo, void *dataptr, int len)
{
    TcpLwipConnection *conn = (pcb != NULL) ? (TcpLwipConnection *)(pcb->callback_arg) : NULL;
    if(conn)
    {
    	// TODO call queue, for save payload data from received packet
    	conn->receiveQueueM->insertBytesFromSegment(pCurTcpSegM, seqNo, dataptr, len);
    }
    else
    {
    	tcpEV << "notifyAboutIncomingSegmentProcessing: conn is null\n";
    }
}

void TCP_lwip::lwip_free_pcb_event(LwipTcpLayer::tcp_pcb* pcb)
{
    TcpLwipConnection *conn = (TcpLwipConnection *)(pcb->callback_arg);
    if (conn != NULL)
    {
        if(conn->pcbM == pcb)
        {
            // conn->sendIndicationToApp(TCP_I_????); // TODO send some indication when need
            removeConnection(*conn);
        }
    }
}

err_t TCP_lwip::lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
         LwipTcpLayer::lwip_event event, struct pbuf *p, u16_t size, err_t err)
{
    TcpLwipConnection *conn = (TcpLwipConnection *)arg;
    assert(conn != NULL);

    switch(event)
    {
    case LwipTcpLayer::LWIP_EVENT_ACCEPT:
        err = tcp_event_accept(*conn, pcb, err);
        break;

    case LwipTcpLayer::LWIP_EVENT_SENT:
        assert(conn->pcbM == pcb);
        err = tcp_event_sent(*conn, size);
        break;

    case LwipTcpLayer::LWIP_EVENT_RECV:
        assert(conn->pcbM == pcb);
        err = tcp_event_recv(*conn, p, err);
        break;

    case LwipTcpLayer::LWIP_EVENT_CONNECTED:
        assert(conn->pcbM == pcb);
        err = tcp_event_conn(*conn, err);
        break;

    case LwipTcpLayer::LWIP_EVENT_POLL:
        assert(conn->pcbM == pcb);
        err = tcp_event_poll(*conn);
        break;

    case LwipTcpLayer::LWIP_EVENT_ERR:
        err = tcp_event_err(*conn, err);
        break;

    default:
        error("Invalid lwip_event: %d", event);
        break;
    }

    return err;
}

err_t TCP_lwip::tcp_event_accept(TcpLwipConnection &conn, LwipTcpLayer::tcp_pcb *pcb, err_t err)
{
    int newConnId = ev.getUniqueNumber();
    TcpLwipConnection *newConn = new TcpLwipConnection(conn, newConnId, pcb);
    // add into appConnMap
    tcpAppConnMapM[newConnId] = newConn;

    newConn->sendEstablishedMsg();

    tcpEV << this << ": TCP_lwip: got accept!\n";
    conn.do_SEND();
    return err;
}

err_t TCP_lwip::tcp_event_sent(TcpLwipConnection &conn, u16_t size)
{
    conn.do_SEND();
    return ERR_OK;
}

err_t TCP_lwip::tcp_event_recv(TcpLwipConnection &conn, struct pbuf *p, err_t err)
{
    if(p == NULL)
    {
        // Received FIN:
        conn.sendIndicationToApp((conn.pcbM->state == LwipTcpLayer::TIME_WAIT)
        		? TCP_I_CLOSED : TCP_I_PEER_CLOSED);
        // TODO is it good?
        pLwipTcpLayerM->tcp_recved(conn.pcbM, 0);
    }
    else
    {
        conn.receiveQueueM->enqueueTcpLayerData(p->payload,p->len);
        pLwipTcpLayerM->tcp_recved(conn.pcbM, p->len);
        pbuf_free(p);
    }

    while(cPacket *dataMsg = conn.receiveQueueM->extractBytesUpTo())
    {
        // send Msg to Application layer:
        send(dataMsg, "appOut", conn.appGateIndexM);
    }
    conn.do_SEND();
    return err;
}

err_t TCP_lwip::tcp_event_conn(TcpLwipConnection &conn, err_t err)
{
    conn.sendEstablishedMsg();
    conn.do_SEND();
    return err;
}

void TCP_lwip::removeConnection(TcpLwipConnection &conn)
{
    conn.pcbM->callback_arg = NULL;
    conn.pcbM = NULL;
    tcpAppConnMapM.erase(conn.connIdM);
    delete &conn;
}

err_t TCP_lwip::tcp_event_err(TcpLwipConnection &conn, err_t err)
{
    tcpEV << this << ": tcp_event_err: " << err << " , conn Id: " << conn.connIdM << "\n";
    switch(err)
    {
    case ERR_ABRT:
        conn.sendIndicationToApp(TCP_I_CLOSED);
        removeConnection(conn);
        break;

    case ERR_RST:
        conn.sendIndicationToApp(TCP_I_CONNECTION_RESET);
        removeConnection(conn);
        break;

    default:
        opp_error("invalid LWIP error code: %d", err);
    }
    return err;
}

err_t TCP_lwip::tcp_event_poll(TcpLwipConnection &conn)
{
    conn.do_SEND();
    return ERR_OK;
}

struct netif * TCP_lwip::ip_route(IPvXAddress const & ipAddr)
{
    return &netIf;
}

void TCP_lwip::handleAppMessage(cMessage *msgP)
{
    TCPCommand *controlInfo = check_and_cast<TCPCommand *>(msgP->getControlInfo());
    int connId = controlInfo->getConnId();

    TcpLwipConnection *conn = findAppConn(connId);
    if (!conn)
    {
        TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(controlInfo);

        const char *sendQueueClass = openCmd->getSendQueueClass();
        if (!sendQueueClass || !sendQueueClass[0])
            sendQueueClass = this->par("sendQueueClass");

        const char *receiveQueueClass = openCmd->getReceiveQueueClass();
        if (!receiveQueueClass || !receiveQueueClass[0])
            receiveQueueClass = this->par("receiveQueueClass");

        // add into appConnMap
        conn = new TcpLwipConnection(*this, connId, msgP->getArrivalGate()->getIndex(),
                                     sendQueueClass, receiveQueueClass);
        tcpAppConnMapM[connId] = conn;

        tcpEV << this << ": TCP connection created for " << msgP << "\n";
    }
    processAppCommand(*conn, msgP);
}

simtime_t roundTime(const simtime_t &timeP, int secSlicesP)
{
	int64_t scale = timeP.getScale()/secSlicesP;
	simtime_t ret = timeP;
	ret /= scale;
	ret *= scale;
	return ret;
}

void TCP_lwip::handleMessage(cMessage *msgP)
{
    if (msgP->isSelfMessage())
    {
        // timer expired
        if(msgP == pLwipFastTimerM)
        { // lwip fast timer
        	tcpEV << "Call tcp_fasttmr()\n";
            pLwipTcpLayerM->tcp_fasttmr();
            scheduleAt(msgP->getArrivalTime() + 0.250, msgP);
            if (simTime() == roundTime(simTime(), 2))
            {
            	tcpEV << "Call tcp_slowtmr()\n";
                pLwipTcpLayerM->tcp_slowtmr();
            }
        }
        else
        {
        	error("Unknown self message");
        }
    }
    else if (msgP->arrivedOn("ipIn") || msgP->arrivedOn("ipv6In"))
    {
        tcpEV << this << ": handle msg: " << msgP->getName() << "\n";
        // must be a TCPSegment
        TCPSegment *tcpseg = dynamic_cast<TCPSegment *>(msgP);
        if(tcpseg)
        {
            handleIpInputMessage(tcpseg);
        }
        else
        {
            //TODO came some other message:
            //e.g: ICMPMessage
        }
    }
    else // must be from app
    {
        tcpEV << this << ": handle msg: " << msgP->getName() << "\n";
        handleAppMessage(msgP);
    }

    if (ev.isGUI())
        updateDisplayString();
}

void TCP_lwip::updateDisplayString()
{
    // TODO implementing it...
}

TcpLwipConnection *TCP_lwip::findAppConn(int connIdP)
{
    TcpAppConnMap::iterator i = tcpAppConnMapM.find(connIdP);
    return i==tcpAppConnMapM.end() ? NULL : (i->second);
}

void TCP_lwip::finish()
{
    isAliveM = false;
}

void TCP_lwip::printConnBrief(TcpLwipConnection& connP)
{
    tcpEV << this << ": connId=" << connP.connIdM << " appGateIndex=" << connP.appGateIndexM;
}

void TCP_lwip::loadStack()
{
    pLwipTcpLayerM = new LwipTcpLayer(*this);

    tcpEV << "TCP_lwip " << this << " has stack " << pLwipTcpLayerM << "\n";

    fprintf(stderr, "Created stack = %p\n", pLwipTcpLayerM);

    fprintf(stderr, "Initialising LWIP stack\n");


    fprintf(stderr, "done.\n");

    pLwipFastTimerM = new cMessage("lwip_fast_timer");
    scheduleAt(0.250, pLwipFastTimerM);
}

void TCP_lwip::ip_output(LwipTcpLayer::tcp_pcb *pcb, IPvXAddress const& srcP, IPvXAddress const& destP, void *dataP, int lenP)
{
    TcpLwipConnection *conn = (pcb != NULL) ? (TcpLwipConnection *)(pcb->callback_arg) : NULL;

    TCPSegment *tcpseg;
    if(conn)
    {
        tcpseg = conn->sendQueueM->createSegmentWithBytes(dataP, lenP);
    }
    else
    {
        tcpseg = new TCPSegment("tcp-segment");

        TCPSerializer().parse((const unsigned char *)dataP, lenP, tcpseg);
    }
    ASSERT(tcpseg);

    tcpEV << this << ": Sending: conn=" << conn << ", data: " << dataP << " of len " << lenP <<
            " from " << srcP << " to " << destP << "\n";

    const char* output = "";

    if (!destP.isIPv6())
    {
        // send over IPv4
        IPControlInfo *controlInfo = new IPControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(srcP.get4());
        controlInfo->setDestAddr(destP.get4());
        tcpseg->setControlInfo(controlInfo);

        output = "ipOut";
    }
    else
    {
        // send over IPv6
        opp_error("LWIP doesn't work on IPv6!");
    }
    if(conn)
    {
        conn->receiveQueueM->notifyAboutSending(tcpseg);
    }

    send(tcpseg, output);
}

void TCP_lwip::processAppCommand(TcpLwipConnection& connP, cMessage *msgP)
{
    printConnBrief(connP);

    // first do actions
    TCPCommand *tcpCommand = check_and_cast<TCPCommand *>(msgP->removeControlInfo());

    switch (msgP->getKind())
    {
        case TCP_C_OPEN_ACTIVE: process_OPEN_ACTIVE(connP, check_and_cast<TCPOpenCommand *>(tcpCommand), msgP); break;
        case TCP_C_OPEN_PASSIVE: process_OPEN_PASSIVE(connP, check_and_cast<TCPOpenCommand *>(tcpCommand), msgP); break;
        case TCP_C_SEND: process_SEND(connP, check_and_cast<TCPSendCommand *>(tcpCommand), check_and_cast<cPacket*>(msgP)); break;
        case TCP_C_CLOSE: process_CLOSE(connP, tcpCommand, msgP); break;
        case TCP_C_ABORT: process_ABORT(connP, tcpCommand, msgP); break;
        case TCP_C_STATUS: process_STATUS(connP, tcpCommand, msgP); break;
        default: opp_error("wrong command from app: %d", msgP->getKind());
    }
}

void TCP_lwip::process_OPEN_ACTIVE(TcpLwipConnection& connP, TCPOpenCommand *tcpCommandP, cMessage *msgP)
{

    if (tcpCommandP->getRemoteAddr().isUnspecified() || tcpCommandP->getRemotePort() == -1)
        opp_error("Error processing command OPEN_ACTIVE: remote address and port must be specified");

    tcpEV << this << ": OPEN: "
        << tcpCommandP->getLocalAddr() << ":" << tcpCommandP->getLocalPort() << " --> "
        << tcpCommandP->getRemoteAddr() << ":" << tcpCommandP->getRemotePort() << "\n";

    ASSERT(pLwipTcpLayerM);

    connP.connect(tcpCommandP->getLocalAddr(), tcpCommandP->getLocalPort(), tcpCommandP->getRemoteAddr(), tcpCommandP->getRemotePort());

    delete tcpCommandP;
    delete msgP;
}

void TCP_lwip::process_OPEN_PASSIVE(TcpLwipConnection& connP, TCPOpenCommand *tcpCommandP, cMessage *msgP)
{
    ASSERT(pLwipTcpLayerM);

    ASSERT(tcpCommandP->getFork()==true);

    if (tcpCommandP->getLocalPort() == -1)
        opp_error("Error processing command OPEN_PASSIVE: local port must be specified");

    tcpEV << this << "Starting to listen on: " << tcpCommandP->getLocalAddr() << ":" << tcpCommandP->getLocalPort() << "\n";

    /*
    process passive open request
    */

    connP.listen(tcpCommandP->getLocalAddr(), tcpCommandP->getLocalPort());

    delete tcpCommandP;
    delete msgP;
}

void TCP_lwip::process_SEND(TcpLwipConnection& connP, TCPSendCommand *tcpCommandP, cPacket *msgP)
{
    delete tcpCommandP;

    connP.send(msgP);
}

void TCP_lwip::process_CLOSE(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    tcpEV << this << ": process_CLOSE()\n";

    delete tcpCommandP;
    delete msgP;

    connP.close();
}

void TCP_lwip::process_ABORT(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    tcpEV << this << ": process_ABORT()\n";

    delete tcpCommandP;
    delete msgP;

    connP.abort();
}

void TCP_lwip::process_STATUS(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP)
{
    delete tcpCommandP; // but we'll reuse msg for reply

    TCPStatusInfo *statusInfo = new TCPStatusInfo();
    connP.fillStatusInfo(*statusInfo);
    msgP->setControlInfo(statusInfo);
    send(msgP, "appOut", connP.appGateIndexM);
}
