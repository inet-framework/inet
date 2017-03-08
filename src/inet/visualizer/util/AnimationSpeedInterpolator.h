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

#endif // ifndef __INET_ANIMATIONSPEEDINTERPOLATOR_H

