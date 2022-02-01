//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021QTAGTPIDHEADERCHECKER_H
#define __INET_IEEE8021QTAGTPIDHEADERCHECKER_H

#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8021qTagTpidHeaderChecker : public PacketFilterBase
{
  protected:
    int tpid = -1;
    cValueArray *vlanIdFilter = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

