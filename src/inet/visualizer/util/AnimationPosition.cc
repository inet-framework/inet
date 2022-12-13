//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/AnimationPosition.h"

namespace inet {

namespace visualizer {

AnimationPosition::AnimationPosition() :
    simulationTime(cSimulation::getActiveSimulation()->getSimTime()),
    animationTime(cSimulation::getActiveEnvir()->getAnimationTime()),
    realTime(computeRealTime())
{
}

AnimationPosition::AnimationPosition(simtime_t simulationTime, double animationTime, double realTime) :
    simulationTime(simulationTime),
    animationTime(animationTime),
    realTime(realTime)
{
}

double AnimationPosition::getTime(TimeType type) const
{
    switch (type) {
        case SIMULATION_TIME:
            return simulationTime.dbl();
        case ANIMATION_TIME:
            return animationTime;
        case REAL_TIME:
            return realTime;
        default:
            throw cRuntimeError("Unknown time type");
    }
}

double AnimationPosition::computeRealTime() const
{
    return opp_get_monotonic_clock_usecs() / 1.0E+6;
}

AnimationPosition& AnimationPosition::operator=(const AnimationPosition& other)
{
    simulationTime = other.simulationTime;
    animationTime = other.animationTime;
    realTime = other.realTime;
    return *this;
}

} // namespace visualizer

} // namespace inet

