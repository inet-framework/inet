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

#ifndef __INET_IRADIOSIGNAL_H
#define __INET_IRADIOSIGNAL_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"
#include "inet/common/mapping/MappingBase.h"
#include "inet/common/mapping/MappingUtils.h"

namespace inet {

namespace physicallayer {

class INET_API IRadioSignal
{
  public:
    /**
     * This enumeration specifies a part of a radio signal.
     */
    enum SignalPart
    {
        SIGNAL_PART_NONE = -1,
        SIGNAL_PART_WHOLE,
        SIGNAL_PART_PREAMBLE,
        SIGNAL_PART_HEADER,
        SIGNAL_PART_DATA
    };

  protected:
    /**
     * The enumeration registered for signal part.
     */
    static cEnum *signalPartEnum;

  public:
    /**
     * Returns the name of the provided signal part.
     */
    static const char *getSignalPartName(SignalPart signalPart);

    /**
     * Returns the time when the signal starts at the start position.
     */
    virtual const simtime_t getStartTime() const = 0;

    /**
     * Returns the time when the signal ends at the end position.
     */
    virtual const simtime_t getEndTime() const = 0;

    /**
     * Returns the position where the signal starts at the start time.
     */
    virtual const Coord getStartPosition() const = 0;

    /**
     * Returns the position where the signal ends at the end time.
     */
    virtual const Coord getEndPosition() const = 0;
};

class INET_API INarrowbandSignal
{
  public:
    virtual Hz getCarrierFrequency() const = 0;
    virtual Hz getBandwidth() const = 0;

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const = 0;
};

class INET_API IScalarSignal
{
  public:
    virtual W getPower() const = 0;
};

class INET_API IDimensionalSignal
{
  public:
    virtual const ConstMapping *getPower() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IRADIOSIGNAL_H

