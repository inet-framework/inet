//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMFILTER_H
#define __INET_STREAMFILTER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/PatternMatcher.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API StreamFilter : public PacketFilterBase, public TransparentProtocolRegistrationListener
{
  protected:
    const char *mode = nullptr;
    cMatchExpression streamNameFilter;

  protected:
    virtual void initialize(int stage) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

