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

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "lwip/lwip_tcp.h"
#include "inet/transportlayer/tcp_lwip/LwipTcpStackIf.h"

namespace inet {

// forward declarations:
class TCPOpenCommand;
class TCPSendCommand;

namespace tcp {

// forward declarations:
class TCPSegment;

class TcpLwipConnection;
class TcpLwipReceiveQueue;
class TcpLwipSendQueue;

/**
 * Encapsulates a Network Simulation Cradle (NSC) instance.
 */

class INET_API TCP_lwIP : public cSimpleModule, public LwipTcpStackIf, public ILifecycle
{
  public:
    TCP_lwIP();
    virtual ~TCP_lwIP();

  protected:
    // called by the OMNeT++ simulation kernel:

    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msgP);
    virtual void finish();

    // LwipTcpStackIf functions:

    // sometime pcb is NULL (tipically when send a RESET )
    virtual void ip_output(LwipTcpLayer::tcp_pcb *pcb,
            L3Address const& src, L3Address const& dest, void *tcpseg, int len);

    virtual err_t lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
            LwipTcpLayer::lwip_event event, struct pbuf *p, u16_t size, err_t err);

    virtual void lwip_free_pcb_event(LwipTcpLayer::tcp_pcb *pcb);

    virtual netif *ip_route(L3Address const& ipAddr);

    virtual void notifyAboutIncomingSegmentProcessing(LwipTcpLayer::tcp_pcb *pcb, uint32 seqNo,
            const void *dataptr, int len);

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

    virtual void updateDisplayString();

    void removeConnection(TcpLwipConnection& conn);
    void printConnBrief(TcpLwipConnection& connP);

    void handleAppMessage(cMessage *msgP);
    void handleIpInputMessage(TCPSegment *tcpsegP);

    // to be refined...

    void processAppCommand(TcpLwipConnection& connP, cMessage *msgP);

    // to be refined and filled in with calls into the NSC stack

    void process_OPEN_ACTIVE(TcpLwipConnection& connP, TCPOpenCommand *tcpCommandP, cMessage *msgP);
    void process_OPEN_PASSIVE(TcpLwipConnection& connP, TCPOpenCommand *tcpCommandP, cMessage *msgP);
    void process_SEND(TcpLwipConnection& connP, TCPSendCommand *tcpCommandP, cPacket *msgP);
    void process_CLOSE(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP);
    void process_ABORT(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP);
    void process_STATUS(TcpLwipConnection& connP, TCPCommand *tcpCommandP, cMessage *msgP);

    // send a connection established msg to application layer
    void sendEstablishedMsg(TcpLwipConnection& connP);

    // ILifeCycle:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  public:
    LwipTcpLayer *getLwipTcpLayer() { return pLwipTcpLayerM; }

    /**
     * To be called from TcpLwipConnection: create a new send queue.
     */
    virtual TcpLwipSendQueue *createSendQueue(TCPDataTransferMode transferModeP);

    /**
     * To be called from TcpLwipConnection: create a new receive queue.
     */
    virtual TcpLwipReceiveQueue *createReceiveQueue(TCPDataTransferMode transferModeP);

  protected:
    typedef std::map<int, TcpLwipConnection *> TcpAppConnMap;    // connId-to-TcpLwipConnection

    // Maps:
    TcpAppConnMap tcpAppConnMapM;

    // fast timer message:
    cMessage *pLwipFastTimerM;

    // network interface:
    struct netif netIf;

  public:
    bool recordStatisticsM;    // output vectors on/off

  protected:
    LwipTcpLayer *pLwipTcpLayerM;
    bool isAliveM;
    TCPSegment *pCurTcpSegM;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCP_LWIP_H

