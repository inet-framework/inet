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

#include "Ieee80211OFDMModulator.h"
#include "QAM16Modulation.h"
#include "QAM64Modulation.h"
#include "BPSKModulation.h"
#include "QPSKModulation.h"

namespace inet {
namespace physicallayer {

void Ieee80211OFDMModulator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        const char *modulationSchemeStr = par("modulationScheme");
        if (!strcmp("QAM-16", modulationSchemeStr))
            modulationScheme = &QAM16Modulation::singleton;
        else if (!strcmp("QAM-64", modulationSchemeStr))
            modulationScheme = &QAM64Modulation::singleton;
        else if (!strcmp("QPSK", modulationSchemeStr))
            modulationScheme = &QPSKModulation::singleton;
        else if (!strcmp("BPSK", modulationSchemeStr))
            modulationScheme = &BPSKModulation::singleton;
        else
            throw cRuntimeError("Unknown modulation scheme = %s", modulationSchemeStr);
    }
}

const ITransmissionSymbolModel *Ieee80211OFDMModulator::modulate(const ITransmissionBitModel *bitModel) const
{
    const int codeWordLength = modulationScheme->getCodeWordLength();
    const int symbolLength = preambleSymbolLength + (bitModel->getBitLength() + codeWordLength - 1) / codeWordLength;
    const double symbolRate = bitModel->getBitRate() / codeWordLength;
    return new TransmissionSymbolModel(symbolLength, symbolRate, NULL, modulationScheme);
}


} // namespace physicallayer
} // namespace inet
