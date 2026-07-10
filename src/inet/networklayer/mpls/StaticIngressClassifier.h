//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_STATICINGRESSCLASSIFIER_H
#define __INET_STATICINGRESSCLASSIFIER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/SimpleModule.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"

namespace inet {

/**
 * Signaling-free, statically configured ingress classifier for MPLS; see
 * StaticIngressClassifier.ned for the XML config format. Together with
 * ~MplsRouterBase and static ~LibTable XML configuration, this module lets an
 * LSP network be built with no signaling protocol (~Ldp/~RsvpTe) at all --
 * every LSR's forwarding decision (ingress push, transit swap/pop) comes
 * entirely from configuration loaded at startup.
 */
class INET_API StaticIngressClassifier : public SimpleModule, public IIngressClassifier
{
  public:
    struct FecEntry {
        Ipv4Address dest;
        Ipv4Address netmask;
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
