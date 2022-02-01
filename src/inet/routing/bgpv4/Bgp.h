//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BGP_H
#define __INET_BGP_H

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/routing/bgpv4/BgpRouter.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"

namespace inet {

namespace bgp {

class INET_API Bgp : public cSimpleModule, protected cListener, public LifecycleUnsupported
{
  private:
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    bool isUp = false;
    BgpRouter *bgpRouter = nullptr; // data structure to fill in
    cMessage *startupTimer = nullptr; // timer for delayed startup

  public:
    Bgp();
    virtual ~Bgp();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void createBgpRouter();
    void handleTimer(cMessage *timer);
    virtual void finish() override;
};

} // namespace bgp

} // namespace inet

#endif

