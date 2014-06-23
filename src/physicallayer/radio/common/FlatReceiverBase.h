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

#ifndef __INET_FLATRECEIVERBASE_H_
#define __INET_FLATRECEIVERBASE_H_

#include "SNIRReceiverBase.h"
#include "IModulation.h"

namespace physicallayer
{

class INET_API FlatReceiverBase : public SNIRReceiverBase
{
    protected:
        const IModulation *modulation;
        W energyDetection;
        W sensitivity;
        Hz carrierFrequency;
        Hz bandwidth;

    protected:
        virtual void initialize(int stage);

        virtual bool computeIsReceptionPossible(const ITransmission *transmission) const;
        virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception) const;
        virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, const RadioReceptionIndication *indication) const;

        virtual bool computeHasBitError(const IListening *listening, double minSNIR, int bitLength, double bitrate) const;

    public:
        FlatReceiverBase() :
            SNIRReceiverBase(),
            modulation(NULL),
            energyDetection(W(sNaN)),
            sensitivity(W(sNaN)),
            carrierFrequency(Hz(sNaN)),
            bandwidth(Hz(sNaN))
        {}

        virtual ~FlatReceiverBase() { delete modulation; }

        virtual void printToStream(std::ostream &stream) const;

        virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;

        virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const;
        virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const;

        virtual const IModulation *getModulation() const { return modulation; }

        virtual Hz getCarrierFrequency() const { return carrierFrequency; }
        virtual void setCarrierFrequency(Hz carrierFrequency) { this->carrierFrequency = carrierFrequency; }

        virtual Hz getBandwidth() const { return bandwidth; }
        virtual void setBandwidth(Hz bandwidth) { this->bandwidth = bandwidth; }
};

}

#endif
