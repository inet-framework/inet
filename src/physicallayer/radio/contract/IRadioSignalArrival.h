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

#ifndef __INET_IRADIOSIGNALARRIVAL_H_
#define __INET_IRADIOSIGNALARRIVAL_H_

#include "Coord.h"
#include "EulerAngles.h"
#include "IPrintableObject.h"

namespace radio
{

/**
 * This interface represents the space and time coordinates of a transmission
 * arriving at a receiver.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IRadioSignalArrival : public IPrintableObject
{
    public:
        virtual const simtime_t getStartPropagationTime() const = 0;
        virtual const simtime_t getEndPropagationTime() const = 0;

        virtual const simtime_t getStartTime() const = 0;
        virtual const simtime_t getEndTime() const = 0;

        virtual const Coord getStartPosition() const = 0;
        virtual const Coord getEndPosition() const = 0;

        virtual const EulerAngles getStartOrientation() const = 0;
        virtual const EulerAngles getEndOrientation() const = 0;
};

}

#endif
