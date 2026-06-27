//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// VRRPv2 (RFC 3768) ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_VRRP_H
#define __INET_VRRP_H

#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/arp/ipv4/Arp.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/vrrp/VrrpAdvertisement_m.h"

namespace inet {
namespace vrrp {

class VrrpVirtualRouter;

/**
 * Implements the VRRPv2 protocol (RFC 3768) for IPv4. A single Vrrp module owns
 * one or more VrrpVirtualRouter submodules (created dynamically from the
 * `configData` XML), demultiplexes incoming advertisements by (interface, VRID),
 * and owns the gates the virtual routers transmit through. See the NED file.
 */
class INET_API Vrrp : public RoutingProtocolBase, protected cListener
{
  protected:
    cModule *host = nullptr; // the network node that contains this module
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<Arp> arp;
    std::vector<VrrpVirtualRouter *> virtualRouters; // owned submodules

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    void loadConfig();
    VrrpVirtualRouter *createVirtualRouter(int interfaceId, int vrid, cXMLElement *group);
    VrrpVirtualRouter *findVirtualRouter(int interfaceId, int vrid);

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    Vrrp() {}
    virtual ~Vrrp() {}

    IInterfaceTable *getInterfaceTable() { return ift.get(); }
    Arp *getArp() { return arp.get(); }

    /** Called by a VrrpVirtualRouter to transmit an advertisement out the IP layer. */
    void sendAdvertisement(VrrpVirtualRouter *vr, const Ptr<VrrpAdvertisement>& adv);
};

} // namespace vrrp
} // namespace inet

#endif
