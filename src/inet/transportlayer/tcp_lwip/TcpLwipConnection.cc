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


#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/packet/Message.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_common/headers/tcphdr.h"
#include "lwip/lwip_tcp.h"
#include "inet/transportlayer/tcp_lwip/TcpLwip.h"
#include "inet/transportlayer/tcp_lwip/TcpLwipConnection.h"
#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"

namespace inet {
namespace tcp {

Define_Module(TcpLwipConnection);

simsignal_t TcpLwipConnection::sndWndSignal = registerSignal("sndWnd");    // snd_wnd
simsignal_t TcpLwipConnection::sndNxtSignal = registerSignal("sndNxt");    // sent seqNo
simsignal_t TcpLwipConnection::sndAckSignal = registerSignal("sndAck");    // sent ackNo
simsignal_t TcpLwipConnection::rcvWndSignal = registerSignal("rcvWnd");    // rcv_wnd
simsignal_t TcpLwipConnection::rcvSeqSignal = registerSignal("rcvSeq");    // received seqNo
simsignal_t TcpLwipConnection::rcvAckSignal = registerSignal("rcvAck");    // received ackNo (=snd_una)

void TcpLwipConnection::recordSend(const TcpHeader& tcpsegP)
{
    emit(sndWndSignal, tcpsegP.getWindow());
    emit(sndNxtSignal, tcpsegP.getSequenceNo());

    if (tcpsegP.getAckBit())
        emit(sndAckSignal, tcpsegP.getAckNo());
}

void TcpLwipConnection::recordReceive(const TcpHeader& tcpsegP)
{
    emit(rcvWndSignal, tcpsegP.getWindow());
    emit(rcvSeqSignal, tcpsegP.getSequenceNo());

    if (tcpsegP.getAckBit())
        emit(rcvAckSignal, tcpsegP.getAckNo());
}

void TcpLwipConnection::initConnection(TcpLwip& tcpLwipP, int connIdP)
{
    connIdM = connIdP;
    sendQueueM = tcpLwipP.createSendQueue();
    receiveQueueM = tcpLwipP.createReceiveQueue();
    tcpLwipM = &tcpLwipP;
    totalSentM = 0;
    isListenerM = false;
    onCloseM = false;
    ASSERT(!pcbM);
    pcbM = tcpLwipM->getLwipTcpLayer()->tcp_new();       //FIXME memory leak
    ASSERT(pcbM);
    pcbM->callback_arg = this;
    sendQueueM->setConnection(this);
    receiveQueueM->setConnection(this);
}

void TcpLwipConnection::initConnection(TcpLwipConnection& connP, int connIdP, LwipTcpLayer::tcp_pcb *pcbP)
{
    connIdM = connIdP;
    sendQueueM = check_and_cast<TcpLwipSendQueue *>(inet::utils::createOne(connP.sendQueueM->getClassName()));
    receiveQueueM = check_and_cast<TcpLwipReceiveQueue *>(inet::utils::createOne(connP.receiveQueueM->getClassName()));
    tcpLwipM = connP.tcpLwipM;
    totalSentM = 0;
    isListenerM = false;
    onCloseM = false;
    ASSERT(!pcbM);
    pcbM = pcbP;
    pcbM->callback_arg = this;
    sendQueueM->setConnection(this);
    receiveQueueM->setConnection(this);
}

TcpLwipConnection::~TcpLwipConnection()
{
#if OMNETPP_BUILDNUM < 1505   //OMNETPP_VERSION < 0x0600    // 6.0 pre9
    ASSERT(!pcbM);
#endif
    delete receiveQueueM;
    delete sendQueueM;
#if OMNETPP_BUILDNUM >= 1505   //OMNETPP_VERSION >= 0x0600    // 6.0 pre9
    if (pcbM) {
        pcbM->callback_arg = nullptr;
        tcpLwipM->getLwipTcpLayer()->tcp_pcb_purge(pcbM);
        memp_free(MEMP_TCP_PCB, pcbM);
        pcbM = nullptr;
    }
#endif
}

void TcpLwipConnection::sendAvailableIndicationToApp(int listenConnId)
{
    EV_INFO << "Notifying app: " << indicationName(TCP_I_AVAILABLE) << "\n";
    auto indication = new Indication(indicationName(TCP_I_AVAILABLE), TCP_I_AVAILABLE);
    L3Address localAddr(pcbM->local_ip.addr), remoteAddr(pcbM->remote_ip.addr);

    TcpAvailableInfo *ind = new TcpAvailableInfo();
    ind->setNewSocketId(connIdM);
    ind->setLocalAddr(localAddr);
    ind->setRemoteAddr(remoteAddr);
    ind->setLocalPort(pcbM->local_port);
    ind->setRemotePort(pcbM->remote_port);

    indication->setControlInfo(ind);
    indication->addTag<TransportProtocolInd>()->setProtocol(&Protocol::tcp);
    indication->addTag<SocketInd>()->setSocketId(listenConnId);
    tcpLwipM->send(indication, "appOut");
    //TODO shouldn't read from lwip until accept arrived
}

void TcpLwipConnection::sendEstablishedMsg()
{
    auto indication = new Indication("TCP_I_ESTABLISHED", TCP_I_ESTABLISHED);
    TcpConnectInfo *tcpConnectInfo = new TcpConnectInfo();

    L3Address localAddr(pcbM->local_ip.addr), remoteAddr(pcbM->remote_ip.addr);

    tcpConnectInfo->setLocalAddr(localAddr);
    tcpConnectInfo->setRemoteAddr(remoteAddr);
    tcpConnectInfo->setLocalPort(pcbM->local_port);
    tcpConnectInfo->setRemotePort(pcbM->remote_port);

    indication->setControlInfo(tcpConnectInfo);
    indication->addTag<TransportProtocolInd>()->setProtocol(&Protocol::udp);
    indication->addTag<SocketInd>()->setSocketId(connIdM);

    tcpLwipM->send(indication, "appOut");
    sendUpEnabled = true;
    do_SEND();
    //TODO now can read from lwip
    sendUpData();
}

const char *TcpLwipConnection::indicationName(int code)
{
#define CASE(x)    case x: \
        s = (const char *)(#x) + 6; break
    const char *s = "unknown";

    switch (code) {
        CASE(TCP_I_DATA);
        CASE(TCP_I_URGENT_DATA);
        CASE(TCP_I_AVAILABLE);
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
    const char *nameOfIndication = indicationName(code);
    EV_DETAIL << "Notifying app: " << nameOfIndication << "\n";
    auto indication = new Indication(nameOfIndication, code);
    TcpCommand *ind = new TcpCommand();
    indication->setControlInfo(ind);
    indication->addTag<TransportProtocolInd>()->setProtocol(&Protocol::tcp);
    indication->addTag<SocketInd>()->setSocketId(connIdM);
    tcpLwipM->send(indication, "appOut");
}

void TcpLwipConnection::fillStatusInfo(TcpStatusInfo& statusInfo)
{
//TODO    statusInfo.setState(fsm.getState());
//TODO    statusInfo.setStateName(stateName(fsm.getState()));

    statusInfo.setLocalAddr((pcbM->local_ip.addr));
    statusInfo.setLocalPort(pcbM->local_port);
    statusInfo.setRemoteAddr((pcbM->remote_ip.addr));
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

void TcpLwipConnection::listen(const L3Address& localAddr, unsigned short localPort)
{
    onCloseM = false;
    tcpLwipM->getLwipTcpLayer()->tcp_bind(pcbM, nullptr, localPort);
    // The next returns a tcp_pcb: need to do some research on how
    // it works; does it actually accept a connection as well? It
    // shouldn't, as there is a tcp_accept
    LwipTcpLayer::tcp_pcb *pcb = pcbM;
    pcbM = nullptr;    // unlink old pcb from this, otherwise lwip_free_pcb_event destroy this conn.
    pcbM = tcpLwipM->getLwipTcpLayer()->tcp_listen(pcb);
    totalSentM = 0;
}

void TcpLwipConnection::connect(const L3Address& localAddr, unsigned short localPort,
        const L3Address& remoteAddr, unsigned short remotePort)
{
    onCloseM = false;
    struct ip_addr src_addr;
    src_addr.addr = localAddr;
    struct ip_addr dest_addr;
    dest_addr.addr = remoteAddr;
    tcpLwipM->getLwipTcpLayer()->tcp_bind(pcbM, &src_addr, localPort);
    tcpLwipM->getLwipTcpLayer()->tcp_connect(pcbM, &dest_addr, (u16_t)remotePort, nullptr);
    totalSentM = 0;
}

void TcpLwipConnection::close()
{
    onCloseM = true;

    if (0 == sendQueueM->getBytesAvailable()) {
        tcpLwipM->getLwipTcpLayer()->tcp_close(pcbM);
        onCloseM = false;
    }
}

void TcpLwipConnection::abort()
{
    tcpLwipM->getLwipTcpLayer()->tcp_abandon(pcbM, false);
    onCloseM = false;
}

void TcpLwipConnection::accept()
{
    if (sendUpEnabled)
        throw cRuntimeError("socket already accepted/not a fork of a listening socket");
    sendEstablishedMsg();
    do_SEND();
}

void TcpLwipConnection::send(Packet *msgP)
{
    sendQueueM->enqueueAppData(msgP);
}

void TcpLwipConnection::notifyAboutSending(const TcpHeader& tcpsegP)
{
    receiveQueueM->notifyAboutSending(&tcpsegP);
    recordSend(tcpsegP);
}

int TcpLwipConnection::send_data(void *data, int datalen)
{
    int error;
    int written = 0;

    if (datalen > 0xFFFF)
        datalen = 0xFFFF; // tcp_write() length argument is uint16_t

    u32_t ss = pcbM->snd_lbb;
    error = tcpLwipM->getLwipTcpLayer()->tcp_write(pcbM, data, datalen, 1);

    if (error == ERR_OK) {
        written = datalen;
    }
    else if (error == ERR_MEM) {
        // Chances are that datalen is too large to fit in the send
        // buffer. If it is really large (larger than a typical MSS,
        // say), we should try segmenting the data ourselves.
        while (1) {
            u16_t snd_buf = pcbM->snd_buf;
            if (0 == snd_buf)
                break;

            if (datalen < snd_buf)
                break;

            error = tcpLwipM->getLwipTcpLayer()->tcp_write(
                        pcbM, ((const char *)data) + written, snd_buf, 1);

            if (error != ERR_OK)
                break;

            written += snd_buf;
            datalen -= snd_buf;
        }
    }

    if (written > 0) {
        ASSERT(pcbM->snd_lbb - ss == (u32_t)written);
        return written;
    }

    return error;
}

void TcpLwipConnection::do_SEND()
{
    if (!sendUpEnabled)
        return;
    char buffer[8 * 536];
    int bytes;
    int allsent = 0;

    while (0 != (bytes = sendQueueM->getBytesForTcpLayer(buffer, sizeof(buffer)))) {
        int sent = send_data(buffer, bytes);

        if (sent > 0) {
            sendQueueM->dequeueTcpLayerMsg(sent);
            allsent += sent;
        }
        else {
            if (sent != ERR_MEM)
                EV_ERROR << "TcpLwip connection: " << connIdM << ": Error do sending, err is " << sent << endl;
            break;
        }
    }

    totalSentM += allsent;
    EV_DETAIL << "do_SEND(): " << connIdM
              << " send:" << allsent
              << ", unsent:" << sendQueueM->getBytesAvailable()
              << ", total sent:" << totalSentM
              << ", all bytes:" << totalSentM + sendQueueM->getBytesAvailable()
              << endl;

    if (onCloseM && (0 == sendQueueM->getBytesAvailable())) {
        tcpLwipM->getLwipTcpLayer()->tcp_close(pcbM);
        onCloseM = false;
    }
}

void TcpLwipConnection::sendUpData()
{
    if (sendUpEnabled) {
        while (Packet *dataMsg = receiveQueueM->extractBytesUpTo()) {
            dataMsg->setKind(TCP_I_DATA);
            dataMsg->addTag<TransportProtocolInd>()->setProtocol(&Protocol::tcp);
            dataMsg->addTag<SocketInd>()->setSocketId(connIdM);
//            int64_t len = dataMsg->getByteLength();
            // send Msg to Application layer:
            tcpLwipM->send(dataMsg, "appOut");
//            while (len > 0) {
//                unsigned int slen = len > 0xffff ? 0xffff : len;
//                tcpLwipM->getLwipTcpLayer()->tcp_recved(pcbM, slen);
//                len -= slen;
//            }
        }
    }
}

} // namespace tcp
} // namespace inet

