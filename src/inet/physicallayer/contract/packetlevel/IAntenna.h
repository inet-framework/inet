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

#ifndef __INET_IANTENNA_H
#define __INET_IANTENNA_H

#include "inet/common/INETDefs.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/contract/packetlevel/IAntennaGain.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {
namespace physicallayer {

/**
 * This interface represents a physical device (a part of the radio) which converts
 * electric signals into radio waves, and vice versa.
 */
class INET_API IAntenna : public IPrintableObject
{
  public:
    /**
     * Returns the mobility of this antenna that describes its position and
     * orientation over time.
     */
    virtual IMobility *getMobility() const = 0;

    /**
     * Returns true if the antenna has directional selectivity (i.e. the gain is not always 1).
     */
    virtual bool isDirectional() const = 0;

    /**
     * Returns the antenna's gain calculator for directional selectivity.
     * This object may be copied as needed, i.e. IAntennaGain objects are
     * expected to have no reference to other objects.
     */
    virtual Ptr<const IAntennaGain> getGain() const = 0;

    /**
     * Returns the number of antennas in the array.
     */
    virtual int getNumAntennas() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_IANTENNA_H

