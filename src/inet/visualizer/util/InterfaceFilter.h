//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFACEFILTER_H
#define __INET_INTERFACEFILTER_H

#include "inet/common/MatchableObject.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

namespace visualizer {

/**
 * This class provides a generic filter for interfaces. The filter is expressed
 * as a pattern using the cMatchExpression format.
 */
class INET_API InterfaceFilter
{
  protected:
    cMatchExpression matchExpression;

  public:
    void setPattern(const char *pattern);

    bool matches(const NetworkInterface *networkInterface) const;
};

} // namespace visualizer

} // namespace inet

#endif

