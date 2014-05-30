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

#ifndef __INET_GRRADIOCHANNEL_H_
#define __INET_GRRADIOCHANNEL_H_

#include "RadioChannel.h"

class INET_API GrRadioChannel : public RadioChannel
{
    public:
        GrRadioChannel()
        {}

        GrRadioChannel(const IRadioSignalPropagation *propagation, const IRadioSignalAttenuation *attenuation, const IRadioBackgroundNoise *backgroundNoise, const simtime_t minInterferenceTime, const simtime_t maxTransmissionDuration, m maxCommunicationRange, m maxInterferenceRange, int numChannels) :
            RadioChannel(propagation, attenuation, backgroundNoise, minInterferenceTime, maxTransmissionDuration, maxCommunicationRange, maxInterferenceRange)
        {}

        virtual cPacket *receivePacket(const IRadio *radio, IRadioFrame *radioFrame);
};

class INET_API GrSignalAttenuation : public IRadioSignalAttenuation
{
    public:
        virtual const IRadioSignalLoss *computeLoss(const IRadioSignalTransmission *transmission, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) const;
        virtual const IRadioSignalReception *computeReception(const IRadio *receiver, const IRadioSignalTransmission *transmission) const;
};

#endif
