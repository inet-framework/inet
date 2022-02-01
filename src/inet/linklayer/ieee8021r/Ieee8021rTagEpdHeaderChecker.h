//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021RTAGEPDHEADERCHECKER_H
#define __INET_IEEE8021RTAGEPDHEADERCHECKER_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8021rTagEpdHeaderChecker : public PacketFilterBase
{
  protected:

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

