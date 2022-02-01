//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ECNMARKER_H
#define __INET_ECNMARKER_H

#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/queueing/base/PacketMarkerBase.h"

namespace inet {
namespace queueing {

class INET_API EcnMarker : public PacketMarkerBase
{
  protected:
    virtual void markPacket(Packet *packet) override;

  public:
    static void setEcn(Packet *packet, IpEcnCode ecn);
    static IpEcnCode getEcn(const Packet *packet);
};

} // namespace queueing
} // namespace inet

#endif

