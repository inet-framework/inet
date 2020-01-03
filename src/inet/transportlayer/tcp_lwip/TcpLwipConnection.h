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

#ifndef __INET_TCPLWIPCONNECTION_H
#define __INET_TCPLWIPCONNECTION_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "lwip/lwip_tcp.h"

namespace inet {

// forward declarations:
class TcpConnectInfo;
class TcpStatusInfo;

namespace tcp {

// forward declarations:
class TcpLwip;
class TcpLwipReceiveQueue;
class TcpLwipSendQueue;
class INetStack;
class INetStreamSocket;

/**
 *
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

    void listen(const L3Address& localAddr, unsigned short localPort);

    void connect(const L3Address& localAddr, unsigned short localPort, const L3Address& remoteAddr, unsigned short remotePort);

    void close();

    void abort();

    void accept();

    void send(Packet *msgP);

    void fillStatusInfo(TcpStatusInfo& statusInfo);

    void notifyAboutSending(const TcpHeader& tcpsegP);

    int send_data(void *data, int len);

    void do_SEND();

    INetStreamSocket *getSocket();

    void initStats();

    bool isSendUpEnabled() { return sendUpEnabled; }

    void sendUpData();
    void recordSend(const TcpHeader& tcpsegP);
    void recordReceive(const TcpHeader& tcpsegP);

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
    static simsignal_t sndWndSignal;    // snd_wnd
    static simsignal_t sndNxtSignal;    // sent seqNo
    static simsignal_t sndAckSignal;    // sent ackNo

    static simsignal_t rcvWndSignal;    // rcv_wnd
    static simsignal_t rcvSeqSignal;    // received seqNo
    static simsignal_t rcvAckSignal;    // received ackNo (= snd_una)
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPLWIPCONNECTION_H

