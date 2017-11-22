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

#ifndef __INET_IEEE80211OFDMERRORMODEL_H
#define __INET_IEEE80211OFDMERRORMODEL_H

#include "inet/physicallayer/contract/bitlevel/ILayeredErrorModel.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmSymbol.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmErrorModel : public Ieee80211NistErrorModel, public ILayeredErrorModel
{
  protected:
    double signalSymbolErrorRate;
    double dataSymbolErrorRate;
    double signalBitErrorRate;
    double dataBitErrorRate;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    Ieee80211OfdmSymbol *corruptOFDMSymbol(const Ieee80211OfdmSymbol *symbol, double ser, int constellationSize, const std::vector<ApskSymbol> *constellation) const;
    void corruptBits(BitVector *bits, double ber, int begin, int end) const;

  public:
    virtual const IReceptionPacketModel *computePacketModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionBitModel *computeBitModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionSymbolModel *computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionSampleModel *computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211LAYEREDERRORMODEL_H

