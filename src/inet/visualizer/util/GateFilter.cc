//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/GateFilter.h"

namespace inet {

namespace visualizer {

void GateFilter::setPattern(const char *pattern)
{
    matchExpression.setPattern(pattern, true, true, true);
}

bool GateFilter::matches(const queueing::IPacketGate *gate) const
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLPATH, check_and_cast<const cObject *>(gate));
    return matchExpression.matches(&matchableObject);
}

} // namespace visualizer

} // namespace inet

