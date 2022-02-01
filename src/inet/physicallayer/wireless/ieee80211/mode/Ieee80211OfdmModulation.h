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
    const ApskModulationBase *subcarrierModulation;

  public:
    Ieee80211OfdmModulation(const ApskModulationBase *subcarrierModulation);

    const ApskModulationBase *getSubcarrierModulation() const { return subcarrierModulation; }
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override { return subcarrierModulation->calculateBER(snir, bandwidth, bitrate); }
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override { return subcarrierModulation->calculateSER(snir, bandwidth, bitrate); }

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

class INET_API Ieee80211OfdmCompliantModulations
{
    // Modulations supported by the OFDM PHY: Table 18-12—Major parameters of the OFDM PHY

  public:
    static const Ieee80211OfdmModulation qbpskModulation;
    static const Ieee80211OfdmModulation bpskModulation;
    static const Ieee80211OfdmModulation qpskModulation;
    static const Ieee80211OfdmModulation qam16Modulation;
    static const Ieee80211OfdmModulation qam64Modulation;
    static const Ieee80211OfdmModulation qam256Modulation;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

