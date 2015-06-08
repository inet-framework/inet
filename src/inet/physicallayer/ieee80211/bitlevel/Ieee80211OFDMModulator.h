//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IEEE80211OFDMMODULATOR_H
#define __INET_IEEE80211OFDMMODULATOR_H

#include "inet/physicallayer/contract/bitlevel/IModulator.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMSymbol.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMModulation.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OFDMModulator : public IModulator
{
  public:
    static const APSKSymbol negativePilotSubcarrier;
    static const APSKSymbol positivePilotSubcarrier;

  protected:
    const Ieee80211OFDMModulation *subcarrierModulation;
    static const double pilotSubcarrierPolarityVector[127];
    unsigned int pilotSubcarrierPolarityVectorOffset;

  protected:
    int getSubcarrierIndex(int ofdmSymbolIndex) const;
    void insertPilotSubcarriers(Ieee80211OFDMSymbol *ofdmSymbol, int symbolID) const;

  public:
    Ieee80211OFDMModulator(const Ieee80211OFDMModulation *subcarrierModulation, unsigned int polarityVectorOffset);

    virtual const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const override;
    const Ieee80211OFDMModulation *getModulation() const override { return subcarrierModulation; }
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifnded __INET_IEEE80211OFDMMODULATOR_H

