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

#ifndef __INET_IENERGYCONSUMER_H
#define __INET_IENERGYCONSUMER_H

#include "inet/power/base/PowerDefs.h"

namespace inet {

namespace power {

/**
 * This is an interface that should be implemented by energy consumer models to
 * integrate with other parts of the power model. Energy consumers should connect
 * to an energy source that will provide the consumed energy. They should also
 * notify the energy source when the power consumption changes.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IEnergyConsumer
{
  public:
    virtual ~IEnergyConsumer() {}

    /**
     * Returns the current power consumption in the range [0, +infinity).
     */
    virtual W getPowerConsumption() const = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IENERGYCONSUMER_H

