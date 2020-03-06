//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMMODULATION_H
#define __INET_IEEE80211OFDMMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OfdmModulation : public IModulation
{
  protected:
    int numSubcarriers;
    const ApskModulationBase *subcarrierModulation;

  public:
    Ieee80211OfdmModulation(int numSubcarriers, const ApskModulationBase *subcarrierModulation);

    virtual int getNumSubcarriers() const { return numSubcarriers; }
    const ApskModulationBase *getSubcarrierModulation() const { return subcarrierModulation; }

    // TODO: is it correct to calculate this with the same bandwidth and bitrate when there are subcarriers?
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override { return subcarrierModulation->calculateBER(snir, bandwidth, bitrate); }
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override { return subcarrierModulation->calculateSER(snir, bandwidth, bitrate); }

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

class INET_API Ieee80211OfdmCompliantModulations
{
    // Modulations supported by the OFDM PHY: Table 18-12—Major parameters of the OFDM PHY

  public:
    static const Ieee80211OfdmModulation subcarriers52QbpskModulation;
    static const Ieee80211OfdmModulation subcarriers52BpskModulation;
    static const Ieee80211OfdmModulation subcarriers52QpskModulation;
    static const Ieee80211OfdmModulation subcarriers52Qam16Modulation;
    static const Ieee80211OfdmModulation subcarriers52Qam64Modulation;
    static const Ieee80211OfdmModulation subcarriers52Qam256Modulation;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

