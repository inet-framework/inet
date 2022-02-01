//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLATRECEIVERBASE_H
#define __INET_FLATRECEIVERBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IErrorModel.h"

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

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

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

#endif

