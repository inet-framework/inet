//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022SNAPCHECKER_H
#define __INET_IEEE8022SNAPCHECKER_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022SnapHeader_m.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8022SnapChecker : public PacketFilterBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;

    static const Protocol *getProtocol(const Ptr<const Ieee8022SnapHeader>& header);
};

} // namespace inet

#endif

