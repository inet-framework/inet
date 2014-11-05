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

#ifndef __INET_NARROWBANDRECEIVERBASE_H
#define __INET_NARROWBANDRECEIVERBASE_H

#include "inet/physicallayer/contract/IModulation.h"
#include "inet/physicallayer/contract/IErrorModel.h"
#include "inet/physicallayer/base/SNIRReceiverBase.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandReceiverBase : public SNIRReceiverBase
{
  protected:
    const IModulation *modulation;
    const IErrorModel *errorModel;
    W energyDetection;
    W sensitivity;
    Hz carrierFrequency;
    Hz bandwidth;

  protected:
    virtual void initialize(int stage);

    virtual bool computeIsReceptionPossible(const ITransmission *transmission) const;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception) const;
    virtual bool computeIsReceptionSuccessful(const ISNIR *snir) const;
    virtual const RadioReceptionIndication *computeReceptionIndication(const ISNIR *snir) const;

  public:
    NarrowbandReceiverBase();
    virtual ~NarrowbandReceiverBase();

    virtual void printToStream(std::ostream& stream) const;

    virtual W getMinReceptionPower() const { return sensitivity; }

    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const;

    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const;
    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const;

    virtual const IModulation *getModulation() const { return modulation; }

    virtual Hz getCarrierFrequency() const { return carrierFrequency; }
    virtual void setCarrierFrequency(Hz carrierFrequency) { this->carrierFrequency = carrierFrequency; }

    virtual Hz getBandwidth() const { return bandwidth; }
    virtual void setBandwidth(Hz bandwidth) { this->bandwidth = bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_NARROWBANDRECEIVERBASE_H

