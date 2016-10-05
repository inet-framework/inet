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

#ifndef __INET_IENERGYSOURCE_H
#define __INET_IENERGYSOURCE_H

#include "inet/power/contract/IEnergyConsumer.h"

namespace inet {

namespace power {

/**
 * This class is a base interface that must be implemented by energy source
 * models to integrate with other parts of the power model. This interface is
 * extended by various energy source interfaces. Actual energy source
 * implementations should implement one of the derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IEnergySource
{
  public:
    virtual ~IEnergySource() {}

    /**
     * Returns the number of energy consumers in the range [0, +infinity).
     */
    virtual int getNumEnergyConsumers() const = 0;

    /**
     * Returns the energy consumer for the provided index. This functions throws
     * an exception if the index is out of range, and it never returns nullptr.
     */
    virtual const IEnergyConsumer *getEnergyConsumer(int index) const = 0;

    /**
     * Adds a new energy consumer to the energy source. The energyConsumer
     * parameter must not be nullptr.
     */
    virtual void addEnergyConsumer(const IEnergyConsumer *energyConsumer) = 0;

    /**
     * Removes a previously added energy consumer from this energy source.
     * This functions throws an exception if the consumer is not found.
     */
    virtual void removeEnergyConsumer(const IEnergyConsumer *energyConsumer) = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IENERGYSOURCE_H

