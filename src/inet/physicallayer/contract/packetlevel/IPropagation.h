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

#ifndef __INET_IPROPAGATION_H
#define __INET_IPROPAGATION_H

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

/**
 * This interface models how a radio signal propagates through space over time.
 */
class INET_API IPropagation : public IPrintableObject
{
  public:
    /**
     * Returns the theoretical propagation speed of radio signals in the range
     * (0, +infinity). The value might be different from the approximation
     * provided by the actual computation of arrival times.
     */
    virtual mps getPropagationSpeed() const = 0;

    /**
     * Returns the time and space coordinates when the transmission arrives
     * at the object that moves with the provided mobility. The result might
     * be an approximation only, because there's a tradeoff between precision
     * and performance. This function never returns nullptr.
     */
    virtual const IArrival *computeArrival(const ITransmission *transmission, IMobility *mobility) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IPROPAGATION_H

