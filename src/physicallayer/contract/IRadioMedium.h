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

#ifndef __INET_IRadioMedium_H_
#define __INET_IRadioMedium_H_

#include "IRadio.h"
#include "IRadioFrame.h"
#include "IArrival.h"
#include "IPropagation.h"
#include "IPathLoss.h"
#include "IAttenuation.h"
#include "IBackgroundNoise.h"
#include "IReceptionDecision.h"
#include "IListeningDecision.h"

namespace inet {

namespace physicallayer
{

/**
 * This interface represents the whole radio medium. It keeps track of all radio
 * signal sources, all radio signal transmissions, and all radio signal receptions.
 *
 * This interface supports optimistic parallel computation of reception decisions
 * and related information.
 */
class INET_API IRadioMedium : public IPrintableObject
{
    public:
        /**
         * Returns the radio signal propagation model of this radio medium. This
         * function never returns NULL.
         */
        virtual const IPropagation *getPropagation() const = 0;

        /**
         * Returns the radio signal path loss model of this radio medium. This
         * function never returns NULL.
         */
        virtual const IPathLoss *getPathLoss() const = 0;

        /**
         * Returns the radio signal attenuation model of this radio medium. This
         * function never returns NULL.
         */
        virtual const IAttenuation *getAttenuation() const = 0;

        /**
         * Returns the background noise model of this radio medium. This function
         * may return NULL.
         */
        virtual const IBackgroundNoise *getBackgroundNoise() const = 0;

        /**
         * Adds a new radio to the radio medium. An exception is thrown if the
         * radio is already added. The radio may immediately start new transmissions
         * and will potentially receive all ongoing and further transmissions.
         */
        virtual void addRadio(const IRadio *radio) = 0;

        /**
         * Removes a radio from the radio medium. An exception is thrown if the
         * radio is not yet added. The radio cannot start new transmissions and
         * will not receive any further transmission including the ongoing ones.
         */
        virtual void removeRadio(const IRadio *radio) = 0;

        /**
         * Returns a new radio frame containing the radio signal transmission that
         * represents the provided MAC frame. A copy of this radio frame is sent
         * to all affected radios. The MAC frame control info must be an instance
         * of the RadioTransmissionRequest class.
         */
        virtual IRadioFrame *transmitPacket(const IRadio *transmitter, cPacket *macFrame) = 0;

        /**
         * Returns the MAC frame that was transmitted in the provided radio frame.
         * The MAC frame control info will be an instance of the RadioReceptionIndication
         * class.
         */
        virtual cPacket *receivePacket(const IRadio *receiver, IRadioFrame *radioFrame) = 0;

        /**
         * Returns a listening decision that describes what the receiver detects
         * on the radio medium.
         */
        virtual const IListeningDecision *listenOnMedium(const IRadio *receiver, const IListening *listening) const = 0;

        /**
         * Returns true when the radio attempts the reception of the provided
         * transmission.
         */
        virtual bool isReceptionAttempted(const IRadio *receiver, const ITransmission *transmission) const = 0;

        /**
         * Returns the space and time coordinates of the transmission arriving at
         * the provided receiver. This function never returns NULL as long as the
         * transmission is live on the radio medium.
         */
        virtual const IArrival *getArrival(const IRadio *receiver, const ITransmission *transmission) const = 0;
};

}

} //namespace


#endif
