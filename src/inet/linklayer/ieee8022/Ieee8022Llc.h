//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022LLC_H
#define __INET_IEEE8022LLC_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022SnapHeader_m.h"

namespace inet {

class INET_API Ieee8022Llc : public OperationalBase, public DefaultProtocolRegistrationListener
{
  protected:
    struct SocketDescriptor {
        int socketId = -1;
        int localSap = -1;
        int remoteSap = -1;

        SocketDescriptor(int socketId, int localSap, int remoteSap = -1)
            : socketId(socketId), localSap(localSap), remoteSap(remoteSap) {}
    };

    friend std::ostream& operator<<(std::ostream& o, const SocketDescriptor& t);

    std::set<const Protocol *> upperProtocols; // where to send packets after decapsulation
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

  protected:
    void clearSockets();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual void encapsulate(Packet *frame);
    virtual void decapsulate(Packet *frame);
    virtual void processPacketFromHigherLayer(Packet *msg);
    virtual bool deliverCopyToSockets(Packet *packet);  // return true when delivered to any socket
    virtual bool isDeliverableToUpperLayer(Packet *packet);
    virtual void processPacketFromMac(Packet *packet);
    virtual void processCommandFromHigherLayer(Request *request);

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

    // for lifecycle:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    virtual ~Ieee8022Llc();
    static const Protocol *getProtocol(const Ptr<const Ieee8022LlcHeader>& header);
};

} // namespace inet

#endif

