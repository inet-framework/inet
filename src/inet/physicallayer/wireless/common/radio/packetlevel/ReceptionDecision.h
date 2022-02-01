//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEPTIONDECISION_H
#define __INET_RECEPTIONDECISION_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionDecision.h"

namespace inet {
namespace physicallayer {

class INET_API ReceptionDecision : public IReceptionDecision, public cObject
{
  protected:
    const IReception *reception;
    IRadioSignal::SignalPart part;
    const bool isReceptionPossible_;
    const bool isReceptionAttempted_;
    const bool isReceptionSuccessful_;

  public:
    ReceptionDecision(const IReception *reception, IRadioSignal::SignalPart part, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReception *getReception() const override { return reception; }
    virtual IRadioSignal::SignalPart getSignalPart() const override { return part; }

    virtual bool isReceptionPossible() const override { return isReceptionPossible_; }
    virtual bool isReceptionAttempted() const override { return isReceptionAttempted_; }
    virtual bool isReceptionSuccessful() const override { return isReceptionSuccessful_; }
};

} // namespace physicallayer
} // namespace inet

#endif

