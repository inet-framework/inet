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

#ifndef __INET_FLATRECEIVERBASE_H
#define __INET_FLATRECEIVERBASE_H

#include "inet/physicallayer/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/contract/packetlevel/IErrorModel.h"

namespace inet {

namespace physicallayer {

class INET_API FlatReceiverBase : public NarrowbandReceiverBase
{
  protected:
    const IErrorModel *errorModel;
    W energyDetection;
    W sensitivity;

  protected:
    virtual void initialize(int stage) override;

    using NarrowbandReceiverBase::computeIsReceptionPossible;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;

    virtual Packet *computeReceivedPacket(const ISnir *snir, bool isReceptionSuccessful) const override;

  public:
    FlatReceiverBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual W getMinReceptionPower() const override { return sensitivity; }

    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;

    virtual const IErrorModel *getErrorModel() const { return errorModel; }

    virtual W getEnergyDetection() const { return energyDetection; }
    virtual void setEnergyDetection(W energyDetection) { this->energyDetection = energyDetection; }

    virtual W getSensitivity() const { return sensitivity; }
    virtual void setSensitivity(W sensitivity) { this->sensitivity = sensitivity; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_FLATRECEIVERBASE_H

