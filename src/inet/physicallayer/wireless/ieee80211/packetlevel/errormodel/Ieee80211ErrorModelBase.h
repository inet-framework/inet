//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211ERRORMODELBASE_H
#define __INET_IEEE80211ERRORMODELBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211ErrorModelBase : public ErrorModelBase
{
  protected:
    virtual double getHeaderSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const = 0;
    virtual double getDataSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const = 0;

    virtual Packet *computeCorruptedPacket(const Packet *packet, double ber) const override;

  public:
    Ieee80211ErrorModelBase();

    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

