//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VEHICLEMOBILITY_H
#define __INET_VEHICLEMOBILITY_H

#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

class INET_API VehicleMobility : public MovingMobilityBase
{
  protected:
    struct Waypoint {
        double x;
        double y;
        double timestamp;

        Waypoint(double x, double y, double timestamp) : x(x), y(y), timestamp(timestamp) {}
    };

  protected:
    // configuration
    std::vector<Waypoint> waypoints;

    // The ground module given by the "groundModule" parameter, pointer stored for easier access.
    physicalenvironment::IGround *ground = nullptr;

    double speed;
    double heading;
    double waypointProximity;
    double angularSpeed;
    int targetPointIndex;

  protected:
    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void move() override;
    virtual void orient() override;

    virtual void readWaypointsFromFile(const char *fileName);

  public:
    virtual double getMaxSpeed() const override { return speed; }
};

} // namespace inet

#endif

