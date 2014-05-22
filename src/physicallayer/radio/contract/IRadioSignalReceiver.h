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
 * Some functions are marked to be part of the reception process. These must be
 * purely functional because they may be called several times before the actual
 * transmission arrives at the receiver.
 */
// TODO: this is rather an interface for receivers that support "what if" questions for the future (parallel computation)
class INET_API IRadioSignalReceiver : public IPrintableObject
{
    public:
        virtual ~IRadioSignalReceiver() {}

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

        virtual const IRadioSignalListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const = 0;
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const = 0;

        /**
         * Returns whether the transmission can be received successfully or not.
         * Part of the reception process, see class comment.
         */
        virtual bool computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const = 0;

        /**
         * Returns whether the reception is actually attempted or ignored by the
         * receiver. Part of the reception process, see class comment.
         */
        virtual bool computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const = 0;

        /**
         * Returns the result of the reception process specifying whether it is
         * successful or not and any other physical properties. Part of the reception
         * process, see class comment.
         */
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const = 0;
};

#endif
