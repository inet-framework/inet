//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPGENERICSERVERAPP_H
#define __INET_TCPGENERICSERVERAPP_H

#include "inet/common/SimpleModule.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/transportlayer/contract/ITcp.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

/**
 * Generic server application. It serves requests whose control is carried in a
 * GenericAppMsgReq region tag on the received data. Clients are usually
 * subclassed from TcpAppBase.
 *
 * @see GenericAppMsgReq, TcpAppBase
 */
class INET_API TcpGenericServerApp : public SimpleModule, public LifecycleUnsupported, public TcpSocket::ICallback, public queueing::IPassivePacketSink, public IModuleInterfaceLookup
{
  protected:
    TcpSocket socket;
    SocketMap socketMap; // per-connection sockets, receivers of TCP indications
    queueing::PassivePacketSinkRef socketOutSink; // tcp request sink for reply packets
    ModuleRefByGate<ITcp> tcp; // for close/read commands
    simtime_t delay;
    simtime_t maxMsgDelay;
    bool autoRead = true;

    long msgsRcvd;
    long msgsSent;
    long bytesRcvd;
    long bytesSent;

    std::map<int, ChunkQueue> socketQueue;


  public:
    virtual ~TcpGenericServerApp();

  protected:
    virtual void sendBack(cMessage *msg);
    virtual void sendOrSchedule(cMessage *msg, simtime_t delay);
    virtual void sendOrScheduleReadCommandIfNeeded(int connId);

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    // TcpSocket::ICallback: the listening socket forks per-connection sockets,
    // which deliver TCP indications to the legacy message-style processing
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketEstablished(TcpSocket *socket, Indication *indication) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override {}
    virtual void socketFailure(TcpSocket *socket, int code) override {}
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;
};

} // namespace inet

#endif

