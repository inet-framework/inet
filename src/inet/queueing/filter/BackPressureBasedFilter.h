//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BACKPRESSUREBASEDFILTER_H
#define __INET_BACKPRESSUREBASEDFILTER_H

#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {
namespace queueing {

class BackPressureBasedFilter : public PacketFilterBase
{
  protected:
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }

    virtual bool matchesPacket(const Packet *packet) const override;

};

} //namespace queueing
} //namespace inet

#endif
