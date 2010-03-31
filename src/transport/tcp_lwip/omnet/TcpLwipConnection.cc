//
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


#include "TcpLwipConnection.h"

#include "headers/defs.h"   // for endian macros
#include "IPControlInfo.h"
#include "IPv6ControlInfo.h"
#include "headers/tcp.h"
#include "lwip/tcp.h"
#include "TCP_lwip.h"
#include "TcpLwipQueues.h"
#include "TCPCommand_m.h"
#include "TCPIPchecksum.h"
#include "TCPSegment.h"
#include "TCPSerializer.h"

#include <assert.h>
#include <dlfcn.h>

// macro for normal ev<< logging (note: deliberately no parens in macro def)
// FIXME
//#define tcpEV (((ev.disable_tracing)||(TCP_lwip::testingS)) ? (std::cout) : (ev))

#define tcpEV ev
//#define tcpEV std::cout


TcpLwipConnection::TcpLwipConnection(TCP_lwip &tcpLwipP, int connIdP, int gateIndexP, const char *sendQueueClassP, const char *recvQueueClassP)
    :
    connIdM(connIdP),
    appGateIndexM(gateIndexP),
    onCloseM(false),
    isListenerM(false),
    tcpLwipM(tcpLwipP),
    receiveQueueM(check_and_cast<TcpLwipReceiveQueue *>(createOne(recvQueueClassP))),
    sendQueueM(check_and_cast<TcpLwipSendQueue *>(createOne(sendQueueClassP)))
{
    pcbM = tcpLwipM.getLwipTcpLayer()->tcp_new();
    assert(pcbM);

    pcbM->callback_arg = this;

    sendQueueM->setConnection(this);
    receiveQueueM->setConnection(this);
}

TcpLwipConnection::TcpLwipConnection(TcpLwipConnection &connP, int connIdP, LwipTcpLayer::tcp_pcb *pcbP)
    :
    connIdM(connIdP),
    appGateIndexM(connP.appGateIndexM),
    onCloseM(false),
    isListenerM(false),
    tcpLwipM(connP.tcpLwipM),
    receiveQueueM(check_and_cast<TcpLwipReceiveQueue *>(createOne(connP.receiveQueueM->getClassName()))),
    sendQueueM(check_and_cast<TcpLwipSendQueue *>(createOne(connP.sendQueueM->getClassName())))
{
    pcbM = pcbP;
    pcbM->callback_arg = this;

    sendQueueM->setConnection(this);
    receiveQueueM->setConnection(this);
}

void TcpLwipConnection::sendEstablishedMsg()
{
    cMessage *msg = new cMessage("TCP_I_ESTABLISHED");
    msg->setKind(TCP_I_ESTABLISHED);

    TCPConnectInfo *tcpConnectInfo = new TCPConnectInfo();

    IPvXAddress localAddr(ntohl(pcbM->local_ip.addr)), remoteAddr(ntohl(pcbM->remote_ip.addr));

    tcpConnectInfo->setConnId(connIdM);
    tcpConnectInfo->setLocalAddr(localAddr);
    tcpConnectInfo->setRemoteAddr(remoteAddr);
    tcpConnectInfo->setLocalPort(pcbM->local_port);
    tcpConnectInfo->setRemotePort(pcbM->remote_port);

    msg->setControlInfo(tcpConnectInfo);

    tcpLwipM.send(msg, "appOut", appGateIndexM);
}

const char *TcpLwipConnection::indicationName(int code)
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

void TcpLwipConnection::sendIndicationToApp(int code)
{
    tcpEV << "Notifying app: " << indicationName(code) << "\n";
    cMessage *msg = new cMessage(indicationName(code));
    msg->setKind(code);
    TCPCommand *ind = new TCPCommand();
    ind->setConnId(connIdM);
    msg->setControlInfo(ind);
    tcpLwipM.send(msg, "appOut", appGateIndexM);
}

void TcpLwipConnection::fillStatusInfo(TCPStatusInfo &statusInfo)
{
//TODO    statusInfo.setState(fsm.getState());
//TODO    statusInfo.setStateName(stateName(fsm.getState()));

    statusInfo.setLocalAddr(IPvXAddress(ntohl(pcbM->local_ip.addr)));
    statusInfo.setLocalPort(pcbM->local_port);
    statusInfo.setRemoteAddr(IPvXAddress(ntohl(pcbM->remote_ip.addr)));
    statusInfo.setRemotePort(pcbM->remote_port);

    statusInfo.setSnd_mss(pcbM->mss);
//TODO    statusInfo.setSnd_una(pcbM->snd_una);
    statusInfo.setSnd_nxt(pcbM->snd_nxt);
//TODO    statusInfo.setSnd_max(pcbM->snd_max);
    statusInfo.setSnd_wnd(pcbM->snd_wnd);
//TODO    statusInfo.setSnd_up(pcbM->snd_up);
    statusInfo.setSnd_wl1(pcbM->snd_wl1);
    statusInfo.setSnd_wl2(pcbM->snd_wl2);
//TODO    statusInfo.setIss(pcbM->iss);
    statusInfo.setRcv_nxt(pcbM->rcv_nxt);
    statusInfo.setRcv_wnd(pcbM->rcv_wnd);
//TODO    statusInfo.setRcv_up(pcbM->rcv_up);
//TODO    statusInfo.setIrs(pcbM->irs);
//TODO    statusInfo.setFin_ack_rcvd(pcbM->fin_ack_rcvd);
}

void TcpLwipConnection::listen(IPvXAddress& localAddr, unsigned short localPort)
{
    tcpLwipM.getLwipTcpLayer()->tcp_bind(pcbM, NULL, localPort);
    // The next returns a tcp_pcb: need to do some research on how
    // it works; does it actually accept a connection as well? It
    // shouldn't, as there is a tcp_accept
    LwipTcpLayer::tcp_pcb *pcb = pcbM;
    pcbM = NULL; // unlink old pcb from this, otherwise lwip_free_pcb_event destroy this conn.
    pcbM = tcpLwipM.getLwipTcpLayer()->tcp_listen(pcb);
}

void TcpLwipConnection::connect(IPvXAddress& localAddr, unsigned short localPort, IPvXAddress& remoteAddr, unsigned short remotePort)
{
    struct ip_addr dest_addr;
    dest_addr.addr = htonl(remoteAddr.get4().getInt());
    tcpLwipM.getLwipTcpLayer()->tcp_connect(pcbM, &dest_addr, (u16_t)remotePort, NULL);
}

void TcpLwipConnection::close()
{
    tcpLwipM.getLwipTcpLayer()->tcp_close(pcbM);
}

void TcpLwipConnection::abort()
{
    tcpLwipM.getLwipTcpLayer()->tcp_close(pcbM);
}

void TcpLwipConnection::send(cPacket *msgP)
{
    sendQueueM->enqueueAppData(msgP);
}

int TcpLwipConnection::send_data(void *data, int datalen)
{
    int error;

    if (datalen > 0xFFFF)
      datalen = 0xFFFF; // tcp_write() length argument is uint16_t
    error = tcpLwipM.getLwipTcpLayer()->tcp_write(pcbM, data, datalen, 1);

    if(error == ERR_OK)
        return datalen;

    if(error == ERR_MEM) {
        // Chances are that datalen is too large to fit in the send
        // buffer. If it is really large (larger than a typical MSS,
        // say), we should try segmenting the data ourselves.
        int written = 0;

        if(datalen < pcbM->snd_buf)
//           return ERR_EAGAIN;
            return written; //TODO
        do
        {
            u16_t snd_buf = pcbM->snd_buf;
            if(0 == snd_buf)
                break;
            error = tcpLwipM.getLwipTcpLayer()->tcp_write(
                    pcbM, ((const char *)data) + written, snd_buf, 1);
            if(error == ERR_OK)
            {
                written += snd_buf;
                datalen -= snd_buf;
            }
        } while(error == ERR_OK && datalen >= pcbM->snd_buf);

        if(written > 0)
            return written;
    }

    return error;
}

void TcpLwipConnection::do_SEND()
{
    char buffer[4096];
    int bytes;
    int allsent = 0;

    while(0 != (bytes = sendQueueM->getBytesForTcpLayer(buffer, sizeof(buffer))))
    {
        int sent = send_data(buffer, bytes);

        if(sent > 0)
        {
            sendQueueM->dequeueTcpLayerMsg(sent);
            allsent += sent;
        }
        else
        {
            tcpEV << "TCP_lwip connection: " << connIdM << ": Error do sending, err is " << sent << "\n";
            break;

        }
    }
    tcpEV << "do_SEND(): " << connIdM << " sent:" << allsent << ", unsent:" << sendQueueM->getBytesAvailable() << "\n";
}
