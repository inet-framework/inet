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
 * This interface represents the whole radio medium. It keeps track of all radio
 * signal sources, all radio signal transmissions, and all radio signal receptions.
 *
 * This interface supports optimistic parallel computation of reception decisions
 * and related information.
 */
class INET_API IRadioChannel : public IPrintableObject
{
    public:
        /**
         * Returns the radio signal propagation model of this radio channel. This
         * function never returns NULL.
         */
        virtual const IRadioSignalPropagation *getPropagation() const = 0;

        /**
         * Returns the radio signal attenuation model of this radio channel. This
         * function never returns NULL.
         */
        virtual const IRadioSignalAttenuation *getAttenuation() const = 0;

        /**
         * Returns the background noise model of this radio channel. This function
         * may return NULL.
         */
        virtual const IRadioBackgroundNoise *getBackgroundNoise() const = 0;

        /**
         * Adds a new radio to the radio channel. An exception is thrown if the
         * radio is already added. The radio may immediately start new transmissions
         * and will potentially receive all ongoing and further transmissions.
         */
        virtual void addRadio(const IRadio *radio) = 0;

        /**
         * Removes a radio from the radio channel. An exception is thrown if the
         * radio is not yet added. The radio cannot start new transmissions and
         * will not receive any further transmission including the ongoing ones.
         */
        virtual void removeRadio(const IRadio *radio) = 0;

        virtual void transmitToChannel(const IRadio *transmitter, const IRadioSignalTransmission *transmission) = 0;

        /**
         * Sends a copy of the provided radio frame to all potential receivers on
         * the radio channel.
         */
        virtual void sendToChannel(IRadio *transmitter, const IRadioFrame *frame) = 0;

        /**
         * Returns a new radio frame including a new radio signal transmission
         * that represents the provided mac frame. The mac frame control info
         * is an instance of the RadioTransmissionRequest class.
         */
        virtual IRadioFrame *transmitPacket(const IRadio *transmitter, cPacket *macFrame) = 0;

        /**
         * Returns the mac frame that was transmitted in the provided radio frame.
         * The mac frame control info is an instance of the RadioReceptionIndication
         * class.
         */
        virtual cPacket *receivePacket(const IRadio *receiver, IRadioFrame *radioFrame) = 0;

        /**
         * Returns a reception decision that describes the reception of the provided
         * transmission by the receiver.
         */
        virtual const IRadioSignalReceptionDecision *receiveFromChannel(const IRadio *receiver, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const = 0;

        /**
         * Returns a listening decision that describes that the receiver detects
         * on the radio channel.
         */
        virtual const IRadioSignalListeningDecision *listenOnChannel(const IRadio *receiver, const IRadioSignalListening *listening) const = 0;

        /**
         * Returns true when the radio attempts the reception of the provided
         * transmission.
         */
        virtual bool isReceptionAttempted(const IRadio *receiver, const IRadioSignalTransmission *transmission) const = 0;

        /**
         * Returns the space and time coordinates of the transmission arriving at
         * the provided receiver. This function never returns NULL as long as the
         * transmission is live on the radio channel.
         */
        virtual const IRadioSignalArrival *getArrival(const IRadio *receiver, const IRadioSignalTransmission *transmission) const = 0;
};

#endif
