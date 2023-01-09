//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDELAYER_H
#define __INET_PACKETDELAYER_H

#include "inet/queueing/base/PacketDelayerBase.h"

namespace inet {

namespace queueing {

class INET_API PacketDelayer : public PacketDelayerBase
{
  protected:
    cPar *delayParameter = nullptr;
    cPar *bitrateParameter = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual clocktime_t computeDelay(Packet *packet) const override;
};

} // namespace queueing
} // namespace inet

#endif

