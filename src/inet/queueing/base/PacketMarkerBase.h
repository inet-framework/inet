//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETMARKERBASE_H
#define __INET_PACKETMARKERBASE_H

#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/queueing/contract/IPacketMarker.h"

namespace inet {
namespace queueing {

class INET_API PacketMarkerBase : public PacketFlowBase, public virtual IPacketMarker
{
  protected:
    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

