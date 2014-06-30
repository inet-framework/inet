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

#ifndef __INET_PHYSICALOBJECT_H_
#define __INET_PHYSICALOBJECT_H_

#include "Shape.h"
#include "EulerAngles.h"
#include "Material.h"

/**
 * This class represents a physical object, a rigid body and its physical properties.
 * The object can change its position and orientation over time.
 */
class INET_API PhysicalObject
{
    protected:
        const int id;
        Coord position;
        EulerAngles orientation;
        const Shape *shape;
        const Material *material;
        const cFigure::Color color;

    public:
        PhysicalObject(int id, const Coord& position, const EulerAngles& orientation, const Shape *shape, const Material *material, const cFigure::Color &color);
        virtual ~PhysicalObject();

        virtual int getId() const { return id; }

        virtual const Coord& getPosition() const { return position; }
        virtual void setPosition(const Coord& position) { this->position = position; }

        virtual const EulerAngles& getOrientation() const { return orientation; }
        virtual void setOrientation(const EulerAngles& orientation) { this->orientation = orientation; }

        virtual const Shape *getShape() const { return shape; }
        virtual const Material *getMaterial() const { return material; }
        virtual const cFigure::Color& getColor() const { return color; }
};

#endif
