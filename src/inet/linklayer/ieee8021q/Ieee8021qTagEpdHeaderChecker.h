//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021QTAGEPDHEADERCHECKER_H
#define __INET_IEEE8021QTAGEPDHEADERCHECKER_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8021qTagEpdHeaderChecker : public PacketFilterBase
{
  protected:
    const Protocol *qtagProtocol = nullptr;
    cValueArray *vlanIdFilter = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

