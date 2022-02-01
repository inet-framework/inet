//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/ground/FlatGround.h"

namespace inet {

namespace physicalenvironment {

Define_Module(FlatGround);

void FlatGround::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        elevation = par("elevation");
}

Coord FlatGround::computeGroundProjection(const Coord& position) const
{
    return Coord(position.x, position.y, elevation);
}

Coord FlatGround::computeGroundNormal(const Coord& position) const
{
    return Coord(0, 0, 1);
}

} // namespace physicalenvironment

} // namespace inet

