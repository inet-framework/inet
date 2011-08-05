//
// ObstacleControl - models obstacles that block radio transmissions
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <set>

#include "Obstacle.h"

Obstacle::Obstacle(std::string id, double attenuationPerWall, double attenuationPerMeter) :
    visualRepresentation(0),
    id(id),
    attenuationPerWall(attenuationPerWall),
    attenuationPerMeter(attenuationPerMeter)
{
}

void Obstacle::setShape(Coords shape)
{
    coords = shape;
    bboxP1 = Coord(1e7, 1e7);
    bboxP2 = Coord(-1e7, -1e7);
    for (Coords::const_iterator i = coords.begin(); i != coords.end(); ++i)
    {
        bboxP1.x = std::min(i->x, bboxP1.x);
        bboxP1.y = std::min(i->y, bboxP1.y);
        bboxP2.x = std::max(i->x, bboxP2.x);
        bboxP2.y = std::max(i->y, bboxP2.y);
    }
}

const Obstacle::Coords& Obstacle::getShape() const
{
    return coords;
}

const Coord Obstacle::getBboxP1() const
{
    return bboxP1;
}

const Coord Obstacle::getBboxP2() const
{
    return bboxP2;
}


namespace
{
    bool isPointInObstacle(Coord point, const Obstacle& o)
    {
        bool isInside = false;
        const Obstacle::Coords& shape = o.getShape();
        Obstacle::Coords::const_iterator i = shape.begin();
        Obstacle::Coords::const_iterator j = (shape.rbegin()+1).base();
        for (; i != shape.end(); j = i++)
        {
            bool inYRangeUp = (point.y >= i->y) && (point.y < j->y);
            bool inYRangeDown = (point.y >= j->y) && (point.y < i->y);
            bool inYRange = inYRangeUp || inYRangeDown;
            if (!inYRange)
                continue;

            bool intersects = point.x < (i->x + ((point.y - i->y) * (j->x - i->x) / (j->y - i->y)));
            if (!intersects)
                continue;

            isInside = !isInside;
        }
        return isInside;
    }

    double segmentsIntersectAt(Coord p1From, Coord p1To, Coord p2From, Coord p2To)
    {
        /*
         * intersection of P1(x1,y1)<--->P2(x2,y2) and P3(x3,y3)<--->P4(x4,y4) lines:
         *
         * http://paulbourke.net/geometry/lineline2d/
         *
         * The equations of the lines are
         *   Pa = P1 + ua ( P2 - P1 )
         *   Pb = P3 + ub ( P4 - P3 )
         *
         * Solving for the point where Pa = Pb gives the following two equations in two unknowns (ua and ub)
         *   x1 + ua (x2 - x1) = x3 + ub (x4 - x3)
         * and
         *   y1 + ua (y2 - y1) = y3 + ub (y4 - y3)
         *
         * Solving gives the following expressions for ua and ub
         *
         *   ua = ((x4-x3)(y1-y3) - (y4-y3)(x1-x3)) / ((y4-y3)(x2-x1) - (x4-x3)(y2-y1))
         *   ub = ((x2-x1)(y1-y3) - (y2-y1)(x1-x3)) / ((y4-y3)(x2-x1) - (x4-x3)(y2-y1))
         *
         * For segments intersection, ua and ub are must be between 0 and 1
         *
         * Substituting either of these into the corresponding equation for the line gives the
         * intersection point. For example the intersection point (x,y) is
         *   x = x1 + ua (x2 - x1)
         *   y = y1 + ua (y2 - y1)
         */

        Coord p1Vec = p1To - p1From;
        Coord p2Vec = p2To - p2From;
        Coord p1p2 = p1From - p2From;

        double d = (p1Vec.x * p2Vec.y - p1Vec.y * p2Vec.x);

        try
        {
            double p1Frac = (p2Vec.x * p1p2.y - p2Vec.y * p1p2.x) / d;
            if (p1Frac < 0 || p1Frac > 1)
                return -1;

            double p2Frac = (p1Vec.x * p1p2.y - p1Vec.y * p1p2.x) / d;
            if (p2Frac < 0 || p2Frac > 1)
                return -1;

            return p1Frac;
        }
        catch (std::exception& e)
        {
            return -1;  // divide by zero, or overflow when d is too small
        }
    }
}

double Obstacle::calculateReceivedPower(double pSend, double carrierFrequency, const Coord& senderPos,
        double senderAngle, const Coord& receiverPos, double receiverAngle) const
{
    // if obstacles has neither walls nor matter: bail.
    if (getShape().size() < 2)
        return pSend;

    // get a list of points (in [0, 1]) along the line between sender and receiver where the beam intersects with this obstacle
    std::multiset<double> intersectAt;
    bool doesIntersect = false;
    const Obstacle::Coords& shape = getShape();

    Obstacle::Coords::const_iterator j = (shape.rbegin()+1).base();
    for (Obstacle::Coords::const_iterator i = shape.begin(); i != shape.end(); j = i++)
    {
        Coord c1 = *i;
        Coord c2 = *j;

        double i = segmentsIntersectAt(senderPos, receiverPos, c1, c2);
        if (i != -1)
        {
            doesIntersect = true;
            intersectAt.insert(i);
        }
    }

    // if beam interacts with neither walls nor matter: bail.
    bool senderInside = isPointInObstacle(senderPos, *this);
    bool receiverInside = isPointInObstacle(receiverPos, *this);
    if (!doesIntersect && !senderInside && !receiverInside)
        return pSend;

    // make sure every other pair of points marks transition through matter and void, respectively.
    if (senderInside)
        intersectAt.insert(0);
    if (receiverInside)
        intersectAt.insert(1);
    ASSERT((intersectAt.size() % 2) == 0);

    // sum up distances in matter.
    double fractionInObstacle = 0;
    for (std::multiset<double>::const_iterator i = intersectAt.begin(); i != intersectAt.end(); )
    {
        double p1 = *(i++);
        double p2 = *(i++);
        fractionInObstacle += (p2 - p1);
    }

    // calculate attenuation
    double numWalls = intersectAt.size();
    double totalDistance = senderPos.distance(receiverPos);
    double attenuation = (attenuationPerWall * numWalls) + (attenuationPerMeter * fractionInObstacle * totalDistance);
    return pSend * pow(10.0, -attenuation/10.0);
}

