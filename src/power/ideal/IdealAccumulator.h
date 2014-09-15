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

#ifndef __INET_IDEALACCUMULATOR_H
#define __INET_IDEALACCUMULATOR_H

#include "IAccumulator.h"
#include "PowerSourceBase.h"

namespace inet {

namespace power {

/**
 * This class implements an ideal accumulator. An ideal accumulator stores
 * infinite amount of energy, its internal resistance is zero and it never
 * becomes depleted.
 *
 * @author Levente Meszaros
 */
class INET_API IdealAccumulator : public PowerSourceBase, public virtual IAccumulator
{
  public:
    virtual J getNominalCapacity() { return J(INFINITY); }

    virtual J getResidualCapacity() { return J(INFINITY); }
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IDEALACCUMULATOR_H

