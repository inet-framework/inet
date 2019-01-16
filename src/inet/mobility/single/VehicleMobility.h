//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_VEHICLEMOBILITY_H
#define __INET_VEHICLEMOBILITY_H

#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

class INET_API VehicleMobility : public MovingMobilityBase
{
  protected:
    struct Waypoint
    {
        double x;
        double y;
        double timestamp;

        Waypoint(double x, double y, double timestamp) : x(x), y(y), timestamp(timestamp) { }
    };

  protected:
    // configuration
    std::vector<Waypoint> waypoints;

    //The ground module given by the "groundModule" parameter, pointer stored for easier access.
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

#endif // ifndef __INET_VEHICLEMOBILITY_H

