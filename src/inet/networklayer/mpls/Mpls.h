//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MPLS_H
#define __INET_MPLS_H

#include <vector>

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/mpls/ConstType.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/mpls/MplsPacket_m.h"

namespace inet {

/**
 * Implements the MPLS protocol; see the NED file for more info.
 */
class INET_API Mpls : public cSimpleModule, public DefaultProtocolRegistrationListener, public IInterfaceRegistrationListener
{
  protected:
    simtime_t delay1;

    // no longer used, see comment in intialize
//    std::vector<bool> labelIf;

    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIngressClassifier> pct;

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

    // IInterfaceRegistrationListener:
    virtual void handleRegisterInterface(const NetworkInterface& interface, cGate *in, cGate *out) override;

    // IProtocolRegistrationListener:
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
};

} // namespace inet

#endif

