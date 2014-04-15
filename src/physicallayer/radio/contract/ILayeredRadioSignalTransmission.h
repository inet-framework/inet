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

#ifndef __INET_ILAYEREDRADIOSIGNALTRANSMISSION_H_
#define __INET_ILAYEREDRADIOSIGNALTRANSMISSION_H_

#include "IRadioSignalTransmission.h"
#include "IRadioSignalPacketModel.h"
#include "IRadioSignalBitModel.h"
#include "IRadioSignalSymbolModel.h"
#include "IRadioSignalSampleModel.h"
#include "IRadioSignalAnalogModel.h"

class INET_API ILayeredRadioSignalTransmission : public virtual IRadioSignalTransmission
{
    public:
        virtual const IRadioSignalTransmissionPacketModel *getPacketModel() const = 0;
        virtual const IRadioSignalTransmissionBitModel    *getBitModel() const = 0;
        virtual const IRadioSignalTransmissionSymbolModel *getSymbolModel() const = 0;
        virtual const IRadioSignalTransmissionSampleModel *getSampleModel() const = 0;
        virtual const IRadioSignalTransmissionAnalogModel *getAnalogModel() const = 0;
};

#endif
