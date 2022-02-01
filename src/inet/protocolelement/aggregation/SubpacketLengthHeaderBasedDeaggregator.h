//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SUBPACKETLENGTHHEADERBASEDDEAGGREGATOR_H
#define __INET_SUBPACKETLENGTHHEADERBASEDDEAGGREGATOR_H

#include "inet/protocolelement/aggregation/base/DeaggregatorBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API SubpacketLengthHeaderBasedDeaggregator : public DeaggregatorBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual std::vector<Packet *> deaggregatePacket(Packet *packet) override;
};

} // namespace inet

#endif

