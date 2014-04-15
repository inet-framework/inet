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

#ifndef __INET_IRADIOSIGNALSAMPLEMODEL_H_
#define __INET_IRADIOSIGNALSAMPLEMODEL_H_

#include "IPrintableObject.h"

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the waveform or sample domain.
 */
class INET_API IRadioSignalSampleModel : public IPrintableObject
{
    public:
        virtual ~IRadioSignalSampleModel() {}

        virtual int getSampleLength() const = 0;

        virtual double getSampleRate() const = 0;

        virtual const std::vector<W> *getSamples() const = 0;
};

class INET_API IRadioSignalTransmissionSampleModel : public virtual IRadioSignalSampleModel
{
};

class INET_API IRadioSignalReceptionSampleModel : public virtual IRadioSignalSampleModel
{
    public:
        /**
         * Returns the receive signal strength indication.
         */
        virtual const W getRSSI() const = 0;
};

#endif
