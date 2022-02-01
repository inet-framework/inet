//
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPLWIPCONNECTION_H
#define __INET_TCPLWIPCONNECTION_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"
#include "lwip/lwip_tcp.h"

namespace inet {
namespace tcp {

// forward declarations:
class TcpLwip;

/**
 * Module for representing a connection in TcpLwip stack
 */
class INET_API TcpLwipConnection : public cSimpleModule
{
  protected:
    // prevent copy constructor:
    TcpLwipConnection(const TcpLwipConnection&);

  public:
    TcpLwipConnection() {}
    ~TcpLwipConnection();

    void initConnection(TcpLwip& tcpLwipP, int connIdP);
    void initConnection(TcpLwipConnection& tcpLwipConnectionP, int connIdP, LwipTcpLayer::tcp_pcb *pcbP);

    /** Utility: sends TCP_I_AVAILABLE indication with TcpAvailableInfo to application */
    void sendAvailableIndicationToApp(int listenConnId);

    void sendEstablishedMsg();

    static const char *indicationName(int code);

    void sendIndicationToApp(int code);

    void notifyAboutSending(const TcpHeader& tcpsegP);

    void do_SEND();

    bool isSendUpEnabled() { return sendUpEnabled; }

    void sendUpData();

    void processAppCommand(cMessage *msgP);

  protected:
    void listen(const L3Address& localAddr, unsigned short localPort);
    void connect(const L3Address& localAddr, unsigned short localPort, const L3Address& remoteAddr, unsigned short remotePort);
    void close();
    void abort();
    void accept();
    void send(Packet *msgP);
    int send_data(void *data, int len);
    void recordSend(const TcpHeader& tcpsegP);
    void recordReceive(const TcpHeader& tcpsegP);
    void process_OPEN_ACTIVE(TcpOpenCommand *tcpCommandP, cMessage *msgP);
    void process_OPEN_PASSIVE(TcpOpenCommand *tcpCommandP, cMessage *msgP);
    void process_ACCEPT(TcpAcceptCommand *tcpCommand, cMessage *msg);
    void process_SEND(Packet *msgP);
    void process_CLOSE(TcpCommand *tcpCommandP, cMessage *msgP);
    void process_ABORT(TcpCommand *tcpCommandP, cMessage *msgP);
    void process_STATUS(TcpCommand *tcpCommandP, cMessage *msgP);
    void fillStatusInfo(TcpStatusInfo& statusInfo);

  public:
    int connIdM;
    LwipTcpLayer::tcp_pcb *pcbM = nullptr;
    TcpLwipSendQueue *sendQueueM = nullptr;
    TcpLwipReceiveQueue *receiveQueueM = nullptr;
    TcpLwip *tcpLwipM = nullptr;

  protected:
    long int totalSentM = 0;
    bool isListenerM = false;
    bool onCloseM = false;
    bool sendUpEnabled = false;

    // statistics
    static simsignal_t sndWndSignal; // snd_wnd
    static simsignal_t sndNxtSignal; // sent seqNo
    static simsignal_t sndAckSignal; // sent ackNo

    static simsignal_t rcvWndSignal; // rcv_wnd
    static simsignal_t rcvSeqSignal; // received seqNo
    static simsignal_t rcvAckSignal; // received ackNo (= snd_una)
};

} // namespace tcp

} // namespace inet

#endif

