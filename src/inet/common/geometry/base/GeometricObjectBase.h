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

#ifndef __INET_GEOMETRICOBJECTBASE_H
#define __INET_GEOMETRICOBJECTBASE_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class represents a 3 dimensional geometric object positioned and oriented
 * in 3 dimensional space.
 */
class INET_API GeometricObjectBase
{
  public:
    GeometricObjectBase() {}
    virtual ~GeometricObjectBase() {}

    /**
     * Returns true if this geometric object is the same as the unspecified
     * singleton instance of this type.
     */
    virtual bool isNil() const = 0;

    /**
     * Returns true if this geometric object is not completely specified.
     */
    virtual bool isUnspecified() const = 0;
};

} // namespace inet

#endif // ifndef __INET_GEOMETRICOBJECTBASE_H

