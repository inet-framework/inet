//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/contract/packetlevel/ITracingObstacleLoss.h"

namespace inet {

namespace physicallayer {

simsignal_t ITracingObstacleLoss::obstaclePenetratedSignal = cComponent::registerSignal("obstaclePenetrated");

ITracingObstacleLoss::ObstaclePenetratedEvent::ObstaclePenetratedEvent(const physicalenvironment::IPhysicalObject *object, Coord intersection1, Coord intersection2, Coord normal1, Coord normal2, double loss) :
    object(object),
    intersection1(intersection1),
    intersection2(intersection2),
    normal1(normal1),
    normal2(normal2),
    loss(loss)
{
}

} // namespace physicallayer

} // namespace inet

