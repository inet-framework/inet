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

// TODO: antenna gain should be computable with two angles
// TODO: add antenna "snapshot" to transmission
class INET_API IRadioAntenna
{
    public:
        virtual ~IRadioAntenna() {}

        virtual IMobility *getMobility() const = 0;

        // TODO: is this so simple? 0..1 or what?
        virtual double getGain(Coord direction) const = 0;
};

#endif
