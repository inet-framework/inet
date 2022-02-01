//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#endif

