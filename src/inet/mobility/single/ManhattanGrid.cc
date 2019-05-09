//
// Copyright (C) 2015 OpenSim Ltd.
// Copyright (C) 2019 Alfonso Ariza, universidad de Malaga.
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
#include "inet/mobility/single/ManhattanGrid.h"
#include  <cmath>

namespace inet {

using namespace physicalenvironment;

Define_Module(ManhattanGrid);

void ManhattanGrid::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    EV_TRACE << "initializing VehicleMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        waitTimeParameter = &par("waitTime");
        hasWaitTime = waitTimeParameter->isExpression() || waitTimeParameter->doubleValue() != 0;
        speedParameter = &par("speed");
        stationary = !speedParameter->isExpression() && speedParameter->doubleValue() == 0;
        double disX = constraintAreaMax.x - constraintAreaMin.x;
        double disY = constraintAreaMax.y - constraintAreaMin.y;
        int numInterX = par("NumXstreets");
        int numInterY = par("NumYstreets");
        double sepX = disX / (double)(numInterX+1);
        double sepY = disY / (double)(numInterY+1);
        for (int i = 0;i <= numInterX+1; i++) {
            for (int j = 0; j <= numInterY+1; j++) {
                double coordX = constraintAreaMin.x+(i*sepX);
                double coordY = constraintAreaMin.y+(j*sepY);
                Coord point(coordX,coordY);
                auto xlist = intersectionX.find(coordX);
                auto ylist = intersectionY.find(coordY);

                if (xlist != intersectionX.end())
                    xlist->second.push_back(point);
                else
                    intersectionX[coordX].push_back(point);

                if (ylist != intersectionY.end())
                    ylist->second.push_back(point);
                else
                    intersectionY[coordY].push_back(point);
            }
        }
    }
}

void ManhattanGrid::setInitialPosition()
{
    // move the position to the close street
    double closeX = 1000000000;
    double closeY = 1000000000;
    double distX = 1000000000;
    double distY = 1000000000;

    MovingMobilityBase::setInitialPosition();

    for (const auto &elem : intersectionX) {
        if (std::abs(lastPosition.x - elem.first) < distY) {
            distX = std::abs(lastPosition.x - elem.first);
            closeX = elem.first;
        }
    }

    for (auto elem : intersectionY) {
        if (std::abs(lastPosition.y - elem.first) < distY) {
            distY = std::abs(lastPosition.y - elem.first);
            closeY = elem.first;
        }
    }
    double speed = speedParameter->doubleValue();

    if (distX <= distY) {
        lastPosition.x = closeX;
        // direction
        lastVelocity.x = speed;
        lastVelocity.y = 0;
        // set target
        targetPosition.x = closeX;
        if (speed > 0) {
            for (auto elem : intersectionX[closeX]) {
                if (elem.y > lastPosition.y) {
                    targetPosition.y = elem.y;
                    break;
                }
            }
        }
        else {
            for (auto it = intersectionX[closeX].rbegin(); it != intersectionX[closeX].rend(); ++it) {
                if (it->y < lastPosition.y) {
                    targetPosition.y = it->y;
                    break;
                }
            }
        }
    }
    else {
        lastPosition.y = closeY;
        //direction
        lastVelocity.x = 0;
        lastVelocity.y = speed;
        // set target
        targetPosition.y = closeY;
        if (speed > 0) {
            for (auto elem : intersectionY[closeY]) {
                if (elem.x > lastPosition.x) {
                    targetPosition.x = elem.x;
                    break;
                }
            }
        }
        else {
            for (auto it = intersectionY[closeY].rbegin(); it != intersectionY[closeY].rend(); ++it) {
                if (it->x < lastPosition.x) {
                    targetPosition.x = it->x;
                    break;
                }
            }
        }
    }
}

void ManhattanGrid::setTargetPosition() {
    if (nextMoveIsWait) {
        simtime_t waitTime = waitTimeParameter->doubleValue();
        nextChange = simTime() + waitTime;
        nextMoveIsWait = false;
    }
    else {
        Coord cross;
        CrossPoints neig;
        if (lastVelocity.x != 0) {
            // movement in a x line
            auto it = intersectionX.find(lastPosition.x);
            auto pos = std::find(it->second.begin(), it->second.end(), lastPosition);
            if (pos == it->second.end()) {
                // not in cross road, search the cross road
                if (lastVelocity.x > 0) {
                    for (auto elem : it->second) {
                        if (elem.y > lastPosition.y) {
                            cross.y = elem.y;
                            cross.x = lastPosition.x;
                            neig.push_back(cross);
                            break;
                        }
                    }
                }
                else {
                    for (auto it2 = it->second.rbegin(); it2 != it->second.rend(); ++it2) {
                        if (it2->y < lastPosition.y) {
                            cross.y = it2->y;
                            cross.x = lastPosition.x;
                            neig.push_back(cross);
                            break;
                        }
                    }
                }
            }
            else {
                cross = *pos;
                if (*pos != it->second.front() && *pos != it->second.back()) {
                    // search the point
                    neig.push_back(*(pos + 1));
                    neig.push_back(*(pos - 1));
                } else if (*pos == it->second.front())
                    neig.push_back(*(pos + 1));
                else if (*pos == it->second.back())
                    neig.push_back(*(pos - 1));
                else
                    throw cRuntimeError("neigh not found");

                auto it2 = intersectionY.find(lastPosition.y);
                auto pos2 = std::find(it2->second.begin(), it2->second.end(),
                        lastPosition);
                if (pos2 == it2->second.end())
                    throw cRuntimeError("Point not found");
                if (cross != *pos2)
                    throw cRuntimeError("Point not found");

                if (*pos2 != it2->second.front()
                        && *pos2 != it2->second.back()) {
                    // search the point
                    neig.push_back(*(pos2 + 1));
                    neig.push_back(*(pos2 - 1));
                } else if (*pos2 == it2->second.front())
                    neig.push_back(*(pos2 + 1));
                else if (*pos2 == it2->second.back())
                    neig.push_back(*(pos2 - 1));
                else
                    throw cRuntimeError("neigh not found");
            }
        }
        else {
            // movement in a y line lastPosition.y
            auto it = intersectionY.find(lastPosition.y);
            auto pos = std::find(it->second.begin(), it->second.end(), lastPosition);
            if (pos == it->second.end()) {
                // not in cross road, search the cross road
                if (lastVelocity.x > 0) {
                    for (auto elem : it->second) {
                        if (elem.x > lastPosition.x) {
                            cross.x = elem.x;
                            cross.y = lastPosition.y;
                            neig.push_back(cross);
                            break;
                        }
                    }
                }
                else {
                    for (auto it2 = it->second.rbegin(); it2 != it->second.rend(); ++it2) {
                        if (it2->x < lastPosition.x) {
                            cross.x = it2->x;
                            cross.y = lastPosition.y;
                            neig.push_back(cross);
                            break;
                        }
                    }
                }
            }
            else {

                cross = *pos;

                if (*pos != it->second.front() && *pos != it->second.back()) {
                    // search the point
                    neig.push_back(*(pos + 1));
                    neig.push_back(*(pos - 1));
                } else if (*pos == it->second.front())
                    neig.push_back(*(pos + 1));
                else if (*pos == it->second.back())
                    neig.push_back(*(pos - 1));
                else
                    throw cRuntimeError("neigh not found");
                auto it2 = intersectionX.find(lastPosition.x);
                auto pos2 = std::find(it2->second.begin(), it2->second.end(),
                        lastPosition);
                if (pos2 == it2->second.end())
                    throw cRuntimeError("Point not found");
                if (cross != *pos2)
                    throw cRuntimeError("Point not found");
                if (*pos2 != it2->second.front()
                        && *pos2 != it2->second.back()) {
                    // search the point
                    neig.push_back(*(pos2 + 1));
                    neig.push_back(*(pos2 - 1));
                } else if (*pos2 == it2->second.front())
                    neig.push_back(*(pos2 + 1));
                else if (*pos2 == it2->second.back())
                    neig.push_back(*(pos2 - 1));
                else
                    throw cRuntimeError("neigh not found");
            }
        }
        // search neigbours cross points.
        // remove the latest

        for (auto it = neig.begin(); it != neig.end(); ++it) {
            if (*it == lastCroosPoint) {
                neig.erase(it);
                break;
            }
        }

        Coord target(targetPosition.x,targetPosition.y);
        lastCroosPoint = target;
        auto pos = intuniform(0,neig.size()-1);
        auto next = neig[pos];
        targetPosition.x = next.x;
        targetPosition.y = next.y;
        double speed = speedParameter->doubleValue();
        double distance = lastPosition.distance(targetPosition);
        simtime_t travelTime = distance / speed;
        nextChange = simTime() + travelTime;
        nextMoveIsWait = hasWaitTime;
    }
}

void ManhattanGrid::move()
{
    LineSegmentsMobilityBase::move();
    raiseErrorIfOutside();
}

double ManhattanGrid::getMaxSpeed() const
{
    return speedParameter->isExpression() ? NaN : speedParameter->doubleValue();
}


} // namespace inet

