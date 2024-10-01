//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BGP_H
#define __INET_BGP_H

#include "inet/common/Protocol.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/routing/bgpv4/BgpRouter.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

namespace bgp {

class INET_API Bgp : public RoutingProtocolBase, protected cListener, public IPassivePacketSink, public IModuleInterfaceLookup
{
  private:
    ModuleRefByPar<IRoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    const Protocol *networkProtocol = &Protocol::ipv4; // address family this BGP instance serves (set from the addressFamily parameter)
    BgpRouter *bgpRouter = nullptr; // data structure to fill in
    cMessage *startupTimer = nullptr; // timer for delayed startup
    cMessage *shutdownTimer = nullptr; // timer for graceful (delayed) shutdown

  public:
    Bgp();
    virtual ~Bgp();
    bool isIpv6() const { return networkProtocol == &Protocol::ipv6; }

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    void startBgp();
    void stopBgp(bool abort);
    void removeBgpRoutes();
    void createBgpRouter();
    void handleTimer(cMessage *timer);
    virtual void finish() override;
};

} // namespace bgp

} // namespace inet

#endif
