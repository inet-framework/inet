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

#ifndef __INET_IRADIOSIGNALPROPAGATION_H_
#define __INET_IRADIOSIGNALPROPAGATION_H_

#include "IMobility.h"
#include "IRadioSignalArrival.h"
#include "IRadioSignalTransmission.h"

/**
 * This interface models how a radio signal propagates through space over time.
 */
class INET_API IRadioSignalPropagation
{
    public:
        virtual ~IRadioSignalPropagation() {}

        // TODO: should not be here
        virtual mps getPropagationSpeed() const = 0;

        virtual const IRadioSignalArrival *computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const = 0;
};

#endif
