//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/ModuleFilter.h"

namespace inet {

namespace visualizer {

void ModuleFilter::setPattern(const char *pattern)
{
    matchExpression.setPattern(pattern, true, true, true);
}

bool ModuleFilter::matches(const cModule *module) const
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLPATH, module);
    return matchExpression.matches(&matchableObject);
}

} // namespace visualizer

} // namespace inet

