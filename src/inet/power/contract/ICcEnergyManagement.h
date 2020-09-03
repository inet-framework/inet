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

#ifndef __INET_ICCENERGYMANAGEMENT_H
#define __INET_ICCENERGYMANAGEMENT_H

#include "inet/power/contract/IEnergyManagement.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API ICcEnergyManagement : public IEnergyManagement
{
  public:
    /**
     * Returns the estimated charge capacity in the range [0, +infinity).
     * It specifies the amount of charge that the energy storage contains at
     * the moment according to the estimation of the management.
     */
    virtual C getEstimatedChargeCapacity() const = 0;
};

} // namespace power

} // namespace inet

#endif

