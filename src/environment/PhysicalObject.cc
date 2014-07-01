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

#include "PhysicalObject.h"

namespace inet {

PhysicalObject::PhysicalObject(int id, const Coord& position, const EulerAngles& orientation, const Shape *shape, const Material *material
#ifdef __CCANVAS_H
        , const cFigure::Color &color
#endif
        ) :
    id(id),
    position(position),
    orientation(orientation),
    shape(shape),
    material(material)
#ifdef __CCANVAS_H
    ,color(color)
#endif
{
}

PhysicalObject::~PhysicalObject()
{
    delete shape;
}

}
