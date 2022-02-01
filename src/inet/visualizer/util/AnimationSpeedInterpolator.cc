//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

