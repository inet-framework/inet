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

#ifndef __INET_IENERGYSTORAGE_H
#define __INET_IENERGYSTORAGE_H

#include "IEnergySource.h"
#include "IEnergySink.h"

namespace inet {

namespace power {

/**
 * This is an interface that should be implemented by energy storage models to
 * integrate with other parts of the power model. Energy storage models should
 * publish changes to their residual capacity using a signal. This is especially
 * important when they get completely charged or completely depleted.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IEnergyStorage : public virtual IEnergySource, public virtual IEnergySink
{
  public:
    /**
     * The signal that is used to publish residual capacity changes including
     * when the energy storage becomes completely depleted or completely charged.
     */
    static simsignal_t residualCapacityChangedSignal;

  public:
    /**
     * Returns the nominal capacity in the range [0, +infinity]. It specifies
     * the maximum amount of energy that the energy storage can contain.
     */
    virtual J getNominalCapacity() = 0;

    /**
     * Returns the residual capacity in the range [0, nominalCapacity]. It
     * specifies the amount of energy that the energy storage contains at the moment.
     */
    virtual J getResidualCapacity() = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IENERGYSTORAGE_H

