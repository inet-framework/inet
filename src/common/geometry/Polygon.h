//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_POLYGON_H
#define __INET_POLYGON_H

#include "Coord.h"
#include "LineSegment.h"

namespace inet {

class INET_API Polygon
{
  protected:
    std::vector<Coord> points;
    double minZ;
    double maxZ;
    Coord getEdgeOutwardNormalVector(const Coord& edgeP1, const Coord& edgeP2) const;

  public:
    Polygon() {}
    Polygon(const std::vector<Coord>& points);

    const std::vector<Coord>& getPoints() const { return points; }
    virtual Coord computeSize() const;
    Coord getNormalUnitVector() const;
    Coord getNormalVector() const;
    double getMinZ() const { return minZ; }
    double getMaxZ() const { return maxZ; }
    virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const;
};

} // namespace inet

#endif // ifndef __INET_POLYGON_H

