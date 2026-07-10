//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LDPCLASSIFIER_H
#define __INET_LDPCLASSIFIER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/SimpleModule.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"

namespace inet {

class Ldp;

/**
 * Ingress classifier for LDP-signaled label-switched paths; see LdpClassifier.ned
 * for the fallback-design rationale (delegates to Ldp::classifyPacket() rather
 * than maintaining its own bind-time FEC/label table).
 */
class INET_API LdpClassifier : public SimpleModule, public IIngressClassifier
{
  protected:
    ModuleRefByPar<Ldp> ldp;

  public:
    LdpClassifier() {}

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    // IIngressClassifier implementation (method invoked by MPLS); delegates as-is to Ldp
    virtual bool lookupLabel(Packet *packet, LabelOpVector& outLabel, int& outInterfaceId) override;
};

} // namespace inet

#endif
