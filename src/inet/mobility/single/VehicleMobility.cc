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

#include <fstream>
#include <iostream>

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/mobility/single/VehicleMobility.h"

namespace inet {

using namespace physicalenvironment;

Define_Module(VehicleMobility);

void VehicleMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    EV_TRACE << "initializing VehicleMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speed = par("speed");
        waypointProximity = par("waypointProximity");
        targetPointIndex = 0;
        heading = 0;
        angularSpeed = 0;
        readWaypointsFromFile(par("waypointFile"));
        ground = findModuleFromPar<IGround>(par("groundModule"), this);
    }
}

void VehicleMobility::setInitialPosition()
{
    lastPosition.x = waypoints[targetPointIndex].x;
    lastPosition.y = waypoints[targetPointIndex].y;
    lastVelocity.x = speed * cos(M_PI * heading / 180);
    lastVelocity.y = speed * sin(M_PI * heading / 180);

    if (ground) {
        lastPosition = ground->computeGroundProjection(lastPosition);
        lastVelocity = ground->computeGroundProjection(lastPosition + lastVelocity) - lastPosition;
    }
}

void VehicleMobility::readWaypointsFromFile(const char *fileName)
{
    auto coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this, false);
    char line[256];
    std::ifstream inputFile(fileName);
    while (true) {
        inputFile.getline(line, 256);
        if (!inputFile.fail()) {
            cStringTokenizer tokenizer(line, ",");
            double value1 = atof(tokenizer.nextToken());
            double value2 = atof(tokenizer.nextToken());
            double value3 = atof(tokenizer.nextToken());
            double x;
            double y;
            double z;
            if (coordinateSystem == nullptr) {
                x = value1;
                y = value2;
                z = value3;
            }
            else {
                Coord sceneCoordinate = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(value1), deg(value2), m(value3)));
                x = sceneCoordinate.x;
                y = sceneCoordinate.y;
                z = sceneCoordinate.z;
            }
            waypoints.push_back(Waypoint(x, y, z));
        }
        else
            break;
    }
}

void VehicleMobility::move()
{
    Waypoint target = waypoints[targetPointIndex];
    double dx = target.x - lastPosition.x;
    double dy = target.y - lastPosition.y;
    if (dx * dx + dy * dy < waypointProximity * waypointProximity)  // reached so change to next (within the predefined proximity of the waypoint)
        targetPointIndex = (targetPointIndex + 1) % waypoints.size();
    double targetDirection = atan2(dy, dx) / M_PI * 180;
    double diff = targetDirection - heading;
    while (diff < -180)
        diff += 360;
    while (diff > 180)
        diff -= 360;
    angularSpeed = diff * 5;
    double timeStep = (simTime() - lastUpdate).dbl();
    heading += angularSpeed * timeStep;

    Coord tempSpeed = Coord(cos(M_PI * heading / 180), sin(M_PI * heading / 180)) * speed;
    Coord tempPosition = lastPosition + tempSpeed * timeStep;

    if (ground)
        tempPosition = ground->computeGroundProjection(tempPosition);

    lastVelocity = tempPosition - lastPosition;
    lastPosition = tempPosition;
}

void VehicleMobility::orient()
{
    if (ground) {
        Coord groundNormal = ground->computeGroundNormal(lastPosition);

        // this will make the wheels follow the ground
        Quaternion quat = Quaternion::rotationFromTo(Coord(0, 0, 1), groundNormal);

        Coord groundTangent = groundNormal % lastVelocity;
        groundTangent.normalize();
        Coord direction = groundTangent % groundNormal;
        direction.normalize(); // this is lastSpeed, normalized and adjusted to be perpendicular to groundNormal

        // our model looks in this direction if we only rotate the Z axis to match the ground normal
        Coord groundX = quat.rotate(Coord(1, 0, 0));

        double dp = groundX * direction;

        double angle;

        if (((groundX % direction) * groundNormal) > 0)
            angle = std::acos(dp);
        else
            // correcting for the case where the angle should be over 90 degrees (or under -90):
            angle = 2*M_PI - std::acos(dp);

        // and finally rotating around the now-ground-orthogonal local Z
        quat *= Quaternion(Coord(0, 0, 1), angle);

        lastOrientation = quat;
    }
    else
        MovingMobilityBase::orient();
}

} // namespace inet

