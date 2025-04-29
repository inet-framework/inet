//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEIVERBASE_H
#define __INET_RECEIVERBASE_H

#include "inet/common/CompoundModule.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceiver.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceiverAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

class INET_API ReceiverBase : public CompoundModule, public virtual IReceiver
{
  protected:
    bool ignoreInterference = false;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual W computeSignalPower(const IListening *listening, const ISnir *snir, const IInterference *interference) const;

    virtual Packet *computeReceivedPacket(const ISnir *snir, bool isReceptionSuccessful) const;

  public:
    ReceiverBase() {}

    virtual IReceiverAnalogModel *getAnalogModel() const { return check_and_cast<IReceiverAnalogModel *>(getSubmodule("analogModel")); }

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual W getMinInterferencePower() const override { return W(NaN); }
    virtual W getMinReceptionPower() const override { return W(NaN); }

    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual bool computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const override;

    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

