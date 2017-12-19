//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/visualizer/util/AnimationPosition.h"

namespace inet {

namespace visualizer {

AnimationPosition::AnimationPosition() :
    simulationTime(getSimulation()->getSimTime()),
    animationTime(getSimulation()->getEnvir()->getAnimationTime()),
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

