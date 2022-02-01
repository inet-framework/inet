//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETLABELER_H
#define __INET_PACKETLABELER_H

#include "inet/queueing/base/PacketLabelerBase.h"
#include "inet/queueing/contract/IPacketFilterFunction.h"

namespace inet {
namespace queueing {

class INET_API PacketLabeler : public PacketLabelerBase
{
  protected:
    std::vector<IPacketFilterFunction *> filters;

  protected:
    virtual void initialize(int stage) override;
    virtual IPacketFilterFunction *createFilterFunction(const char *filterClass) const;
    virtual void markPacket(Packet *packet) override;

  public:
    virtual ~PacketLabeler();
};

} // namespace queueing
} // namespace inet

#endif

