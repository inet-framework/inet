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

#include "StaticEllipseMobility.h"
#include <assert.h>

namespace inet {

Define_Module(StaticEllipseMobility);

StaticEllipseMobility::Point2D StaticEllipseMobility::getIntersection(struct Line2D p, struct Line2D q)
{
    // Long calculations are only the formula of a intersection of two lines defined each with two points.
    double x = ((q.b.x-q.a.x)*(p.b.x*p.a.y-p.a.x*p.b.y) - (p.b.x-p.a.x)*(q.b.x*q.a.y-q.a.x*q.b.y))/((q.b.y-q.a.y)*(p.b.x-p.a.x) - (p.b.y-p.a.y)*(q.b.x-q.a.x));
    double y = ((p.a.y-p.b.y)*(q.b.x*q.a.y-q.a.x*q.b.y) - (q.a.y-q.b.y)*(p.b.x*p.a.y-p.a.x*p.b.y))/((q.b.y-q.a.y)*(p.b.x-p.a.x) - (p.b.y-p.a.y)*(q.b.x-q.a.x));

    struct Point2D result = {x,y};

    return result;
}

void StaticEllipseMobility::setAreaPosition(int numHosts)
{
    // Should ask before
    assert(constraintAreaMax.x < INFINITY);
    assert(constraintAreaMax.y < INFINITY);
    assert(numHosts%4 == 0);

    struct Point2D origin = {0.0, 0.0};

    if(constraintAreaMin.x > -INFINITY)
    {
        origin.x = constraintAreaMin.x;
    }

    if(constraintAreaMin.y > -INFINITY)
    {
        origin.y = constraintAreaMin.y;
    }

    int index = visualRepresentation->getIndex();
    double width = constraintAreaMax.x - origin.x;
    double height = constraintAreaMax.y - origin.y;

    struct Point2D zeroPoint;
    zeroPoint.x = origin.x + width/2;
    zeroPoint.y = origin.y + height/2;

    int numInQuarter = numHosts/4;

    struct Point2D quarter;
    switch (index/numInQuarter)
    {
        case 0: quarter = {-1.0, 1.0}; break;
        case 1: quarter = {1.0, 1.0}; break;
        case 2: quarter = {1.0, -1.0}; break;
        case 3: quarter = {-1.0, -1.0}; break;
    }

    struct Point2D s = {quarter.x*width/2, 0};
    struct Point2D p = {-1*(quarter.x*width/2), 0};

    struct Point2D a;
    // The magic here is quarter.y which defines if the points x-coordinate increase or decrease.
    a.x = (quarter.y*(width*(index%numInQuarter))/numInQuarter) - (quarter.y * width/2);
    // The magic here is quarter.y which defines if the points y-coordinate is positive or negative.
    a.y = quarter.y * height;

    struct Point2D b;
    // The magic here is quarter.x which defines if the points x-coordinate is positive or negative.
    b.x = quarter.x * (width/2);
    // The magic here is quarter.x which defines if the points y-coordinate increase or decrease.
    // Furthermore ((quarter.x+quarter.y)/2*height) defines if the first point is at 0, height or -height.
    b.y = (quarter.x*(-1)) * ((height)*(index%numInQuarter)/numInQuarter) + ((quarter.x+quarter.y)/2*height);

    struct Line2D v = {s,a};
    struct Line2D w = {p,b};

    struct Point2D position = getIntersection(v,w);
    double x = zeroPoint.x + position.x;
    double y = zeroPoint.y + position.y;
    double z = par("initialZ");
    EV << "Position host" << index << " - x: " << x << ", y: " << y << ", z: " << z << "\n";
    lastPosition.x = x;
    lastPosition.y = y;
    lastPosition.z = z;
    recordScalar("x", x);
    recordScalar("y", y);
    recordScalar("z", z);
}

void StaticEllipseMobility::setInitialPosition()
{
    int numHosts = par("numHosts");

    if((constraintAreaMax.x < INFINITY) && (constraintAreaMax.y < INFINITY))
    {
        if ((numHosts % 4) == 0)
        {
            setAreaPosition(numHosts);
        }
        else
        {
            throw cRuntimeError("numHosts should be a multiple of 4 for StaticEllipseMobility with constraintAreaMaxX and constraintAreaMaxX unequal infinity");
        }
    }
    else
    {
        throw cRuntimeError("constraintAreaMax.x and constraintAreaMax.y shouldn't be infinity for StaticEllipseMobility");
    }
}

}// namespace inet
