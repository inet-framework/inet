//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SEGMENTROUTING_H
#define __INET_SEGMENTROUTING_H

#include <map>
#include <set>

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/ted/Ted.h"
#include "inet/routing/base/RoutingProtocolBase.h"

namespace inet {

/**
 * Segment Routing (RFC 8660) node-SID label programming; see SegmentRouting.ned
 * for the full design writeup and the sidTable XML format.
 */
class INET_API SegmentRouting : public RoutingProtocolBase, public cListener
{
  protected:
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<Ted> ted;

    int srgbBase = 0;
    int srgbSize = 0;

    // parsed from the "sidTable" XML parameter: router id -> node-SID index
    std::map<Ipv4Address, int> sidByRouter;

    Ipv4Address routerId;

    // inLabels of the LIB entries currently installed by this module (so that
    // recomputation can remove exactly its own entries and never touch
    // coexisting Ldp/RsvpTe LIB entries)
    std::set<int> ownedLabels;

    static simsignal_t sidEntriesInstalledSignal;

  public:
    SegmentRouting() {}

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // cListener method: reacts to tedChangedSignal, emitted at the host level by Ted
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // static configuration
    virtual void readSidTableFromXML(const cXMLElement *sidTable);

    // Removes every LIB entry currently owned by this module, then recomputes node-SID
    // forwarding for every OTHER router in sidByRouter from the Ted's current topology and
    // (re)installs the resulting LIB entries. Idempotent; safe to call repeatedly (on every
    // tedChangedSignal) and on startup.
    virtual void recomputeAndInstall();

    // Resolves the local outgoing interface id toward directly-connected peer router
    // `peerRouterId` (as identified by Ted::getInterfaceAddrByPeerAddress()).
    virtual int resolveOutInterfaceId(Ipv4Address peerRouterId);
};

} // namespace inet

#endif
