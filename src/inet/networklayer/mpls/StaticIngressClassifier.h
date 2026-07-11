//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_STATICINGRESSCLASSIFIER_H
#define __INET_STATICINGRESSCLASSIFIER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/SimpleModule.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"

namespace inet {

/**
 * Signaling-free, statically configured ingress classifier for MPLS; see
 * StaticIngressClassifier.ned for the XML config format. Together with
 * ~MplsRouterBase and static ~LibTable XML configuration, this module lets an
 * LSP network be built with no signaling protocol (~Ldp/~RsvpTe) at all --
 * every LSR's forwarding decision (ingress push, transit swap/pop) comes
 * entirely from configuration loaded at startup.
 *
 * Dual-stack (Workstream F3): `dest` is a generic ~L3Address (IPv4 or IPv6,
 * auto-detected from the XML `dest` string), matched against the packet's
 * native destination address via ~L3Address::matches(), keyed off an integer
 * `prefixLength` -- an IPv4 fecentry may still express this as a dotted
 * `netmask=` attribute for backward compatibility (converted to a prefix
 * length at load time); an IPv6 `dest` requires the new `prefixLength=`
 * attribute instead, since IPv6 has no netmask-string convention.
 */
class INET_API StaticIngressClassifier : public SimpleModule, public IIngressClassifier
{
  public:
    struct FecEntry {
        L3Address dest;
        int prefixLength;
        int label;
        int outInterfaceId;
    };

  protected:
    std::vector<FecEntry> fecTable;
    ModuleRefByPar<IInterfaceTable> ift;

  public:
    StaticIngressClassifier() {}

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    // IIngressClassifier implementation (method invoked by MPLS): longest-prefix-match
    // lookup over the static FEC table, producing a PUSH label op and the resolved
    // outgoing interface.
    virtual bool lookupLabel(Packet *packet, LabelOpVector& outLabel, int& outInterfaceId) override;

  protected:
    // static configuration
    virtual void readTableFromXML(const cXMLElement *fectable);
};

} // namespace inet

#endif
