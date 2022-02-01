//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONTENTBASEDLABELER_H
#define __INET_CONTENTBASEDLABELER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketLabelerBase.h"

namespace inet {
namespace queueing {

class INET_API ContentBasedLabeler : public PacketLabelerBase
{
  protected:
    std::vector<PacketFilter *> filters;

  protected:
    virtual void initialize(int stage) override;
    virtual void markPacket(Packet *packet) override;

  public:
    virtual ~ContentBasedLabeler();
};

} // namespace queueing
} // namespace inet

#endif

