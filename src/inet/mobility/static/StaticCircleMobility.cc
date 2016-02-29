//
// Copyright (C) 2016 Kai Kientopf <kai.kientopf@uni-muenster.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#define _USE_MATH_DEFINES

#include "inet/mobility/static/StaticCircleMobility.h"
#include <cmath>
// #include <math.h>

namespace inet {

Define_Module(StaticCircleMobility);

double StaticCircleMobility::getRadius(int numHosts, double distance) {
    return std::sqrt(
            (distance * distance) / (2 * (1 - std::cos(2 * M_PI / numHosts))));
}

StaticCircleMobility::Point2D StaticCircleMobility::identifyCenter(int numHosts,
        double distance) {
    // Calculate center and test if the circle fits in the area.
    double radius = getRadius(numHosts, distance);
    Point2D center = { radius, radius };

    if (constraintAreaMin.x > -INFINITY) {
        center.x += constraintAreaMin.x;
    }
    if (constraintAreaMin.y > -INFINITY) {
        center.y += constraintAreaMin.y;
    }
    if (constraintAreaMax.x < INFINITY) {
        center.x = constraintAreaMax.x / 2;
        if (center.x < radius) {
            throw cRuntimeError(
                    "Circle don't fit in specified x-axis-area in StaticCircleMobility.");
        }
    }
    if (constraintAreaMax.y < INFINITY) {
        center.y = constraintAreaMax.y / 2;
        if (center.y < radius) {
            throw cRuntimeError(
                    "Circle don't fit in specified y-axis-area in StaticCircleMobility.");
        }
    }
    if (constraintAreaMin.x > -INFINITY) {
        center.x += constraintAreaMin.x / 2;
        if ((center.x - constraintAreaMin.x) < radius) {
            throw cRuntimeError(
                    "Circle don't fit in specified x-axis-area in StaticCircleMobility.");
        }
    }
    if (constraintAreaMin.y > -INFINITY) {
        center.y += constraintAreaMin.y / 2;
        if ((center.y - constraintAreaMin.y) < radius) {
            throw cRuntimeError(
                    "Circle don't fit in specified y-axis-area in StaticCircleMobility.");
        }
    }

    return center;
}

StaticCircleMobility::Point2D StaticCircleMobility::getPosition(int index,
        int numHosts, double distance, Point2D center) {
    Point2D position;
    double angle = std::fmod((index * (2 * M_PI / numHosts)), (M_PI / 2));
    // 0 for (+,+), 1 for (-,+), 2 for (-,-), 3 for (+,-)
    int quarter = (int) ((index * (2 * M_PI / numHosts)) / (M_PI / 2));
    double radius = getRadius(numHosts, distance);

    // Height of triangle
    double y = radius * std::sin(angle);
    // Pythagoras
    double x = std::sqrt(radius * radius - y * y);

    switch (quarter) {
    case 1: {
        position.x = x + center.x;
        position.y = y + center.y;
        break;
    }
    case 2: {
        position.x = -y + center.x;
        position.y = x + center.y;
        break;
    }
    case 3: {
        position.x = -x + center.x;
        position.y = -y + center.y;
        break;
    }
    case 0: {
        position.x = y + center.x;
        position.y = -x + center.y;
        break;
    }
    }

    return position;
}

void StaticCircleMobility::setInitialPosition() {
    int numHosts = par("numHosts");
    double distance = par("distance");
    bool clockwise = par("clockwise");
    int shift = par("shift");
    int index = (visualRepresentation->getIndex() + shift) % numHosts;

    if (!clockwise) {
        index = (numHosts - index) % numHosts;
    }

    Point2D center = identifyCenter(numHosts, distance);

    Point2D position = getPosition(index, numHosts, distance, center);

    lastPosition.x = position.x;
    lastPosition.y = position.y;
    lastPosition.z = par("initialZ");

    EV << "Position host" << index << " - x: " << lastPosition.x << ", y: "
              << lastPosition.y << ", z: " << lastPosition.z << "\n";

    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
    recordScalar("z", lastPosition.z);
}

} /* namespace inet */
