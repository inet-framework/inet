//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SNIRRECEIVERBASE_H
#define __INET_SNIRRECEIVERBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ISnir.h"

namespace inet {

namespace physicallayer {

class INET_API SnirReceiverBase : public ReceiverBase
{
  protected:
    enum class SnirThresholdMode {
        STM_UNDEFINED = -1,
        STM_MIN,
        STM_MEAN
    };

    double snirThreshold = NaN;
    SnirThresholdMode snirThresholdMode = SnirThresholdMode::STM_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double getSNIRThreshold() const { return snirThreshold; }

    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

