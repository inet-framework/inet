//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/InterfaceFilter.h"

namespace inet {

namespace visualizer {

void InterfaceFilter::setPattern(const char *pattern)
{
    matchExpression.setPattern(pattern, false, true, true);
}

bool InterfaceFilter::matches(const NetworkInterface *networkInterface) const
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLNAME, networkInterface);
    return matchExpression.matches(&matchableObject);
}

} // namespace visualizer

} // namespace inet

