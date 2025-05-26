//
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp_lwip/TcpLwipConnection.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_common/headers/tcphdr.h"
#include "inet/transportlayer/tcp_lwip/TcpLwip.h"
#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"
#include "lwip/lwip_tcp.h"

namespace inet {
namespace tcp {

Define_Module(TcpLwipConnection);

simsignal_t TcpLwipConnection::sndWndSignal = registerSignal("sndWnd"); // snd_wnd
simsignal_t TcpLwipConnection::sndNxtSignal = registerSignal("sndNxt"); // sent seqNo
simsignal_t TcpLwipConnection::sndAckSignal = registerSignal("sndAck"); // sent ackNo
simsignal_t TcpLwipConnection::rcvWndSignal = registerSignal("rcvWnd"); // rcv_wnd
simsignal_t TcpLwipConnection::rcvSeqSignal = registerSignal("rcvSeq"); // received seqNo
simsignal_t TcpLwipConnection::rcvAckSignal = registerSignal("rcvAck"); // received ackNo (=snd_una)

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
    pcbM = tcpLwipM->getLwipTcpLayer()->tcp_new(); // FIXME memory leak
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
    delete receiveQueueM;
    delete sendQueueM;
    if (pcbM) {
        pcbM->callback_arg = nullptr;
        tcpLwipM->getLwipTcpLayer()->tcp_pcb_purge(pcbM);
        memp_free(MEMP_TCP_PCB, pcbM);
        pcbM = nullptr;
    }
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
    ind->setAutoRead(autoRead);

    indication->setControlInfo(ind);
    indication->addTag<TransportProtocolInd>()->setProtocol(&Protocol::tcp);
    indication->addTag<SocketInd>()->setSocketId(listenConnId);
    tcpLwipM->send(indication, "appOut");
    // TODO shouldn't read from lwip until accept arrived
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
    tcpConnectInfo->setAutoRead(autoRead);

    indication->setControlInfo(tcpConnectInfo);
    indication->addTag<TransportProtocolInd>()->setProtocol(&Protocol::udp);
    indication->addTag<SocketInd>()->setSocketId(connIdM);

    tcpLwipM->send(indication, "appOut");
    sendUpEnabled = true;
    do_SEND();
    // TODO now can read from lwip
    sendUpData();
}

const char *TcpLwipConnection::indicationName(int code)
{
#define CASE(x)    case x: \
        s = #x; s += 6; break
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
    // TODO    statusInfo.setState(fsm.getState());
    // TODO    statusInfo.setStateName(stateName(fsm.getState()));

    statusInfo.setLocalAddr((pcbM->local_ip.addr));
    statusInfo.setLocalPort(pcbM->local_port);
    statusInfo.setRemoteAddr((pcbM->remote_ip.addr));
    statusInfo.setRemotePort(pcbM->remote_port);
    statusInfo.setAutoRead(autoRead);

    statusInfo.setSnd_mss(pcbM->mss);
    // TODO    statusInfo.setSnd_una(pcbM->snd_una);
    statusInfo.setSnd_nxt(pcbM->snd_nxt);
    // TODO    statusInfo.setSnd_max(pcbM->snd_max);
    statusInfo.setSnd_wnd(pcbM->snd_wnd);
    // TODO    statusInfo.setSnd_up(pcbM->snd_up);
    statusInfo.setSnd_wl1(pcbM->snd_wl1);
    statusInfo.setSnd_wl2(pcbM->snd_wl2);
    // TODO    statusInfo.setIss(pcbM->iss);
    statusInfo.setRcv_nxt(pcbM->rcv_nxt);
    statusInfo.setRcv_wnd(pcbM->rcv_wnd);
    // TODO    statusInfo.setRcv_up(pcbM->rcv_up);
    // TODO    statusInfo.setIrs(pcbM->irs);
    // TODO    statusInfo.setFin_ack_rcvd(pcbM->fin_ack_rcvd);
}

void TcpLwipConnection::listen(const L3Address& localAddr, unsigned short localPort)
{
    onCloseM = false;
    tcpLwipM->getLwipTcpLayer()->tcp_bind(pcbM, nullptr, localPort);
    // The next returns a tcp_pcb: need to do some research on how
    // it works; does it actually accept a connection as well? It
    // shouldn't, as there is a tcp_accept
    LwipTcpLayer::tcp_pcb *pcb = pcbM;
    pcbM = nullptr; // unlink old pcb from this, otherwise lwip_free_pcb_event destroy this conn.
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

            error = tcpLwipM->getLwipTcpLayer()->tcp_write(pcbM, ((const char *)data) + written, snd_buf, 1);

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
        while (true) {
            B len = receiveQueueM->getExtractableBytesUpTo();
            if (!autoRead && len > B(maxByteCountRequested))
                len = B(maxByteCountRequested);
            Packet *dataMsg = len > b(0) ? receiveQueueM->extractBytesUpTo(len) : nullptr;
            if (dataMsg == nullptr)
                break;
            dataMsg->setKind(TCP_I_DATA);
            dataMsg->addTag<TransportProtocolInd>()->setProtocol(&Protocol::tcp);
            dataMsg->addTag<SocketInd>()->setSocketId(connIdM);
            tcpLwipM->send(dataMsg, "appOut");
            if (!autoRead)
                maxByteCountRequested = 0;
        }
    }
}

void TcpLwipConnection::processAppCommand(cMessage *msgP)
{
    // first do actions
    TcpCommand *tcpCommand = check_and_cast_nullable<TcpCommand *>(msgP->removeControlInfo());

    switch (msgP->getKind()) {
        case TCP_C_OPEN_ACTIVE:
            process_OPEN_ACTIVE(check_and_cast<TcpOpenCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_OPEN_PASSIVE:
            process_OPEN_PASSIVE(check_and_cast<TcpOpenCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_ACCEPT:
            process_ACCEPT(check_and_cast<TcpAcceptCommand *>(tcpCommand), msgP);
            break;

        case TCP_C_SEND:
            process_SEND(check_and_cast<Packet *>(msgP));
            break;

        case TCP_C_CLOSE:
            ASSERT(tcpCommand);
            process_CLOSE(tcpCommand, msgP);
            break;

        case TCP_C_ABORT:
            ASSERT(tcpCommand);
            process_ABORT(tcpCommand, msgP);
            break;

        case TCP_C_STATUS:
            ASSERT(tcpCommand);
            process_STATUS(tcpCommand, msgP);
            break;

        case TCP_C_READ:
            process_READ_REQUEST(tcpCommand, msgP);
            break;

        default:
            throw cRuntimeError("Wrong command from app: %d", msgP->getKind());
    }
}

void TcpLwipConnection::process_OPEN_ACTIVE(TcpOpenCommand *tcpCommandP, cMessage *msgP)
{
    ASSERT(tcpLwipM->getLwipTcpLayer());

    if (tcpCommandP->getRemoteAddr().isUnspecified() || tcpCommandP->getRemotePort() == -1)
        throw cRuntimeError("Error processing command OPEN_ACTIVE: remote address and port must be specified");

    int localPort = tcpCommandP->getLocalPort();
    if (localPort == -1)
        localPort = 0;
    autoRead = tcpCommandP->getAutoRead();

    EV_INFO << this << ": OPEN: "
            << tcpCommandP->getLocalAddr() << ":" << localPort << " --> "
            << tcpCommandP->getRemoteAddr() << ":" << tcpCommandP->getRemotePort() << "\n";
    connect(tcpCommandP->getLocalAddr(), localPort,
            tcpCommandP->getRemoteAddr(), tcpCommandP->getRemotePort());
    delete tcpCommandP;
    delete msgP;
}

void TcpLwipConnection::process_OPEN_PASSIVE(TcpOpenCommand *tcpCommandP,
        cMessage *msgP)
{
    ASSERT(tcpLwipM->getLwipTcpLayer());

    if (tcpCommandP->getFork() == false)
        throw cRuntimeError("Error processing command OPEN_PASSIVE: non-forking listening connections are not supported yet");

    if (tcpCommandP->getLocalPort() == -1)
        throw cRuntimeError("Error processing command OPEN_PASSIVE: local port must be specified");

    autoRead = tcpCommandP->getAutoRead();
    EV_INFO << this << "Starting to listen on: " << tcpCommandP->getLocalAddr() << ":"
            << tcpCommandP->getLocalPort() << "\n";
    listen(tcpCommandP->getLocalAddr(), tcpCommandP->getLocalPort());
    delete tcpCommandP;
    delete msgP;
}

void TcpLwipConnection::process_ACCEPT(TcpAcceptCommand *tcpCommand, cMessage *msg)
{
    accept();
    delete tcpCommand;
    delete msg;
}

void TcpLwipConnection::process_SEND(Packet *msgP)
{
    EV_INFO << this << ": processing SEND command, len=" << msgP->getByteLength() << endl;
    send(msgP);
}

void TcpLwipConnection::process_CLOSE(TcpCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing CLOSE(" << connIdM << ") command\n";
    delete tcpCommandP;
    delete msgP;
    close();
}

void TcpLwipConnection::process_ABORT(TcpCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing ABORT(" << connIdM << ") command\n";
    delete tcpCommandP;
    delete msgP;
    abort();
}

void TcpLwipConnection::process_STATUS(TcpCommand *tcpCommandP, cMessage *msgP)
{
    EV_INFO << this << ": processing STATUS(" << connIdM << ") command\n";
    delete tcpCommandP; // but we'll reuse msg for reply
    TcpStatusInfo *statusInfo = new TcpStatusInfo();
    fillStatusInfo(*statusInfo);
    msgP->setControlInfo(statusInfo);
    msgP->setKind(TCP_I_STATUS);
    tcpLwipM->send(msgP, "appOut");
}

void TcpLwipConnection::process_READ_REQUEST(TcpCommand *tcpCommand, cMessage *msg)
{
    if (autoRead)
        throw cRuntimeError("TCP READ arrived, but connection used in autoRead mode");
    //check whether we have data in the TCP queue. Store how much data the application wants. Check for pending read request.
    TcpReadCommand *readCmd = check_and_cast<TcpReadCommand *>(tcpCommand);
    if (readCmd->getMaxByteCount() <= 0)
        throw cRuntimeError("Illegal argument: numberOfBytes in TCP READ command is negative or zero.");
    if (maxByteCountRequested != 0)
        throw cRuntimeError("A second TCP READ command arrived before data for the previous READ was sent up");
    maxByteCountRequested = readCmd->getMaxByteCount();
    if (!sendUpEnabled)
        throw cRuntimeError("READ without ACCEPT");
    sendUpData();
    delete msg;
}

} // namespace tcp
} // namespace inet

