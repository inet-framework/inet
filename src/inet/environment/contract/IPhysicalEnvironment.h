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

#ifndef __INET_IPHYSICALENVIRONMENT_H
#define __INET_IPHYSICALENVIRONMENT_H

#include "inet/common/IVisitor.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Rotation.h"
#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/environment/contract/IPhysicalObject.h"
#include "inet/environment/contract/IMaterialRegistry.h"

namespace inet {

namespace physicalenvironment {

class INET_API IPhysicalEnvironment
{
  public:
    virtual const Coord& getSpaceMin() const = 0;
    virtual const Coord& getSpaceMax() const = 0;
    virtual const IMaterialRegistry *getMaterialRegistry() const = 0;

    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const = 0;
};

} // namespace physicalenvironment

} // namespace inet

#endif // ifndef __INET_IPHYSICALENVIRONMENT_H

