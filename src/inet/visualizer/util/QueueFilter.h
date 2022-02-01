//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUEUEFILTER_H
#define __INET_QUEUEFILTER_H

#include "inet/common/MatchableObject.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {

namespace visualizer {

/**
 * This class provides a generic filter for queues. The filter is expressed
 * as a pattern using the cMatchExpression format.
 */
class INET_API QueueFilter
{
  protected:
    cMatchExpression matchExpression;

  public:
    void setPattern(const char *pattern);

    bool matches(const queueing::IPacketQueue *queue) const;
};

} // namespace visualizer

} // namespace inet

#endif

