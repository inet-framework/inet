//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

void NetworkNodeFilter::setPattern(const char *pattern)
{
    matchExpression.setPattern(pattern, false, true, true);
}

bool NetworkNodeFilter::matches(const cModule *module) const
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLNAME, module);
    return matchExpression.matches(&matchableObject);
}

} // namespace visualizer

} // namespace inet

