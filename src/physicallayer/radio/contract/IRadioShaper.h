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

#ifndef __INET_IRADIOSHAPER_H_
#define __INET_IRADIOSHAPER_H_

#include "IRadioSignalSymbolModel.h"
#include "IRadioSignalSampleModel.h"

class INET_API IRadioShaper
{
    public:
        virtual ~IRadioShaper() {}

        virtual const IRadioSignalTransmissionSampleModel *shape(const IRadioSignalTransmissionSymbolModel *symbolModel) const = 0;

        virtual const IRadioSignalReceptionSymbolModel *filter(const IRadioSignalReceptionSampleModel *sampleModel) const = 0;
};

#endif
