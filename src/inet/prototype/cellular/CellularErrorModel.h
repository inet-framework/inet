//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CELLULARERRORMODEL_H
#define __INET_CELLULARERRORMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.h"

namespace inet {

namespace cellular {

using namespace inet::physicallayer;

/**
 * A dummy transport block error model: the effective SNIR is the linear mean of the SNIR over
 * the cells of the allocation, and the block error rate is a logistic function of it around the
 * required SNIR of the MCS.
 */
class INET_API CellularErrorModel : public ErrorModelBase
{
  protected:
    double slope = NaN;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    // the effective SNIR of the reception, as a ratio
    virtual double computeEffectiveSnir(const ISnir *snir) const;
    // the block error rate of a transport block with the MCS at the effective SNIR (a ratio)
    virtual double computeBlockErrorRate(double effectiveSnir, int mcs) const;

    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override { return NaN; }
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override { return NaN; }

    // a failed transport block is lost as a whole, so the copy of the packet only gets the flag
    virtual Packet *computeCorruptedPacket(const ISnir *snir) const override;
};

} // namespace cellular

} // namespace inet

#endif

