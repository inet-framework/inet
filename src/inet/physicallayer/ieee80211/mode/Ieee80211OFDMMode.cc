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

#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMDefs.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMModulation.h"

namespace inet {

namespace physicallayer {


Ieee80211OFDMModeBase::Ieee80211OFDMModeBase(const Ieee80211OFDMModulation *modulation, const Ieee80211OFDMCode *code, Hz channelSpacing, Hz bandwidth) :
    Ieee80211OFDMTimingRelatedParametersBase(channelSpacing),
    modulation(modulation),
    code(code),
    bandwidth(bandwidth),
    netBitrate(bps(NaN)),
    grossBitrate(bps(NaN))
{
}

Ieee80211OFDMMode::Ieee80211OFDMMode(const char *name, const Ieee80211OFDMPreambleMode *preambleMode, const Ieee80211OFDMSignalMode *signalMode, const Ieee80211OFDMDataMode *dataMode, Hz channelSpacing, Hz bandwidth) :
    Ieee80211ModeBase(name),
    Ieee80211OFDMTimingRelatedParametersBase(channelSpacing),
    preambleMode(preambleMode),
    signalMode(signalMode),
    dataMode(dataMode)
{
}

Ieee80211OFDMPreambleMode::Ieee80211OFDMPreambleMode(Hz channelSpacing) :
    Ieee80211OFDMTimingRelatedParametersBase(channelSpacing)
{
}

Ieee80211OFDMSignalMode::Ieee80211OFDMSignalMode(const Ieee80211OFDMCode *code, const Ieee80211OFDMModulation *modulation, Hz channelSpacing, Hz bandwidth, unsigned int rate) :
    Ieee80211OFDMModeBase(modulation, code, channelSpacing, bandwidth),
    rate(rate)
{
}

Ieee80211OFDMDataMode::Ieee80211OFDMDataMode(const Ieee80211OFDMCode *code, const Ieee80211OFDMModulation *modulation, Hz channelSpacing, Hz bandwidth) :
    Ieee80211OFDMModeBase(modulation, code, channelSpacing, bandwidth)
{
}

bps Ieee80211OFDMModeBase::computeGrossBitrate(const Ieee80211OFDMModulation *modulation) const
{
    int codedBitsPerOFDMSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    return bps(codedBitsPerOFDMSymbol / getSymbolInterval());
}

bps Ieee80211OFDMModeBase::computeNetBitrate(bps grossBitrate, const Ieee80211OFDMCode *code) const
{
    const ConvolutionalCode *convolutionalCode = code ? code->getConvolutionalCode() : nullptr;
    if (convolutionalCode)
        return grossBitrate * convolutionalCode->getCodeRatePuncturingK() / convolutionalCode->getCodeRatePuncturingN();
    return grossBitrate;
}

bps Ieee80211OFDMModeBase::getGrossBitrate() const
{
    if (std::isnan(grossBitrate.get()))
        grossBitrate = computeGrossBitrate(modulation);
    return grossBitrate;
}

bps Ieee80211OFDMModeBase::getNetBitrate() const
{
    if (std::isnan(netBitrate.get()))
        netBitrate = computeNetBitrate(getGrossBitrate(), code);
    return netBitrate;
}

const simtime_t Ieee80211OFDMMode::getSlotTime() const
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

const simtime_t Ieee80211OFDMMode::getSifsTime() const
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

const simtime_t Ieee80211OFDMMode::getCcaTime() const
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

const simtime_t Ieee80211OFDMMode::getPhyRxStartDelay() const
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

const simtime_t Ieee80211OFDMMode::getRifsTime() const
{
    throw cRuntimeError("Undefined physical layer parameter");
    return SIMTIME_ZERO;
}

const simtime_t Ieee80211OFDMMode::getRxTxTurnaroundTime() const
{
    throw cRuntimeError("< 2");
    return 0;
}

std::ostream& Ieee80211OFDMPreambleMode::printToStream(std::ostream& stream, int level) const
{
    return stream << "Ieee80211OFDMPreambleMode";
}

std::ostream& Ieee80211OFDMSignalMode::printToStream(std::ostream& stream, int level) const
{
    return stream << "Ieee80211OFDMSignalMode";
}

std::ostream& Ieee80211OFDMDataMode::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OFDMDataMode";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", netBitrate = " << netBitrate;
    return stream;
}

std::ostream& Ieee80211OFDMMode::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OFDMMode";
    if (level >= PRINT_LEVEL_DEBUG)
        stream << ", preambleMode = " << printObjectToString(preambleMode, level - 1)
               << ", signalMode = " << printObjectToString(signalMode, level - 1);
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", dataMode = " << printObjectToString(dataMode, level - 1);
    return stream;
}

int Ieee80211OFDMDataMode::getBitLength(int dataBitLength) const
{
    return getServiceBitLength() + dataBitLength + getTailBitLength(); // TODO: padding?
}

const simtime_t Ieee80211OFDMDataMode::getDuration(int dataBitLength) const
{
    // IEEE Std 802.11-2007, section 17.3.2.2, table 17-3
    // corresponds to N_{DBPS} in the table
    unsigned int codedBitsPerOFDMSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    const ConvolutionalCode *convolutionalCode = code ? code->getConvolutionalCode() : nullptr;
    unsigned int dataBitsPerOFDMSymbol = convolutionalCode ? convolutionalCode->getDecodedLength(codedBitsPerOFDMSymbol) : codedBitsPerOFDMSymbol;
    // IEEE Std 802.11-2007, section 17.3.5.3, equation (17-11)
    unsigned int numberOfSymbols = lrint(ceil((double)getBitLength(dataBitLength) / dataBitsPerOFDMSymbol));
    // Add signal extension for ERP PHY
    return numberOfSymbols * getSymbolInterval();
}

const Ieee80211OFDMMode& Ieee80211OFDMCompliantModes::getCompliantMode(unsigned int signalRateField, Hz channelSpacing)
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
const Ieee80211OFDMPreambleMode Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz(MHz(5));
const Ieee80211OFDMPreambleMode Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz(MHz(10));
const Ieee80211OFDMPreambleMode Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz(MHz(20));

// Signal Modes
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate13(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20), 13);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate15(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20), 15);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate5(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20), 5);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate7(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20), 7);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate9(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20), 9);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate11(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20), 11);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate1(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20), 1);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate3(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20), 3);

const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate13(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20), 13);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate15(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20), 15);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate5(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20), 5);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate7(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20), 7);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate9(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20), 9);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate11(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20), 11);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate1(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20), 1);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate3(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20), 3);

const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate13(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20), 13);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate15(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20), 15);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate5(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20), 5);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate7(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20), 7);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate9(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20), 9);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate11(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20), 11);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate1(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20), 1);
const Ieee80211OFDMSignalMode Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate3(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20), 3);

// Data modes
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode1_5Mbps(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleaving, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode2_25Mbps(&Ieee80211OFDMCompliantCodes::ofdmCC3_4BPSKInterleaving, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(5), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode3MbpsCS5MHz(&Ieee80211OFDMCompliantCodes::ofdmCC1_2QPSKInterleaving, &Ieee80211OFDMCompliantModulations::qpskModulation, MHz(5), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode3MbpsCS10MHz(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleaving, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode4_5MbpsCS5MHz(&Ieee80211OFDMCompliantCodes::ofdmCC3_4QPSKInterleaving, &Ieee80211OFDMCompliantModulations::qpskModulation, MHz(5), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode4_5MbpsCS10MHz(&Ieee80211OFDMCompliantCodes::ofdmCC3_4BPSKInterleaving, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(10), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode6MbpsCS5MHz(&Ieee80211OFDMCompliantCodes::ofdmCC1_2QAM16Interleaving, &Ieee80211OFDMCompliantModulations::qam16Modulation, MHz(5), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode6MbpsCS10MHz(&Ieee80211OFDMCompliantCodes::ofdmCC1_2QPSKInterleaving, &Ieee80211OFDMCompliantModulations::qpskModulation, MHz(10), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode6MbpsCS20MHz(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleaving, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode9MbpsCS5MHz(&Ieee80211OFDMCompliantCodes::ofdmCC3_4QAM16Interleaving, &Ieee80211OFDMCompliantModulations::qam16Modulation, MHz(5), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode9MbpsCS10MHz(&Ieee80211OFDMCompliantCodes::ofdmCC3_4QPSKInterleaving, &Ieee80211OFDMCompliantModulations::qpskModulation, MHz(10), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode9MbpsCS20MHz(&Ieee80211OFDMCompliantCodes::ofdmCC3_4BPSKInterleaving, &Ieee80211OFDMCompliantModulations::bpskModulation, MHz(20), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode12MbpsCS5MHz(&Ieee80211OFDMCompliantCodes::ofdmCC2_3QAM64Interleaving, &Ieee80211OFDMCompliantModulations::qam64Modulation, MHz(5), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode12MbpsCS10MHz(&Ieee80211OFDMCompliantCodes::ofdmCC1_2QAM16Interleaving, &Ieee80211OFDMCompliantModulations::qam16Modulation, MHz(10), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode12MbpsCS20MHz(&Ieee80211OFDMCompliantCodes::ofdmCC1_2QPSKInterleaving, &Ieee80211OFDMCompliantModulations::qpskModulation, MHz(20), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode13_5Mbps(&Ieee80211OFDMCompliantCodes::ofdmCC3_4QAM64Interleaving, &Ieee80211OFDMCompliantModulations::qam64Modulation, MHz(5), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode18MbpsCS10MHz(&Ieee80211OFDMCompliantCodes::ofdmCC3_4QAM16Interleaving, &Ieee80211OFDMCompliantModulations::qam16Modulation, MHz(10), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode18MbpsCS20MHz(&Ieee80211OFDMCompliantCodes::ofdmCC3_4QPSKInterleaving, &Ieee80211OFDMCompliantModulations::qpskModulation, MHz(20), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode24MbpsCS10MHz(&Ieee80211OFDMCompliantCodes::ofdmCC2_3QAM64Interleaving, &Ieee80211OFDMCompliantModulations::qam64Modulation, MHz(10), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode24MbpsCS20MHz(&Ieee80211OFDMCompliantCodes::ofdmCC1_2QAM16Interleaving, &Ieee80211OFDMCompliantModulations::qam16Modulation, MHz(20), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode27Mbps(&Ieee80211OFDMCompliantCodes::ofdmCC2_3QAM64Interleaving, &Ieee80211OFDMCompliantModulations::qam64Modulation, MHz(20), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode36Mbps(&Ieee80211OFDMCompliantCodes::ofdmCC3_4QAM16Interleaving, &Ieee80211OFDMCompliantModulations::qam16Modulation, MHz(20), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode48Mbps(&Ieee80211OFDMCompliantCodes::ofdmCC2_3QAM64Interleaving, &Ieee80211OFDMCompliantModulations::qam64Modulation, MHz(20), MHz(20));
const Ieee80211OFDMDataMode Ieee80211OFDMCompliantModes::ofdmDataMode54Mbps(&Ieee80211OFDMCompliantCodes::ofdmCC3_4QAM64Interleaving, &Ieee80211OFDMCompliantModulations::qam64Modulation, MHz(20), MHz(20));

// Modes
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode1_5Mbps("ofdmMode1_5Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate13, &Ieee80211OFDMCompliantModes::ofdmDataMode1_5Mbps, MHz(5), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode2_25Mbps("ofdmMode2_25Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate15, &Ieee80211OFDMCompliantModes::ofdmDataMode2_25Mbps, MHz(5), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode3MbpsCS5MHz("ofdmMode3MbpsCS5MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate5, &Ieee80211OFDMCompliantModes::ofdmDataMode3MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode3MbpsCS10MHz("ofdmMode3MbpsCS10MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate13, &Ieee80211OFDMCompliantModes::ofdmDataMode3MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode4_5MbpsCS5MHz("ofdmMode4_5MbpsCS5MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate7, &Ieee80211OFDMCompliantModes::ofdmDataMode4_5MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode4_5MbpsCS10MHz("ofdmMode4_5MbpsCS10MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate15, &Ieee80211OFDMCompliantModes::ofdmDataMode4_5MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS5MHz("ofdmMode6MbpsCS5MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate9, &Ieee80211OFDMCompliantModes::ofdmDataMode6MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS10MHz("ofdmMode6MbpsCS10MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate5, &Ieee80211OFDMCompliantModes::ofdmDataMode6MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS20MHz("ofdmMode6MbpsCS20MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate13, &Ieee80211OFDMCompliantModes::ofdmDataMode6MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS5MHz("ofdmMode9MbpsCS5MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate11, &Ieee80211OFDMCompliantModes::ofdmDataMode9MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS10MHz("ofdmMode9MbpsCS10MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate7, &Ieee80211OFDMCompliantModes::ofdmDataMode9MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS20MHz("ofdmMode9MbpsCS20MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate15, &Ieee80211OFDMCompliantModes::ofdmDataMode9MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS5MHz("ofdmMode12MbpsCS5MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate1, &Ieee80211OFDMCompliantModes::ofdmDataMode12MbpsCS5MHz, MHz(5), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS10MHz("ofdmMode12MbpsCS10MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate9, &Ieee80211OFDMCompliantModes::ofdmDataMode12MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS20MHz("ofdmMode12MbpsCS20MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate5, &Ieee80211OFDMCompliantModes::ofdmDataMode12MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode13_5Mbps("ofdmMode13_5Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS5MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode1_5MbpsRate3, &Ieee80211OFDMCompliantModes::ofdmDataMode13_5Mbps, MHz(5), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS10MHz("ofdmMode18MbpsCS10MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate11, &Ieee80211OFDMCompliantModes::ofdmDataMode18MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS20MHz("ofdmMode18MbpsCS20MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate7, &Ieee80211OFDMCompliantModes::ofdmDataMode18MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS10MHz("ofdmMode24MbpsCS10MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate1, &Ieee80211OFDMCompliantModes::ofdmDataMode24MbpsCS10MHz, MHz(10), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS20MHz("ofdmMode24MbpsCS20MHz", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate9, &Ieee80211OFDMCompliantModes::ofdmDataMode24MbpsCS20MHz, MHz(20), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode27Mbps("ofdmMode27Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS10MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode3MbpsRate3, &Ieee80211OFDMCompliantModes::ofdmDataMode27Mbps, MHz(10), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode36Mbps("ofdmMode36Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate11, &Ieee80211OFDMCompliantModes::ofdmDataMode36Mbps, MHz(20), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode48Mbps("ofdmMode48Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate1, &Ieee80211OFDMCompliantModes::ofdmDataMode48Mbps, MHz(20), MHz(20));
const Ieee80211OFDMMode Ieee80211OFDMCompliantModes::ofdmMode54Mbps("ofdmMode54Mbps", &Ieee80211OFDMCompliantModes::ofdmPreambleModeCS20MHz, &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate3, &Ieee80211OFDMCompliantModes::ofdmDataMode54Mbps, MHz(20), MHz(20));

} // namespace physicallayer

} // namespace inet
