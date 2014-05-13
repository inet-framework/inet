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

#ifndef __INET_IRADIOCHANNEL_H_
#define __INET_IRADIOCHANNEL_H_

#include "IRadio.h"
#include "IRadioFrame.h"
#include "IRadioSignalArrival.h"
#include "IRadioSignalPropagation.h"
#include "IRadioSignalAttenuation.h"
#include "IRadioBackgroundNoise.h"
#include "IRadioSignalReceptionDecision.h"
#include "IRadioSignalListeningDecision.h"

/**
 * This purely virtual interface provides an abstraction for different radio channels.
 */
class INET_API IRadioChannel
{
    public:
        virtual ~IRadioChannel() {}

        virtual const IRadioSignalPropagation *getPropagation() const = 0;
        virtual const IRadioSignalAttenuation *getAttenuation() const = 0;
        virtual const IRadioBackgroundNoise *getBackgroundNoise() const = 0;

        virtual void addRadio(const IRadio *radio) = 0;
        virtual void removeRadio(const IRadio *radio) = 0;

        // TODO: separate start and end of transmission to allow mobility changes in between
        virtual void transmitToChannel(const IRadio *radio, const IRadioSignalTransmission *transmission) = 0;
        virtual void sendToChannel(IRadio *radio, const IRadioFrame *frame) = 0;

        virtual IRadioFrame *transmitPacket(const IRadio *radio, cPacket *macFrame) = 0;
        virtual cPacket *receivePacket(const IRadio *radio, IRadioFrame *radioFrame) = 0;

        virtual const IRadioSignalReceptionDecision *receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const = 0;
        virtual const IRadioSignalListeningDecision *listenOnChannel(const IRadio *radio, const IRadioSignalListening *listening) const = 0;

        virtual bool isPotentialReceiver(const IRadio *radio, const IRadioSignalTransmission *transmission) const = 0;
        virtual bool isReceptionAttempted(const IRadio *radio, const IRadioSignalTransmission *transmission) const = 0;

        virtual const IRadioSignalArrival *getArrival(const IRadio *radio, const IRadioSignalTransmission *transmission) const = 0;
};

#endif
