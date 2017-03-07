//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_IENERGYGENERATOR_H
#define __INET_IENERGYGENERATOR_H

#include "inet/power/base/PowerDefs.h"

namespace inet {

namespace power {

class IEnergySink;

/**
 * This class is a base interface that must be implemented by energy generator
 * models to integrate with other parts of the power model. Energy generators
 * connect to an energy sink that absorbs the generated energy. Energy
 * generators are required to notify their energy sink when their energy
 * generation changes. This interface is extended by various energy generator
 * interfaces. Actual energy generator implementations should implement one of
 * the derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IEnergyGenerator
{
  public:
    virtual ~IEnergyGenerator() {}

    /**
     * Returns the energy sink that absorbs energy from this energy generator.
     * This function never returns nullptr.
     */
    virtual IEnergySink *getEnergySink() const = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IENERGYGENERATOR_H

