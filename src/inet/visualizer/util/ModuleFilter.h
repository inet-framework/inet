//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEFILTER_H
#define __INET_MODULEFILTER_H

#include "inet/common/MatchableObject.h"

namespace inet {

namespace visualizer {

/**
 * This class provides a generic filter for modules. The filter is expressed
 * as a pattern using the cMatchExpression format.
 */
class INET_API ModuleFilter
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

