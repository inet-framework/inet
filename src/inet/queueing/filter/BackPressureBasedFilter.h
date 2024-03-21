//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BACKPRESSUREBASEDFILTER_H
#define __INET_BACKPRESSUREBASEDFILTER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFilterBase.h"

using namespace omnetpp;

namespace inet {
namespace queueing {

class BackPressureBasedFilter : public PacketFilterBase, public TransparentProtocolRegistrationListener
{
  protected:
    virtual bool matchesPacket(const Packet *packet) const override;
    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} //namespace queueing
} //namespace inet
#endif
