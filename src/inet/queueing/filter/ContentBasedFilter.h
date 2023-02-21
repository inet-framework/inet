//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONTENTBASEDFILTER_H
#define __INET_CONTENTBASEDFILTER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {
namespace queueing {

class INET_API ContentBasedFilter : public PacketFilterBase, public TransparentProtocolRegistrationListener
{
  protected:
    PacketFilter filter;

  protected:
    virtual void initialize(int stage) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace queueing
} // namespace inet

#endif

