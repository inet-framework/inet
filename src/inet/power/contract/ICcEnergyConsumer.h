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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_ICCENERGYCONSUMER_H
#define __INET_ICCENERGYCONSUMER_H

#include "inet/power/contract/IEnergyConsumer.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API ICcEnergyConsumer : public virtual IEnergyConsumer
{
  public:
    /**
     * The signal that is used to publish current consumption changes.
     */
    static simsignal_t currentConsumptionChangedSignal;

  public:
    /**
     * Returns the current consumption in the range [0, +infinity).
     */
    virtual A getCurrentConsumption() const = 0;
};

} // namespace power

} // namespace inet

#endif

