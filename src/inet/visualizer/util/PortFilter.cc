//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/PortFilter.h"

namespace inet {

namespace visualizer {

void PortFilter::setPattern(const char *pattern)
{
    matchExpression.setPattern(pattern, false, true, true);
}

bool PortFilter::matches(int value) const
{
    std::string text = std::to_string(value);
    cMatchableString matchableString(text.c_str());
    return matchExpression.matches(&matchableString);
}

} // namespace visualizer

} // namespace inet

