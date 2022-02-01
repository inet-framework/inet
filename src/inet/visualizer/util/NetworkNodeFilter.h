//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNODEFILTER_H
#define __INET_NETWORKNODEFILTER_H

#include "inet/common/MatchableObject.h"

namespace inet {

namespace visualizer {

/**
 * This class provides a generic filter for network nodes. The filter is expressed
 * as a pattern using the cMatchExpression format.
 */
class INET_API NetworkNodeFilter
{
  protected:
    cMatchExpression matchExpression;

  public:
    void setPattern(const char *pattern);

    bool matches(const cModule *module) const;
};

} // namespace visualizer

} // namespace inet

#endif

