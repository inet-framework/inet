//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ANIMATIONSPEEDINTERPOLATOR_H
#define __INET_ANIMATIONSPEEDINTERPOLATOR_H

#include "inet/common/INETMath.h"
#include "inet/visualizer/util/AnimationPosition.h"

namespace inet {

namespace visualizer {

class INET_API AnimationSpeedInterpolator
{
  protected:
    double lastAnimationSpeed = NaN;
    AnimationPosition lastAnimationPosition;

    AnimationPosition::TimeType targetType;
    double targetTime = NaN;
    double targetAnimationSpeed = NaN;

  public:
    double getCurrentAnimationSpeed() const;
    void setCurrentAnimationSpeed(double currentAnimationSpeed);

    AnimationPosition::TimeType getTargetType() const { return targetType; }
    double getTargetTime() const { return targetTime; }

    double getTargetAnimationSpeed() const { return targetAnimationSpeed; }
    void setTargetAnimationSpeed(AnimationPosition::TimeType targetType, double targetTime, double targetAnimationSpeed);

    void setAnimationSpeed(double animationSpeed);
};

} // namespace visualizer

} // namespace inet

#endif

