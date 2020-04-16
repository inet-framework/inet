//
// Copyright (C) 2006 Sam Jansen, Andras Varga,
//               2010 Zoltan Bojthe
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

#ifndef __INET_TCP_LWIP_H
#define __INET_TCP_LWIP_H

#include <map>

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "lwip/lwip_tcp.h"
#include "inet/transportlayer/tcp_lwip/LwipTcpStackIf.h"

namespace inet {

// forward declarations:
class TcpOpenCommand;

namespace tcp {

// forward declarations:
class TcpHeader;

class TcpLwipConnection;
class TcpLwipReceiveQueue;
class TcpLwipSendQueue;

/**
 * Encapsulates a Network Simulation Cradle (NSC) instance.
 */

class INET_API TcpLwip : public cSimpleModule, public LwipTcpStackIf, public LifecycleUnsupported
{
  public:
    TcpLwip();
    virtual ~TcpLwip();

  protected:
    // called by the OMNeT++ simulation kernel:

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msgP) override;
    virtual void finish() override;

    // LwipTcpStackIf functions:

    // sometime pcb is nullptr (tipically when send a RESET )
    virtual void ip_output(LwipTcpLayer::tcp_pcb *pcb,
            L3Address const& src, L3Address const& dest, void *tcpseg, int len) override;

    virtual err_t lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
            LwipTcpLayer::lwip_event event, struct pbuf *p, u16_t size, err_t err) override;

    virtual void lwip_free_pcb_event(LwipTcpLayer::tcp_pcb *pcb) override;

    virtual netif *ip_route(L3Address const& ipAddr) override;

    virtual void notifyAboutIncomingSegmentProcessing(LwipTcpLayer::tcp_pcb *pcb, uint32 seqNo,
            const void *dataptr, int len) override;

    // internal event functions:

    err_t tcp_event_accept(TcpLwipConnection& conn, LwipTcpLayer::tcp_pcb *pcb, err_t err);

    err_t tcp_event_sent(TcpLwipConnection& conn, u16_t size);

    err_t tcp_event_recv(TcpLwipConnection& conn, struct pbuf *p, err_t err);

    err_t tcp_event_conn(TcpLwipConnection& conn, err_t err);

    err_t tcp_event_poll(TcpLwipConnection& conn);

    err_t tcp_event_err(TcpLwipConnection& conn, err_t err);

    // internal utility functions:

    // find a TcpLwipConnection by connection ID
    TcpLwipConnection *findAppConn(int connIdP);

    // find a TcpLwipConnection by Lwip pcb
    TcpLwipConnection *findConnByPcb(LwipTcpLayer::tcp_pcb *pcb);

    virtual void refreshDisplay() const override;

    void removeConnection(TcpLwipConnection& conn);
    void printConnBrief(TcpLwipConnection& connP);

    void handleAppMessage(cMessage *msgP);
    void handleIpInputMessage(Packet *packet);

    // to be refined...

    void processAppCommand(TcpLwipConnection& connP, cMessage *msgP);

    // to be refined and filled in with calls into the NSC stack

    void process_OPEN_ACTIVE(TcpLwipConnection& connP, TcpOpenCommand *tcpCommandP, cMessage *msgP);
    void process_OPEN_PASSIVE(TcpLwipConnection& connP, TcpOpenCommand *tcpCommandP, cMessage *msgP);
    void process_ACCEPT(TcpLwipConnection& connP, TcpAcceptCommand *tcpCommand, cMessage *msg);
    void process_SEND(TcpLwipConnection& connP, Packet *msgP);
    void process_CLOSE(TcpLwipConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP);
    void process_ABORT(TcpLwipConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP);
    void process_STATUS(TcpLwipConnection& connP, TcpCommand *tcpCommandP, cMessage *msgP);

    // send a connection established msg to application layer
    //void sendEstablishedMsg(TcpLwipConnection& connP);

  public:
    LwipTcpLayer *getLwipTcpLayer() { return pLwipTcpLayerM; }

    /**
     * To be called from TcpLwipConnection: create a new send queue.
     */
    virtual TcpLwipSendQueue *createSendQueue();

    /**
     * To be called from TcpLwipConnection: create a new receive queue.
     */
    virtual TcpLwipReceiveQueue *createReceiveQueue();

  protected:
    typedef std::map<int, TcpLwipConnection *> TcpAppConnMap;    // connId-to-TcpLwipConnection

    // Maps:
    TcpAppConnMap tcpAppConnMapM;

    // fast timer message:
    cMessage *pLwipFastTimerM;

    // network interface:
    struct netif netIf;

  protected:
    LwipTcpLayer *pLwipTcpLayerM;
    bool isAliveM;
    Packet *pCurTcpSegM;
    TcpCrcInsertion crcInsertion;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCP_LWIP_H

