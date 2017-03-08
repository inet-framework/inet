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
    double currentTime = currentAnimationPosition.getTime(targetType);
    if (currentTime >= targetTime)
        return targetAnimationSpeed;
    else {
        double lastTime = lastAnimationPosition.getTime(targetType);
        double alpha = smootherstep(lastTime, targetTime, currentTime);
        return (1 - alpha) * lastAnimationSpeed + alpha * targetAnimationSpeed;
    }
}

void AnimationSpeedInterpolator::setCurrentAnimationSpeed(double currentAnimationSpeed)
{
    lastAnimationPosition = AnimationPosition();
    lastAnimationSpeed = currentAnimationSpeed;
}

void AnimationSpeedInterpolator::setTargetAnimationSpeed(AnimationPosition::TimeType targetType, double targetTime, double targetAnimationSpeed)
{
    this->targetType = targetType;
    this->targetTime = targetTime;
    this->targetAnimationSpeed = targetAnimationSpeed;
}

void AnimationSpeedInterpolator::setAnimationSpeed(double animationSpeed)
{
    setCurrentAnimationSpeed(animationSpeed);
    setTargetAnimationSpeed(AnimationPosition::REAL_TIME, 0, animationSpeed);
}

} // namespace visualizer

} // namespace inet

