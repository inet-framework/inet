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

#include "inet/common/geometry/common/CanvasProjection.h"

namespace inet {

std::map<const cCanvas *, CanvasProjection *> CanvasProjection::canvasProjections;

EXECUTE_ON_SHUTDOWN(CanvasProjection::dropCanvasProjections());

void CanvasProjection::dropCanvasProjections()
{
    for (auto it : canvasProjections)
        delete it.second;
    canvasProjections.clear();
}

CanvasProjection::CanvasProjection(RotationMatrix rotation, cFigure::Point translation) :
    rotation(rotation),
    scale(cFigure::Point(1, 1)),
    translation(translation)
{
}

CanvasProjection::~CanvasProjection()
{
}

cFigure::Point CanvasProjection::computeCanvasPoint(const Coord& point) const
{
    double depth;
    return computeCanvasPoint(point, depth);
}

cFigure::Point CanvasProjection::computeCanvasPoint(const Coord& point, double& depth) const
{
    Coord rotatedPoint = rotation.rotateVector(point);
    depth = rotatedPoint.z;
    return cFigure::Point(rotatedPoint.x * scale.x + translation.x, rotatedPoint.y * scale.y + translation.y);
}

Coord CanvasProjection::computeCanvasPointInverse(const cFigure::Point& point, double depth) const
{
    Coord p((point.x - translation.x) / scale.x, (point.y - translation.y) / scale.y, depth);
    return rotation.rotateVectorInverse(p);
}

CanvasProjection *CanvasProjection::getCanvasProjection(const cCanvas *canvas)
{
    auto it = canvasProjections.find(canvas);
    if (it == canvasProjections.end())
        return canvasProjections[canvas] = new CanvasProjection();
    else
        return it->second;
}

} // namespace inet

