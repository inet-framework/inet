//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONSTANTRATESERRORMODEL_H
#define __INET_CONSTANTRATESERRORMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.h"

namespace inet {

/**
 * An error model that returns its packetErrorRate and bitErrorRate parameters
 * as they are, whatever they are (even a bit error rate of 0 for a lost packet),
 * for the tests of ErrorModelBase and FlatReceiverBase.
 */
class ConstantRatesErrorModel : public physicallayer::ErrorModelBase
{
  protected:
    double packetErrorRate = NaN;
    double bitErrorRate = NaN;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double computePacketErrorRate(const physicallayer::ISnir *snir, physicallayer::IRadioSignal::SignalPart part) const override { return packetErrorRate; }
    virtual double computeBitErrorRate(const physicallayer::ISnir *snir, physicallayer::IRadioSignal::SignalPart part) const override { return bitErrorRate; }
    virtual double computeSymbolErrorRate(const physicallayer::ISnir *snir, physicallayer::IRadioSignal::SignalPart part) const override { return NaN; }
};

} // namespace inet

#endif
