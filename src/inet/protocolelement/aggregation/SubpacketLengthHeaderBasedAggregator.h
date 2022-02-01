//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SUBPACKETLENGTHHEADERBASEDAGGREGATOR_H
#define __INET_SUBPACKETLENGTHHEADERBASEDAGGREGATOR_H

#include "inet/protocolelement/aggregation/base/AggregatorBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API SubpacketLengthHeaderBasedAggregator : public AggregatorBase
{
  protected:
    virtual void continueAggregation(Packet *packet) override;
};

} // namespace inet

#endif

