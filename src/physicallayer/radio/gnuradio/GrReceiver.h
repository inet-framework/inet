//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_GRRECEIVER_H_
#define __INET_GRRECEIVER_H_

#include <gnuradio/gr_complex.h>
#include "bbn/bbn_receiver.h"
#include "GenericImplementation.h"
#include "IRadioSignalTransmission.h"
#include "IRadioSignalTransmitter.h"
#include "ImplementationBase.h"
#include "GrTransmitter.h"

class INET_API GrReceiver : public cCompoundModule, public IRadioSignalReceiver
{
        bbn_receiver_sptr receiver;
        mutable simtime_t lastReceptionTime;
    public:
        GrReceiver();
        virtual ~GrReceiver();

        virtual W getMinInterferencePower() const { return W(NaN); }
        virtual W getMinReceptionPower() const { return W(NaN); }

        virtual const IRadioSignalListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;
        virtual const IRadioSignalListeningDecision *computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;

        virtual bool computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const;
        virtual bool computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const;
        virtual const IRadioSignalReceptionDecision *computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const;
        virtual void printToStream(std::ostream&) const;
};

class INET_API GrSignalReceptionDecision : public RadioSignalReceptionDecision
{
        cPacket *macFrame;
    public:
        GrSignalReceptionDecision(const IRadioSignalReception *reception, cPacket *macFrame, const RadioReceptionIndication *indication, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful);

        virtual cPacket *getMacFrame() const { return macFrame; }
};

#endif
