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

#include "Ieee80211OFDMDemodulator.h"
#include "QAM16Modulation.h"
#include "QAM64Modulation.h"
#include "BPSKModulation.h"
#include "QPSKModulation.h"
#include "APSKSymbol.h"
#include "SignalBitModel.h"

namespace inet {
namespace physicallayer {

void Ieee80211OFDMDemodulator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        // TODO: It is not obvious now that the modulation scheme should be a parameter.
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
        signalModulationScheme = &BPSKModulation::singleton;
    }
}

BitVector Ieee80211OFDMDemodulator::demodulateSignalSymbol(const OFDMSymbol *signalSymbol) const
{
    return demodulateField(signalSymbol, signalModulationScheme);
}

BitVector Ieee80211OFDMDemodulator::demodulateDataSymbol(const OFDMSymbol *dataSymbol) const
{
    return demodulateField(dataSymbol, modulationScheme);
}

BitVector Ieee80211OFDMDemodulator::demodulateField(const OFDMSymbol *signalSymbol, const APSKModulationBase* modulationScheme) const
{
    std::vector<const APSKSymbol*> apskSymbols = signalSymbol->getSubCarrierSymbols();
    BitVector field;
    for (unsigned int i = 0; i < apskSymbols.size(); i++)
    {
        const APSKSymbol *apskSymbol = apskSymbols.at(i);
        ShortBitVector bits = signalModulationScheme->demapToBitRepresentation(apskSymbol);
        for (unsigned int j = 0; i < bits.getSize(); j++)
            field.appendBit(bits.getBit(j));
    }
    return field;
}

const IReceptionBitModel* Ieee80211OFDMDemodulator::createBitModel(const BitVector *bitRepresentation) const
{
    ShortBitVector rate;
    for (int i = 0; i < 4; i++)
        rate.appendBit(bitRepresentation->getBit(i));
    // The bits R1–R4 shall be set, dependent on RATE, according to the values in Table 18-6.
    const ConvolutionalCoder::ConvolutionalCoderInfo *fecInfo = getFecInfoFromSignalFieldRate(rate);
    ShortBitVector length;
    // The PLCP LENGTH field shall be an unsigned 12-bit integer that indicates the number of octets in the
    // PSDU that the MAC is currently requesting the PHY to transmit. This value is used by the PHY to
    // determine the number of octet transfers that will occur between the MAC and the PHY after receiving a
    // request to start transmission.
    for (int i = 4; i < 16; i++)
        length.appendBit(bitRepresentation->getBit(i));
    // Note:
    // IInterleaverInfo, IScramblerInfo are NULLs, since their infos are IEEE 802.11 specific (that is,
    // the scrambling method always uses the S(x) = x^7 + x^4 + 1 polynomial and similarly the interleaving
    // always uses the permutation equations defined in 18.3.5.7 Data interleaving (so a PHY frame does not
    // contain anything about these procedures), contrarily the FEC decoding may be 1/2 or 3/4, etc. which we
    // can compute from the SIGNAL field) and thus are hard-coded in the Ieee80211LayeredDecoder.
    // TODO: FecInfo, ber, bitErrorCount, bitRate
    // TODO: ConvoluationalCoderInfo needs to be factored out from ConvoluationalCoder
    // TODO: Other infos also need to be factored out from their parent classes.
    return new ReceptionBitModel(bitRepresentation->getSize(), 0, bitRepresentation, fecInfo, NULL, NULL, 0, 0);
}

const ConvolutionalCoder::ConvolutionalCoderInfo* Ieee80211OFDMDemodulator::getFecInfoFromSignalFieldRate(const ShortBitVector& rate) const
{
    // Table 18-6—Contents of the SIGNAL field
    // Table 18-4—Modulation-dependent parameters
    if (rate == ShortBitVector("1101") || rate == ShortBitVector("0101") || rate == ShortBitVector("1001"))
        return NULL; // 1/2
    else if (rate == ShortBitVector("1111") || rate == ShortBitVector("0111") || rate == ShortBitVector("1011") ||
             rate == ShortBitVector("0111"))
        return NULL; // 3/4
    else if (rate == ShortBitVector("0001"))
        return NULL; // 2/3
    else
        throw cRuntimeError("Unknown rate field  = %s", rate.toString().c_str());
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
