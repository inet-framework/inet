//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GATEFILTER_H
#define __INET_GATEFILTER_H

#include "inet/common/MatchableObject.h"
#include "inet/queueing/contract/IPacketGate.h"

namespace inet {

namespace visualizer {

/**
 * This class provides a generic filter for gates. The filter is expressed
 * as a pattern using the cMatchExpression format.
 */
class INET_API GateFilter
{
  protected:
    cMatchExpression matchExpression;

  public:
    void setPattern(const char *pattern);

    bool matches(const queueing::IPacketGate *gate) const;
};

} // namespace visualizer

} // namespace inet

#endif

