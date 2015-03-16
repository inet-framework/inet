//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211OFDMMODULATION_H
#define __INET_IEEE80211OFDMMODULATION_H

#include "inet/physicallayer/base/APSKModulationBase.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OFDMModulation : public IModulation
{
    protected:
        const APSKModulationBase *subcarrierModulation;

    public:
        Ieee80211OFDMModulation(const APSKModulationBase *modulation);

        const APSKModulationBase *getSubcarrierModulation() const { return subcarrierModulation; }
        virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const { return subcarrierModulation->calculateBER(snir, bandwidth, bitrate); }
        virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const { return subcarrierModulation->calculateSER(snir, bandwidth, bitrate); }

        virtual void printToStream(std::ostream& stream) const { std::cout << "Ieee80211OFDMModulation"; }
};

class INET_API Ieee80211OFDMCompliantModulations
{
  // Modulations supported by the OFDM PHY: Table 18-12—Major parameters of the OFDM PHY
  public:
    static const Ieee80211OFDMModulation bpskModulation;
    static const Ieee80211OFDMModulation qpskModulation;
    static const Ieee80211OFDMModulation qam16Modulation;
    static const Ieee80211OFDMModulation qam64Modulation;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211OFDMMODULATION_H */
