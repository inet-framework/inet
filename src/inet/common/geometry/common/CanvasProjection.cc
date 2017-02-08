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

CanvasProjection CanvasProjection::defaultCanvasProjection;
std::map<const cCanvas *, const CanvasProjection *> CanvasProjection::canvasProjections;

CanvasProjection::CanvasProjection(Rotation rotation, cFigure::Point translation) :
    rotation(rotation),
    scale(cFigure::Point(1, 1)),
    translation(translation)
{
}

cFigure::Point CanvasProjection::computeCanvasPoint(const Coord& point) const
{
    Coord rotatedPoint = rotation.rotateVectorClockwise(point);
    return cFigure::Point(rotatedPoint.x * scale.x + translation.x, rotatedPoint.y * scale.y + translation.y);
}

const CanvasProjection *CanvasProjection::getCanvasProjection(const cCanvas *canvas)
{
    auto it = canvasProjections.find(canvas);
    if (it == canvasProjections.end())
        return &defaultCanvasProjection;
    else
        return it->second;
}

void CanvasProjection::setCanvasProjection(const cCanvas *canvas, const CanvasProjection *canvasProjection)
{
    canvasProjections[canvas] = canvasProjection;
}

} // namespace inet

