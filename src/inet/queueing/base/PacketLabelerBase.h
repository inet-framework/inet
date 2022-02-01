//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETLABELERBASE_H
#define __INET_PACKETLABELERBASE_H

#include "inet/queueing/base/PacketMarkerBase.h"
#include "inet/queueing/contract/IPacketFilterFunction.h"

namespace inet {
namespace queueing {

class INET_API PacketLabelerBase : public PacketMarkerBase
{
  protected:
    std::vector<std::string> labels;

  protected:
    virtual void initialize(int stage) override;
};

} // namespace queueing
} // namespace inet

#endif

