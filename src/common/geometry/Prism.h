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

#ifndef __INET_PRISM_H_
#define __INET_PRISM_H_

#include "Shape.h"
#include "Polygon.h"

class INET_API Prism : public Shape
{
    protected:
        double height;
        Polygon base;

    public:
        Prism();

        virtual bool isIntersectingLineSegment(const Coord p1, const Coord p2) const;
        virtual double computeIntersectionDistance(const Coord p1, const Coord p2) const;
};

#endif
