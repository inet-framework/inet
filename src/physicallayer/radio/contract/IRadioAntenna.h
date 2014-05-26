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

#ifndef __INET_IRADIOANTENNA_H_
#define __INET_IRADIOANTENNA_H_

#include "IMobility.h"

/**
 * This interface represents a physical device (a part of the radio) which converts
 * electric signals into radio waves, and vice versa.
 */
// TODO: antenna gain should be computable with one angle?
class INET_API IRadioAntenna : public IPrintableObject
{
    public:
        /**
         * Returns the mobility of this antenna that describes its position and
         * orientation over time.
         */
        virtual IMobility *getMobility() const = 0;

        /**
         * Returns the maximum possible gain in any direction.
         */
        virtual double getMaxGain() const = 0;

        /**
         * Returns the antenna gain in the provided direction.
         */
        virtual double getGain(Coord direction) const = 0;
};

#endif
