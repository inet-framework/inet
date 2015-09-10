//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IMODULATION_H
#define __INET_IMODULATION_H

#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the process of varying one or more physical
 * properties of a periodic waveform, called the carrier signal, with a
 * modulating signal that typically contains information to be transmitted.
 */
class INET_API IModulation : public IPrintableObject
{
  public:
    /**
     * Returns the bit error rate as a function of the signal to noise and
     * interference ratio, the bandwidth, and the gross (physical) bitrate.
     */
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const = 0;

    /**
     * Returns the symbol error rate as a function of the signal to noise
     * and interference ratio, the bandwidth, and the gross (physical) bitrate.
     */
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IMODULATION_H

