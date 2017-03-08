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

#ifndef __INET_ANIMATIONPOSITION_H
#define __INET_ANIMATIONPOSITION_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace visualizer {

class INET_API AnimationPosition
{
  public:
    enum TimeType {
        SIMULATION_TIME,
        ANIMATION_TIME,
        REAL_TIME
    };

  protected:
    simtime_t simulationTime;
    double animationTime;
    double realTime;

  protected:
    double computeRealTime() const;

  public:
    AnimationPosition();
    AnimationPosition(simtime_t simulationTime, double animationTime, double realTime);

    simtime_t getSimulationTime() const { return simulationTime; }
    double getAnimationTime() const { return animationTime; }
    double getRealTime() const { return realTime; }

    double getTime(TimeType type) const;

    AnimationPosition& operator=(const AnimationPosition& other);
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ANIMATIONPOSITION_H

