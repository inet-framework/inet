//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MESSAGEAUTHENTICATIONCODECHECKER_H
#define __INET_MESSAGEAUTHENTICATIONCODECHECKER_H

#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API MessageAuthenticationCodeChecker : public PacketFilterBase
{
  protected:
    b headerLength = b(-1);

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

