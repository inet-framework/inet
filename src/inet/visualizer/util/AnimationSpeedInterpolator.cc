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

#include "inet/visualizer/util/AnimationSpeedInterpolator.h"

namespace inet {

namespace visualizer {

static double smootherstep(double edge0, double edge1, double x)
{
    x = std::max(0.0, std::min(1.0, (x - edge0) / (edge1 - edge0)));
    return x * x * x * (x * (x * 6 - 15) + 10);
}

double AnimationSpeedInterpolator::getCurrentAnimationSpeed() const
{
    AnimationPosition currentAnimationPosition;
    double currentRealTime = currentAnimationPosition.getRealTime();
    if (currentRealTime >= targetRealTime)
        return targetAnimationSpeed;
    else {
        double lastRealTime = lastAnimationPosition.getRealTime();
        double alpha = smootherstep(lastRealTime, targetRealTime, currentRealTime);
        return (1 - alpha) * lastAnimationSpeed + alpha * targetAnimationSpeed;
    }
}

void AnimationSpeedInterpolator::setCurrentAnimationSpeed(double currentAnimationSpeed)
{
    lastAnimationPosition = AnimationPosition();
    lastAnimationSpeed = currentAnimationSpeed;
}

void AnimationSpeedInterpolator::setTargetAnimationSpeed(double realTimeDelta, double targetAnimationSpeed)
{
    AnimationPosition currentAnimationPosition;
    targetRealTime = currentAnimationPosition.getRealTime() + realTimeDelta;
    this->targetAnimationSpeed = targetAnimationSpeed;
}

void AnimationSpeedInterpolator::setAnimationSpeed(double animationSpeed)
{
    setCurrentAnimationSpeed(animationSpeed);
    setTargetAnimationSpeed(0, animationSpeed);
}

} // namespace visualizer

} // namespace inet

