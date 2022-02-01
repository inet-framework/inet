//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETFILTER_H
#define __INET_PACKETFILTER_H

#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/queueing/contract/IPacketFilterFunction.h"

namespace inet {
namespace queueing {

class INET_API PacketFilter : public PacketFilterBase
{
  protected:
    IPacketFilterFunction *packetFilterFunction = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual IPacketFilterFunction *createFilterFunction(const char *filterClass) const;
    virtual bool matchesPacket(const Packet *packet) const override;

  public:
    virtual ~PacketFilter() { delete packetFilterFunction; }
};

} // namespace queueing
} // namespace inet

#endif

