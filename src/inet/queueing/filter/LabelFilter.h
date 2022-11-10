//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LABELFILTER_H
#define __INET_LABELFILTER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/PatternMatcher.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {
namespace queueing {

class INET_API LabelFilter : public PacketFilterBase, public TransparentProtocolRegistrationListener
{
  protected:
    cMatchExpression labelFilter;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace queueing
} // namespace inet

#endif

