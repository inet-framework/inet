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

#ifndef __INET_IRADIOSIGNAL_H_
#define __INET_IRADIOSIGNAL_H_

#include "Coord.h"

/**
 * This purely virtual interface provides an abstraction for different radio signals.
 */
class INET_API IRadioSignal
{
  public:
    virtual ~IRadioSignal() { }

//    virtual Coord getTransmissionBeginPosition() const = 0;
    virtual simtime_t getTransmissionBeginTime() const = 0;

//    virtual Coord getTransmissionEndPosition() const = 0;
    virtual simtime_t getTransmissionEndTime() const = 0;

//    virtual void *getTransmissionPower() const = 0;

//    virtual void *getAttenuations() const = 0;
};

#endif
