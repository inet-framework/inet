//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SRPOLICY_H
#define __INET_SRPOLICY_H

#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/SimpleModule.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/networklayer/srmpls/SegmentRouting.h"

namespace inet {

/**
 * SR-policy ingress classifier: imposes an explicit segment-list label stack
 * (node-SIDs and/or adjacency-SIDs) on matching traffic; see SrPolicy.ned for
 * the full design writeup, the policies XML format, and the segment-list
 * canonicalization this module implements.
 */
class INET_API SrPolicy : public SimpleModule, public IIngressClassifier
{
  public:
    struct Segment {
        enum Kind { NODE, ADJ } kind = NODE;

        // NODE: the label to push, resolved ONCE at config time as
        // segmentRouting->getSrgbBase() + sidIndex (homogeneous SRGB, so this
        // never changes at runtime); `nodeRouter` is the router this sid
        // belongs to (also resolved once at config time), used only for the
        // per-packet next-hop/reachability computation, never for the label
        // value itself.
        int label = -1;
        Ipv4Address nodeRouter;

        // ADJ: the LOCAL interface id this segment crosses, resolved once at
        // config time from the segment's interface-name token. The label
        // itself is NOT cached here -- unlike node-SID labels, adjacency-SID
        // labels can appear/disappear at runtime (see SegmentRouting::
        // getAdjSidLabel()), so it is looked up fresh on every packet.
        int interfaceId = -1;
    };

    struct PolicyEntry {
        Ipv4Address dest;
        Ipv4Address netmask;
        std::vector<Segment> segments; // wire order: segments[0] = outermost/first-traversed
    };

  protected:
    std::vector<PolicyEntry> policyTable;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<SegmentRouting> sr;

  public:
    SrPolicy() {}

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    // IIngressClassifier implementation (method invoked by MPLS): longest-prefix-match lookup
    // over the policy table, producing a PUSH stack (see resolveSegmentList()) and the resolved
    // outgoing interface.
    virtual bool lookupLabel(Packet *packet, LabelOpVector& outLabel, int& outInterfaceId) override;

  protected:
    // static configuration
    virtual void readPoliciesFromXML(const cXMLElement *policiesXml);

    // Resolves a matched policy's segment list into a PUSH label stack and the outgoing
    // interface, applying the first-segment PHP/adjacency canonicalization documented in
    // SrPolicy.ned. Returns false if the policy currently cannot be realized at all (an
    // unreachable node-SID target, a withdrawn adjacency SID, or the whole stack collapsing to
    // nothing -- see .ned doc's Limitations section for why this maps to "no mapping" rather
    // than a drop).
    virtual bool resolveSegmentList(const std::vector<Segment>& segments, LabelOpVector& outLabel, int& outInterfaceId);
};

std::ostream& operator<<(std::ostream& os, const SrPolicy::PolicyEntry& policy);

} // namespace inet

#endif
