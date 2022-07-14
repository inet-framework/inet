//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4SOCKETTABLE_H
#define __INET_IPV4SOCKETTABLE_H

#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

class INET_API Ipv4SocketTable : public OperationalBase
{
  public:
    struct Socket {
        int socketId = -1;
        Ipv4Address localAddress;
        Ipv4Address remoteAddress;
        int protocolId = -1;
        bool steal = false;

        Socket(int socketId) : socketId(socketId) {}

        friend std::ostream& operator<<(std::ostream& o, const Socket& t);
    };

  protected:
    std::map<int, Socket *> socketIdToSocketMap;

  protected:
    virtual void initialize(int stage) override;

    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("Incoming message not allowed"); }
    virtual void handleMessageWhenUp(cMessage *msg) override { throw cRuntimeError("Incoming message not allowed"); }

    virtual void clearSockets();

    /**
     * ILifecycle methods
     */
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override { clearSockets(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { clearSockets(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override { clearSockets(); }

  public:
    virtual ~Ipv4SocketTable();

    virtual void addSocket(int socketId, int protocolId, Ipv4Address localAddress);
    virtual bool connectSocket(int socketId, Ipv4Address remoteAddress);
    virtual bool removeSocket(int socketId);
    virtual std::vector<Socket *> findSockets(Ipv4Address localAddress, Ipv4Address remoteAddress, int protocolId) const;
};

} // namespace inet

#endif
