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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmDefs.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {


Ieee80211OfdmModeBase::Ieee80211OfdmModeBase(const Ieee80211OfdmModulation *modulation, const Ieee80211OfdmCode *code, Hz channelSpacing, Hz bandwidth) :
    Ieee80211OfdmTimingRelatedParametersBase(channelSpacing),
    modulation(modulation),
    code(code),
    bandwidth(bandwidth),
    netBitrate(bps(NaN)),
    grossBitrate(bps(NaN))
{
}

Ieee80211OfdmMode::Ieee80211OfdmMode(const char *name, const Ieee80211OfdmPreambleMode *preambleMode, const Ieee80211OfdmSignalMode *signalMode, const Ieee80211OfdmDataMode *dataMode, Hz channelSpacing, Hz bandwidth) :
    Ieee80211ModeBase(name),
    Ieee80211OfdmTimingRelatedParametersBase(channelSpacing),
    preambleMode(preambleMode),
    signalMode(signalMode),
    dataMode(dataMode)
{
}

Ieee80211OfdmPreambleMode::Ieee80211OfdmPreambleMode(Hz channelSpacing) :
    Ieee80211OfdmTimingRelatedParametersBase(channelSpacing)
{
}

Ieee80211OfdmSignalMode::Ieee80211OfdmSignalMode(const Ieee80211OfdmCode *code, const Ieee80211OfdmModulation *modulation, Hz channelSpacing, Hz bandwidth, unsigned int rate) :
    Ieee80211OfdmModeBase(modulation, code, channelSpacing, bandwidth),
    rate(rate)
{
}

Ieee80211OfdmDataMode::Ieee80211OfdmDataMode(const Ieee80211OfdmCode *code, const Ieee80211OfdmModulation *modulation, Hz channelSpacing, Hz bandwidth) :
    Ieee80211OfdmModeBase(modulation, code, channelSpacing, bandwidth)
{
}

bps Ieee80211OfdmModeBase::computeGrossBitrate(const Ieee80211OfdmModulation *modulation) const
{
    int codedBitsPerOFDMSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    return bps(codedBitsPerOFDMSymbol / getSymbolInterval());
}

bps Ieee80211OfdmModeBase::computeNetBitrate(bps grossBitrate, const Ieee80211OfdmCode *code) const
{
    const ConvolutionalCode *convolutionalCode = code ? code->getConvolutionalCode() : nullptr;
    if (convolutionalCode)
        return grossBitrate * convolutionalCode->getCodeRatePuncturingK() / convolutionalCode->getCodeRatePuncturingN();
    return grossBitrate;
}

bps Ieee80211OfdmModeBase::getGrossBitrate() const
{
    if (std::isnan(grossBitrate.get()))
        grossBitrate = computeGrossBitrate(modulation);
    return grossBitrate;
}

bps Ieee80211OfdmModeBase::getNetBitrate() const
{
    if (std::isnan(netBitrate.get()))
        netBitrate = computeNetBitrate(getGrossBitrate(), code);
    return netBitrate;
}

const simtime_t Ieee80211OfdmMode::getSlotTime() const
{
    if (channelSpacing == MHz(20))
        return 9E-6;
    else if (channelSpacing == MHz(10))
        return 13E-6;
    else if (channelSpacing == MHz(5))
        return 21E-6;
    else
        throw cRuntimeError("Unknown channel spacing = %f", channelSpacing.get());
}

const simtime_t Ieee80211OfdmMode::getSifsTime() const
{
    if (channelSpacing == MHz(20))
        return 16E-6;
    else if (channelSpacing == MHz(10))
        return 32E-6;
    else if (channelSpacing == MHz(5))
        return 64E-6;
    else
        throw cRuntimeError("Unknown channel spacing = %f", channelSpacing.get());
}

const simtime_t Ieee80211OfdmMode::getCcaTime() const
{
    // < 4, < 8, < 16
    if (channelSpacing == MHz(20))
        return 4E-6;
    else if (channelSpacing == MHz(10))
        return 8E-6;
    else if (channelSpacing == MHz(5))
        return 16E-6;
    else
        throw cRuntimeError("Unknown channel spacing = %f", channelSpacing.get());
}

const simtime_t Ieee80211OfdmMode::getPhyRxStartDelay() const
{
    if (channelSpacing == MHz(20))
        return 25E-6;
    else if (channelSpacing == MHz(10))
        return 49E-6;
    else if (channelSpacing == MHz(5))
        return 97E-6;
    else
        throw cRuntimeError("Unknown channel spacing = %f", channelSpacing.get());
}

const simtime_t Ieee80211OfdmMode::getRifsTime() const
{
    return -1;
}

const simtime_t Ieee80211OfdmMode::getRxTxTurnaroundTime() const
{
    // TODO: < 2;
    return -1;
}

std::ostream& Ieee80211OfdmPreambleMode::printToStream(std::ostream& stream, int level) const
{
    return stream << "Ieee80211OfdmPreambleMode";
}

std::ostream& Ieee80211OfdmSignalMode::printToStream(std::ostream& stream, int level) const
{
    return stream << "Ieee80211OfdmSignalMode";
}

std::ostream& Ieee80211OfdmDataMode::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OfdmDataMode";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", netBitrate = " << netBitrate;
    return stream;
}

std::ostream& Ieee80211OfdmMode::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OfdmMode";
    if (level <= PRINT_LEVEL_DEBUG)
        stream << ", preambleMode = " << printObjectToString(preambleMode, level + 1)
               << ", signalMode = " << printObjectToString(signalMode, level + 1);
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", dataMode = " << printObjectToString(dataMode, level + 1);
    return stream;
}

b Ieee80211OfdmDataMode::getPaddingLength(b dataLength) const
{
    unsigned int codedBitsPerOFDMSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol; // N_DBPS
    if (code->getConvolutionalCode()) {
        const ConvolutionalCode *convolutionalCode = code->getConvolutionalCode();
        dataBitsPerOFDMSymbol = convolutionalCode->getDecodedLength(codedBitsPerOFDMSymbol);
    }
    unsigned int dataBitsLength = 6 + b(dataLength).get() + 16;
    unsigned int numberOfOFDMSymbols = lrint(ceil(1.0 * dataBitsLength / dataBitsPerOFDMSymbol));
    unsigned int numberOfBitsInTheDataField = dataBitsPerOFDMSymbol * numberOfOFDMSymbols; // N_DATA
    unsigned int numberOfPadBits = numberOfBitsInTheDataField - dataBitsLength; // N_PAD
    return b(numberOfPadBits);
}

b Ieee80211OfdmDataMode::getCompleteLength(b dataLength) const
{
    return dataLength + getTailFieldLength() + getPaddingLength(dataLength);
}

const simtime_t Ieee80211OfdmDataMode::getDuration(b dataLength) const
{
    // IEEE Std 802.11-2007, section 17.3.2.2, table 17-3
    // corresponds to N_{DBPS} in the table
    unsigned int codedBitsPerOFDMSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    const ConvolutionalCode *convolutionalCode = code ? code->getConvolutionalCode() : nullptr;
    unsigned int dataBitsPerOFDMSymbol = convolutionalCode ? convolutionalCode->getDecodedLength(codedBitsPerOFDMSymbol) : codedBitsPerOFDMSymbol;
    // IEEE Std 802.11-2007, section 17.3.5.3, equation (17-11)
    unsigned int numberOfSymbols = lrint(ceil((double)(getServiceFieldLength() + getCompleteLength(dataLength)).get() / dataBitsPerOFDMSymbol));
    // Add signal extension for ERP PHY
    return numberOfSymbols * getSymbolInterval();
}

const Ieee80211OfdmMode& Ieee80211OfdmCompliantModes::getCompliantMode(unsigned int signalRateField, Hz channelSpacing)
{
    // Table 18-6—Contents of the SIGNAL field
    if (channelSpacing == MHz(20)) {
        switch (signalRateField) {
            case 13: // 1101
                return ofdmMode6MbpsCS20MHz;

            case 15: // 1111
                return ofdmMode9MbpsCS20MHz;

            case 5: // 0101
                return ofdmMode12MbpsCS20MHz;

            case 7: // 0111
                return ofdmMode18MbpsCS20MHz;

            case 9: // 1001
                return ofdmMode24MbpsCS20MHz;

            case 11: // 1011
                return ofdmMode36Mbps;

            case 1: // 0001
                return ofdmMode48Mbps;

            case 3: // 0011
                return ofdmMode54Mbps;

            default:
                throw cRuntimeError("%d is not a valid rate", signalRateField);
        }
    }
    else if (channelSpacing == MHz(10)) {
        switch (signalRateField) {
            case 13:
                return ofdmMode3MbpsCS10MHz;

            case 15:
                return ofdmMode4_5MbpsCS10MHz;

            case 5:
                return ofdmMode6MbpsCS10MHz;

            case 7:
                return ofdmMode9MbpsCS10MHz;

            case 9:
                return ofdmMode12MbpsCS10MHz;

            case 11:
                return ofdmMode18MbpsCS10MHz;

            case 1:
                return ofdmMode24MbpsCS10MHz;

            case 3:
                return ofdmMode27Mbps;

            default:
                throw cRuntimeError("%d is not a valid rate", signalRateField);
        }
    }
    else if (channelSpacing == MHz(5)) {
        switch (signalRateField) {
            case 13:
                return ofdmMode1_5Mbps;

            case 15:
                return ofdmMode2_25Mbps;

            case 5:
                return ofdmMode3MbpsCS5MHz;

            case 7:
                return ofdmMode4_5MbpsCS5MHz;

            case 9:
                return ofdmMode6MbpsCS5MHz;

            case 11:
                return ofdmMode9MbpsCS5MHz;

            case 1:
                return ofdmMode12MbpsCS5MHz;

            case 3:
                return ofdmMode13_5Mbps;

            default:
                throw cRuntimeError("%d is not a valid rate", signalRateField);
        }
    }
    else
        throw cRuntimeError("Channel spacing = %f must be 5, 10 or 20 MHz", channelSpacing.get());
}


// Preamble modes
const Ieee80211OfdmPreambleMode Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz(MHz(5));
const Ieee80211OfdmPreambleMode Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz(MHz(10));
const Ieee80211OfdmPreambleMode Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz(MHz(20));

// Signal Modes
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate13(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20), 13);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate15(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20), 15);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate5(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20), 5);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate7(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20), 7);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate9(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20), 9);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate11(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20), 11);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate1(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20), 1);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate3(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20), 3);

const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate13(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20), 13);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate15(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20), 15);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate5(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20), 5);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate7(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20), 7);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate9(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20), 9);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate11(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20), 11);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate1(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20), 1);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate3(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20), 3);

const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate13(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20), 13);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate15(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20), 15);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate5(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20), 5);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate7(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20), 7);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate9(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20), 9);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate11(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20), 11);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate1(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20), 1);
const Ieee80211OfdmSignalMode Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate3(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20), 3);

// Data modes
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode1_5Mbps(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleaving, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode2_25Mbps(&Ieee80211OfdmCompliantCodes::ofdmCC3_4BPSKInterleaving, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(5), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode3MbpsCS5MHz(&Ieee80211OfdmCompliantCodes::ofdmCC1_2QPSKInterleaving, &Ieee80211OfdmCompliantModulations::qpskModulation, MHz(5), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode3MbpsCS10MHz(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleaving, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode4_5MbpsCS5MHz(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QPSKInterleaving, &Ieee80211OfdmCompliantModulations::qpskModulation, MHz(5), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode4_5MbpsCS10MHz(&Ieee80211OfdmCompliantCodes::ofdmCC3_4BPSKInterleaving, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(10), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode6MbpsCS5MHz(&Ieee80211OfdmCompliantCodes::ofdmCC1_2QAM16Interleaving, &Ieee80211OfdmCompliantModulations::qam16Modulation, MHz(5), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode6MbpsCS10MHz(&Ieee80211OfdmCompliantCodes::ofdmCC1_2QPSKInterleaving, &Ieee80211OfdmCompliantModulations::qpskModulation, MHz(10), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode6MbpsCS20MHz(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleaving, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode9MbpsCS5MHz(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM16Interleaving, &Ieee80211OfdmCompliantModulations::qam16Modulation, MHz(5), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode9MbpsCS10MHz(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QPSKInterleaving, &Ieee80211OfdmCompliantModulations::qpskModulation, MHz(10), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode9MbpsCS20MHz(&Ieee80211OfdmCompliantCodes::ofdmCC3_4BPSKInterleaving, &Ieee80211OfdmCompliantModulations::bpskModulation, MHz(20), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode12MbpsCS5MHz(&Ieee80211OfdmCompliantCodes::ofdmCC2_3QAM64Interleaving, &Ieee80211OfdmCompliantModulations::qam64Modulation, MHz(5), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode12MbpsCS10MHz(&Ieee80211OfdmCompliantCodes::ofdmCC1_2QAM16Interleaving, &Ieee80211OfdmCompliantModulations::qam16Modulation, MHz(10), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode12MbpsCS20MHz(&Ieee80211OfdmCompliantCodes::ofdmCC1_2QPSKInterleaving, &Ieee80211OfdmCompliantModulations::qpskModulation, MHz(20), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode13_5Mbps(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM64Interleaving, &Ieee80211OfdmCompliantModulations::qam64Modulation, MHz(5), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode18MbpsCS10MHz(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM16Interleaving, &Ieee80211OfdmCompliantModulations::qam16Modulation, MHz(10), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode18MbpsCS20MHz(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QPSKInterleaving, &Ieee80211OfdmCompliantModulations::qpskModulation, MHz(20), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode24MbpsCS10MHz(&Ieee80211OfdmCompliantCodes::ofdmCC2_3QAM64Interleaving, &Ieee80211OfdmCompliantModulations::qam64Modulation, MHz(10), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode24MbpsCS20MHz(&Ieee80211OfdmCompliantCodes::ofdmCC1_2QAM16Interleaving, &Ieee80211OfdmCompliantModulations::qam16Modulation, MHz(20), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode27Mbps(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM64Interleaving, &Ieee80211OfdmCompliantModulations::qam64Modulation, MHz(10), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode36Mbps(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM16Interleaving, &Ieee80211OfdmCompliantModulations::qam16Modulation, MHz(20), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode48Mbps(&Ieee80211OfdmCompliantCodes::ofdmCC2_3QAM64Interleaving, &Ieee80211OfdmCompliantModulations::qam64Modulation, MHz(20), MHz(20));
const Ieee80211OfdmDataMode Ieee80211OfdmCompliantModes::ofdmDataMode54Mbps(&Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM64Interleaving, &Ieee80211OfdmCompliantModulations::qam64Modulation, MHz(20), MHz(20));

// Modes
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode1_5Mbps("ofdmMode1_5Mbps", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate13, &Ieee80211OfdmCompliantModes::ofdmDataMode1_5Mbps, MHz(5), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode2_25Mbps("ofdmMode2_25Mbps", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate15, &Ieee80211OfdmCompliantModes::ofdmDataMode2_25Mbps, MHz(5), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode3MbpsCS5MHz("ofdmMode3MbpsCS5MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate5, &Ieee80211OfdmCompliantModes::ofdmDataMode3MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode3MbpsCS10MHz("ofdmMode3MbpsCS10MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate13, &Ieee80211OfdmCompliantModes::ofdmDataMode3MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode4_5MbpsCS5MHz("ofdmMode4_5MbpsCS5MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate7, &Ieee80211OfdmCompliantModes::ofdmDataMode4_5MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode4_5MbpsCS10MHz("ofdmMode4_5MbpsCS10MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate15, &Ieee80211OfdmCompliantModes::ofdmDataMode4_5MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS5MHz("ofdmMode6MbpsCS5MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate9, &Ieee80211OfdmCompliantModes::ofdmDataMode6MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS10MHz("ofdmMode6MbpsCS10MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate5, &Ieee80211OfdmCompliantModes::ofdmDataMode6MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS20MHz("ofdmMode6MbpsCS20MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate13, &Ieee80211OfdmCompliantModes::ofdmDataMode6MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode9MbpsCS5MHz("ofdmMode9MbpsCS5MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate11, &Ieee80211OfdmCompliantModes::ofdmDataMode9MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode9MbpsCS10MHz("ofdmMode9MbpsCS10MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate7, &Ieee80211OfdmCompliantModes::ofdmDataMode9MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode9MbpsCS20MHz("ofdmMode9MbpsCS20MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate15, &Ieee80211OfdmCompliantModes::ofdmDataMode9MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode12MbpsCS5MHz("ofdmMode12MbpsCS5MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate1, &Ieee80211OfdmCompliantModes::ofdmDataMode12MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode12MbpsCS10MHz("ofdmMode12MbpsCS10MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate9, &Ieee80211OfdmCompliantModes::ofdmDataMode12MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode12MbpsCS20MHz("ofdmMode12MbpsCS20MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate5, &Ieee80211OfdmCompliantModes::ofdmDataMode12MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode13_5Mbps("ofdmMode13_5Mbps", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode1_5MbpsRate3, &Ieee80211OfdmCompliantModes::ofdmDataMode13_5Mbps, MHz(5), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode18MbpsCS10MHz("ofdmMode18MbpsCS10MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate11, &Ieee80211OfdmCompliantModes::ofdmDataMode18MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode18MbpsCS20MHz("ofdmMode18MbpsCS20MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate7, &Ieee80211OfdmCompliantModes::ofdmDataMode18MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode24MbpsCS10MHz("ofdmMode24MbpsCS10MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate1, &Ieee80211OfdmCompliantModes::ofdmDataMode24MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode24MbpsCS20MHz("ofdmMode24MbpsCS20MHz", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate9, &Ieee80211OfdmCompliantModes::ofdmDataMode24MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode27Mbps("ofdmMode27Mbps", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode3MbpsRate3, &Ieee80211OfdmCompliantModes::ofdmDataMode27Mbps, MHz(10), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode36Mbps("ofdmMode36Mbps", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate11, &Ieee80211OfdmCompliantModes::ofdmDataMode36Mbps, MHz(20), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode48Mbps("ofdmMode48Mbps", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate1, &Ieee80211OfdmCompliantModes::ofdmDataMode48Mbps, MHz(20), MHz(20));
const Ieee80211OfdmMode Ieee80211OfdmCompliantModes::ofdmMode54Mbps("ofdmMode54Mbps", &Ieee80211OfdmCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate3, &Ieee80211OfdmCompliantModes::ofdmDataMode54Mbps, MHz(20), MHz(20));

} // namespace physicallayer
} // namespace inet

