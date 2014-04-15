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

#ifndef __INET_IRADIOSIGNALANALOGMODEL_H_
#define __INET_IRADIOSIGNALANALOGMODEL_H_

#include "IPrintableObject.h"

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the analog domain.
 */
class INET_API IRadioSignalAnalogModel : public IPrintableObject
{
    public:
        virtual ~IRadioSignalAnalogModel() {}

        virtual const simtime_t getDuration() const = 0;
};

class INET_API IRadioSignalTransmissionAnalogModel : public virtual IRadioSignalAnalogModel
{
};

class INET_API IRadioSignalReceptionAnalogModel : public virtual IRadioSignalAnalogModel
{
    public:
        /**
         * Returns the signal to noise plus interference ratio.
         */
        virtual double getSNIR() const = 0;
};

#endif
