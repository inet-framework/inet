//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/QueueFilter.h"

namespace inet {

namespace visualizer {

void QueueFilter::setPattern(const char *pattern)
{
    matchExpression.setPattern(pattern, true, true, true);
}

bool QueueFilter::matches(const queueing::IPacketQueue *queue) const
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLPATH, check_and_cast<const cObject *>(queue));
    return matchExpression.matches(&matchableObject);
}

} // namespace visualizer

} // namespace inet

