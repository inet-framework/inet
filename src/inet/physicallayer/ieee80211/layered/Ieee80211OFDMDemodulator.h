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

#ifndef __INET_OFDMDEMODULATOR_H
#define __INET_OFDMDEMODULATOR_H

#include "inet/physicallayer/contract/ISignalBitModel.h"
#include "inet/physicallayer/contract/ISignalSymbolModel.h"
#include "inet/physicallayer/contract/IDemodulator.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/modulation/OFDMSymbol.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaving.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OFDMDemodulator : public cSimpleModule, public IDemodulator
{
    protected:
        const APSKModulationBase *signalModulationScheme;
        const APSKModulationBase *dataModulationScheme;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("The module doesn't handle self messages"); }
        BitVector demodulateField(const OFDMSymbol *signalSymbol, const APSKModulationBase *modulationScheme) const;
        BitVector demodulateDataSymbol(const OFDMSymbol *dataSymbol) const;
        BitVector demodulateSignalSymbol(const OFDMSymbol *signalSymbol) const;
        const IReceptionBitModel *createBitModel(const BitVector *bitRepresentation) const;
        bool isPilotOrDcSubcarrier(int i) const;

    public:
        virtual const IReceptionBitModel *demodulate(const IReceptionSymbolModel *symbolModel) const;
        void printToStream(std::ostream& stream) const { stream << "TODO"; }
};

} // namespace physicallayer
} // namespace inet

#endif /* __INET_OFDMDEMODULATOR_H */
