//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PORTFILTER_H
#define __INET_PORTFILTER_H

#include "inet/common/MatchableObject.h"

namespace inet {

namespace visualizer {

/**
 * This class provides a generic filter for ports. The filter is expressed
 * as a pattern using the cMatchExpression format.
 */
class INET_API PortFilter
{
  protected:
    cMatchExpression matchExpression;

  public:
    void setPattern(const char *pattern);

    bool matches(int value) const;
};

} // namespace visualizer

} // namespace inet

#endif

