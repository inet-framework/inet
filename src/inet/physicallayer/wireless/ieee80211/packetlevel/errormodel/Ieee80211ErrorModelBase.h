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
    double spectralEfficiency1bit = 2000000.0 / 1000000.0; // 1 bit per symbol with 1 MSPS
    double spectralEfficiency2bit = 2000000.0 / 1000000.0 / 2.0; // 2 bits per symbol, 1 MSPS
    double sirPerfect = 10.0;
    double sirImpossible = 0.1;

  protected:
    virtual double getHeaderSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snir) const = 0;
    virtual double getDataSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snir) const = 0;

    virtual Packet *computeCorruptedPacket(const Packet *packet, double ber) const override;

    virtual double getDsssDbpskSuccessRate(unsigned int bitLength, double snir) const;
    virtual double getDsssDqpskSuccessRate(unsigned int bitLength, double snir) const;
    virtual double getDsssDqpskCck5_5SuccessRate(unsigned int bitLength, double snir) const;
    virtual double getDsssDqpskCck11SuccessRate(unsigned int bitLength, double snir) const;

  public:
    Ieee80211ErrorModelBase();

    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

