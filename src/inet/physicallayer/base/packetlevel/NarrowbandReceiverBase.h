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

#include "inet/physicallayer/base/packetlevel/SNIRReceiverBase.h"
#include "inet/physicallayer/contract/packetlevel/IModulation.h"
#include "inet/physicallayer/contract/packetlevel/IErrorModel.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandReceiverBase : public SNIRReceiverBase
{
  protected:
    const IModulation *modulation;
    Hz carrierFrequency;
    Hz bandwidth;

  protected:
    virtual void initialize(int stage) override;

    virtual bool computeIsReceptionPossible(const ITransmission *transmission) const override;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception) const override;

  public:
    NarrowbandReceiverBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const override;
    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference, const ISNIR *snir) const override;

    virtual const IModulation *getModulation() const { return modulation; }
    virtual void setModulation(const IModulation *modulation) { this->modulation = modulation; }

    virtual Hz getCarrierFrequency() const { return carrierFrequency; }
    virtual void setCarrierFrequency(Hz carrierFrequency) { this->carrierFrequency = carrierFrequency; }

    virtual Hz getBandwidth() const { return bandwidth; }
    virtual void setBandwidth(Hz bandwidth) { this->bandwidth = bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_NARROWBANDRECEIVERBASE_H

