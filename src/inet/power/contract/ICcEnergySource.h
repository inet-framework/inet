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

#ifndef __INET_ICCENERGYSOURCE_H
#define __INET_ICCENERGYSOURCE_H

#include "inet/power/contract/IEnergySource.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API ICcEnergySource : public virtual IEnergySource
{
  public:
    /**
     * The signal that is used to publish current consumption changes.
     */
    static simsignal_t currentConsumptionChangedSignal;

  public:
    /**
     * Returns the open circuit voltage in the range [0, +infinity).
     */
    virtual V getNominalVoltage() const = 0;

    /**
     * Returns the output voltage in the ragne [0, +infinity).
     */
    virtual V getOutputVoltage() const = 0;

    /**
     * Returns the total current consumption in the range [0, +infinity).
     */
    virtual A getTotalCurrentConsumption() const = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ICCENERGYSOURCE_H

