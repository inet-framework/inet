//
// Copyright (C) 2006 Sam Jansen, Andras Varga,
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPLWIP_H
#define __INET_TCPLWIP_H

#include <map>

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"
#include "inet/transportlayer/tcp_lwip/LwipTcpStackIf.h"
#include "inet/transportlayer/tcp_lwip/TcpLwipConnection.h"
#include "inet/transportlayer/tcp_lwip/queues/TcpLwipQueues.h"
#include "lwip/lwip_tcp.h"

namespace inet {
namespace tcp {

/**
 * Module for using the LwIP TCP stack.
 */
class INET_API TcpLwip : public cSimpleModule, public LwipTcpStackIf, public LifecycleUnsupported
{
  public:
    TcpLwip();
    virtual ~TcpLwip();

  protected:
    /** @name cSimpleModule redefinitions */
    //@{
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msgP) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;
    //@}

    /** @name LwipTcpStackIf functions */
    //@{
    virtual void ip_output(LwipTcpLayer::tcp_pcb *pcb,
            L3Address const& src, L3Address const& dest, void *tcpseg, int len) override;
    // sometime the pcb is nullptr (tipically when send a RESET )

    virtual err_t lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
            LwipTcpLayer::lwip_event event, struct pbuf *p, u16_t size, err_t err) override;

    virtual void lwip_free_pcb_event(LwipTcpLayer::tcp_pcb *pcb) override;

    virtual netif *ip_route(L3Address const& ipAddr) override;

    virtual void notifyAboutIncomingSegmentProcessing(LwipTcpLayer::tcp_pcb *pcb, uint32_t seqNo,
            const void *dataptr, int len) override;
    //@}

    /** @name internal event functions */
    //@{
    err_t tcp_event_accept(TcpLwipConnection& conn, LwipTcpLayer::tcp_pcb *pcb, err_t err);
    err_t tcp_event_sent(TcpLwipConnection& conn, u16_t size);
    err_t tcp_event_recv(TcpLwipConnection& conn, struct pbuf *p, err_t err);
    err_t tcp_event_conn(TcpLwipConnection& conn, err_t err);
    err_t tcp_event_poll(TcpLwipConnection& conn);
    err_t tcp_event_err(TcpLwipConnection& conn, err_t err);
    //@}

    /** @name internal utility functions */
    //@{

    // find a TcpLwipConnection by connection ID
    TcpLwipConnection *findAppConn(int connIdP);

    // find a TcpLwipConnection by Lwip pcb
    TcpLwipConnection *findConnByPcb(LwipTcpLayer::tcp_pcb *pcb);

    void removeConnection(TcpLwipConnection& conn);
    void printConnBrief(TcpLwipConnection& connP);

    void handleUpperCommand(cMessage *msgP);
    void handleLowerPacket(Packet *packet);
    //@}

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
    typedef std::map<int, TcpLwipConnection *> TcpAppConnMap; // connId-to-TcpLwipConnection

    // Maps:
    TcpAppConnMap tcpAppConnMapM;

    // fast timer message:
    cMessage *pLwipFastTimerM = nullptr;

    // network interface:
    struct netif netIf;

  protected:
    LwipTcpLayer *pLwipTcpLayerM = nullptr;
    bool isAliveM = false;
    Packet *pCurTcpSegM = nullptr;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
};

} // namespace tcp
} // namespace inet

#endif

