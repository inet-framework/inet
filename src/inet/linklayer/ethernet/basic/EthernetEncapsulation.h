//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ETHERNETENCAPSULATION_H
#define __INET_ETHERNETENCAPSULATION_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

/**
 * Performs Ethernet II encapsulation/decapsulation. More info in the NED file.
 */
class INET_API EthernetEncapsulation : public OperationalBase, public IPassivePacketSink
{
  protected:
    std::set<const Protocol *> upperProtocols; // where to send packets after decapsulation
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    int seqNum;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    PassivePacketSinkRef lowerLayerSink;
    PassivePacketSinkRef upperLayerSink;

    // statistics
    long totalFromHigherLayer; // total number of packets received from higher layer
    long totalFromMAC; // total number of frames received from MAC
    long totalPauseSent; // total number of PAUSE frames sent
    static simsignal_t encapPkSignal;
    static simsignal_t decapPkSignal;
    static simsignal_t pauseSentSignal;

    struct Socket {
        int socketId = -1;
        int interfaceId = -1;
        MacAddress localAddress;
        MacAddress remoteAddress;
        const Protocol *protocol = nullptr;
        bool steal = false;

        Socket(int socketId) : socketId(socketId) {}
        bool matches(Packet *packet, int ifaceId, const Ptr<const EthernetMacHeader>& ethernetMacHeader);
    };

    friend std::ostream& operator<<(std::ostream& o, const Socket& t);
    std::map<int, Socket *> socketIdToSocketMap;

  protected:
    virtual ~EthernetEncapsulation();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual void processCommandFromHigherLayer(Request *msg);
    virtual void processPacketFromHigherLayer(Packet *msg);
    virtual void processPacketFromMac(Packet *packet);
    virtual void handleSendPause(cMessage *msg);
    virtual bool hasUpperProtocol(const Protocol *protocol);

    virtual void refreshDisplay() const override;

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("upperLayerIn") || gate->isName("lowerLayerIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("upperLayerIn") || gate->isName("lowerLayerIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    // for lifecycle:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void clearSockets();
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif

