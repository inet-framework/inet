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

// TODO: this is rather an interface for receivers that support "what if" questions for the future
// TODO: create a new interface for stream like transmitters/receivers (transmissionStart, transmissionEnd, receptionStart, receptionEnd)
// TODO: add support for synchronization
// TODO: add support for reception state
// TODO: extract reception, totalNoise interface
// TODO: virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalReception *reception, const IRadioSignalNoise *noise) const = 0;
class INET_API IRadioSignalReceiver : public IPrintableObject
{
    public:
        virtual ~IRadioSignalReceiver() {}

        virtual const IRadioSignalListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const = 0;
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const = 0;

        virtual bool computeIsReceptionPossible(const IRadioSignalReception *reception) const = 0;
        virtual bool computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const = 0;
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const = 0;
};

#endif
