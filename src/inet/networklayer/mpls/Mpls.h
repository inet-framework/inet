//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MPLS_H
#define __INET_MPLS_H

#include "inet/common/SimpleModule.h"
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/mpls/ConstType.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/mpls/MplsPacket_m.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

/**
 * Implements the MPLS protocol; see the NED file for more info.
 */
class INET_API Mpls : public SimpleModule, public IPassivePacketSink
{
  protected:
    simtime_t delay1;

    // no longer used, see comment in intialize
//    std::vector<bool> labelIf;

    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIngressClassifier> pct;

    PassivePacketSinkRef lowerLayerOutSink;
    PassivePacketSinkRef upperLayerOutSink;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

  protected:
    virtual void processPacketFromL3(Packet *msg);
    virtual void processPacketFromL2(Packet *msg);
    virtual void processMplsPacketFromL2(Packet *mplsPacket);

    virtual bool tryLabelAndForwardIpv4Datagram(Packet *ipdatagram);
    virtual void labelAndForwardIpv4Datagram(Packet *ipdatagram);

    virtual void sendToL2(Packet *msg);
    virtual void sendToL3(Packet *msg);

    void pushLabel(Packet *packet, Ptr<MplsHeader>& newMplsHeader);
    void swapLabel(Packet *packet, Ptr<MplsHeader>& newMplsHeader);
    void popLabel(Packet *packet);
    virtual void doStackOps(Packet *packet, const LabelOpVector& outLabel);

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

