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

#ifndef __INET_IRADIOSIGNALRECEIVER_H_
#define __INET_IRADIOSIGNALRECEIVER_H_

#include "IRadioSignalListening.h"
#include "IRadioSignalNoise.h"
#include "IRadioSignalReception.h"
#include "IRadioSignalListeningDecision.h"
#include "IRadioSignalReceptionDecision.h"
#include "RadioControlInfo_m.h"

/**
 * This interface represents a physical device (a part of the radio) which converts
 * electric signals into packets.
 *
 * The receiver interface supports optimistic parallel computation of reception
 * results. For this reason some functions are marked to be purely functional.
 * They are qualified with const and all of their parameters are also qualified
 * with const. Moreover they don't access any state that can change over time to
 * avoid non-deterministic behavior. These functions may be called from background
 * threads running parallel with the main simulation thread. They may also be
 * called several times due to cache invalidation before the actual result is
 * needed.
 */
// TODO: this is rather an interface for receivers that support "what if" questions for the future (parallel computation)
// TODO: the reception API must be purely functional enforced by the compiler (unfortunately this is impossible in C++)
// TODO: so we need to be very careful, because it's pretty easy to break the purely function API just be reading a non-const member
// TODO: it's probably better to split this interface and all other interfaces into two, one with state that changes over time
// TDOO: and another with purely function API qualified with const functions and members variables
class INET_API IRadioSignalReceiver : public IPrintableObject
{
    public:
        /**
         * Returns the minimum interference power below which receptions are to
         * be ignored while computing the interference. Returns a value in the
         * range [0, +infinity] or NaN if unspecified.
         */
        virtual W getMinInterferencePower() const = 0;

        /**
         * Returns the minimum reception power below which successful reception
         * is definitely not possible. Returns a value in the range [0, +infinity]
         * or NaN if unspecified.
         */
        virtual W getMinReceptionPower() const = 0;

        /**
         * Returns a description of how the receiver is listening on the radio channel.
         */
        virtual const IRadioSignalListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const = 0;

        /**
         * Returns the result of the listening process specifying the reception
         * state of the receiver.
         */
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const = 0;

        /**
         * Returns whether the transmission can be received successfully or not.
         */
        virtual bool computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const = 0;

        /**
         * Returns whether the reception is actually attempted or ignored by the
         * receiver.
         */
        virtual bool computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const = 0;

        /**
         * Returns the result of the reception process specifying whether it was
         * successful or not and any other physical properties. This function must
         * be purely functional and support optimistic parallel computation.
         */
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const = 0;
};

#endif
