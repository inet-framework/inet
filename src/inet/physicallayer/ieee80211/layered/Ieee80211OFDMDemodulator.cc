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

#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMDemodulator.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/APSKSymbol.h"
#include "inet/physicallayer/layered/SignalBitModel.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OFDMDemodulator);

void Ieee80211OFDMDemodulator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        signalModulationScheme = &BPSKModulation::singleton;
        const char *modulationSchemeStr = par("demodulationScheme");
        if (!strcmp("QAM-16", modulationSchemeStr))
            dataModulationScheme = &QAM16Modulation::singleton;
        else if (!strcmp("QAM-64", modulationSchemeStr))
            dataModulationScheme = &QAM64Modulation::singleton;
        else if (!strcmp("QPSK", modulationSchemeStr))
            dataModulationScheme = &QPSKModulation::singleton;
        else if (!strcmp("BPSK", modulationSchemeStr))
            dataModulationScheme = &BPSKModulation::singleton;
        else
            throw cRuntimeError("Unknown modulation scheme = %s", modulationSchemeStr);
    }
}

BitVector Ieee80211OFDMDemodulator::demodulateSignalSymbol(const OFDMSymbol *signalSymbol) const
{
    EV_DEBUG << "Demodulating the following SIGNAL symbols: " << *signalSymbol << endl;
    return demodulateField(signalSymbol, signalModulationScheme);
}

BitVector Ieee80211OFDMDemodulator::demodulateDataSymbol(const OFDMSymbol *dataSymbol) const
{
    EV_DEBUG << "Demodulating the following DATA symbols: " << *dataSymbol << endl;
    return demodulateField(dataSymbol, dataModulationScheme);
}

BitVector Ieee80211OFDMDemodulator::demodulateField(const OFDMSymbol *signalSymbol, const APSKModulationBase* modulationScheme) const
{
    std::vector<const APSKSymbol*> apskSymbols = signalSymbol->getSubCarrierSymbols();
    BitVector field;
    for (unsigned int i = 0; i < apskSymbols.size(); i++)
    {
        if (!isPilotOrDcSubcarrier(i))
        {
            const APSKSymbol *apskSymbol = apskSymbols.at(i);
            ShortBitVector bits = modulationScheme->demapToBitRepresentation(apskSymbol);
            for (unsigned int j = 0; j < bits.getSize(); j++)
                field.appendBit(bits.getBit(j));
        }
    }
    EV_DEBUG << "The field symbols has been demodulated into the following bit stream: " << field << endl;
    return field;
}

const IReceptionBitModel* Ieee80211OFDMDemodulator::createBitModel(const BitVector *bitRepresentation) const
{
//    ShortBitVector rate = getRate(bitRepresentation);
    // The bits R1–R4 shall be set, dependent on RATE, according to the values in Table 18-6.
//    const Ieee80211ConvolutionalCode *fecInfo = getFecFromSignalFieldRate(rate);
//    ShortBitVector length;
    // The PLCP LENGTH field shall be an unsigned 12-bit integer that indicates the number of octets in the
    // PSDU that the MAC is currently requesting the PHY to transmit. This value is used by the PHY to
    // determine the number of octet transfers that will occur between the MAC and the PHY after receiving a
    // request to start transmission.
//    for (int i = 4; i < 16; i++)
//        length.appendBit(bitRepresentation->getBit(i));
//    // Note:
    // IScramblerInfo is NULL, since their info is IEEE 802.11 specific (that is,the scrambling
    // method always uses the S(x) = x^7 + x^4 + 1 polynomial.
    // The permutation equations of the interleaving method is fixed but some parameters may vary,
    // those parameters depend on the modulation scheme which we can compute from the SIGNAL field.
//    const Ieee80211Interleaving *interleaving = getInterleavingFromModulation();
//    int bitLength, double bitRate, const BitVector *bits, const IModulation *modulation, double ber, int bitErrorCount) :
    // TODO: ber, bitErrorCount, bitRate
    return new ReceptionBitModel(bitRepresentation->getSize(), 0, bitRepresentation, dataModulationScheme, 0, 0);
}

bool Ieee80211OFDMDemodulator::isPilotOrDcSubcarrier(int i) const
{
   return i == 5 || i == 19 || i == 33 || i == 47 || i == 26; // pilots are: 5,19,33,47, 26 (0+26) is a dc subcarrier
}

const IReceptionBitModel* Ieee80211OFDMDemodulator::demodulate(const IReceptionSymbolModel* symbolModel) const
{
    const std::vector<const ISymbol*> *symbols = symbolModel->getSymbols();
    const OFDMSymbol *signalSymbol = dynamic_cast<const OFDMSymbol *>(symbols->at(0)); // The first OFDM symbol is the SIGNAL symbol
    BitVector *bitRepresentation = new BitVector(demodulateSignalSymbol(signalSymbol));
    for (unsigned int i = 1; i < symbols->size(); i++)
    {
        const OFDMSymbol *dataSymbol = dynamic_cast<const OFDMSymbol *>(symbols->at(i));
        BitVector dataBits = demodulateDataSymbol(dataSymbol);
        for (unsigned int j = 0; j < dataBits.getSize(); j++)
            bitRepresentation->appendBit(dataBits.getBit(j));
    }
    return createBitModel(bitRepresentation);
}

} // namespace physicallayer
} // namespace inet
