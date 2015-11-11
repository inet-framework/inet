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

#include "inet/physicallayer/ieee80211/mode/Ieee80211HTMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HTCode.h"
#include "inet/physicallayer/modulation/QBPSKModulation.h"
#include <tuple>

namespace inet {
namespace physicallayer {

Ieee80211HTCompliantModes Ieee80211HTCompliantModes::singleton;

Ieee80211HTMode::Ieee80211HTMode(const char *name, const Ieee80211HTPreambleMode* preambleMode, const Ieee80211HTDataMode* dataMode, const BandMode carrierFrequencyMode) :
        Ieee80211ModeBase(name),
        preambleMode(preambleMode),
        dataMode(dataMode),
        carrierFrequencyMode(carrierFrequencyMode)
{
}

Ieee80211HTModeBase::Ieee80211HTModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        bandwidth(bandwidth),
        guardIntervalType(guardIntervalType),
        mcsIndex(modulationAndCodingScheme),
        numberOfSpatialStreams(numberOfSpatialStreams),
        netBitrate(bps(NaN)),
        grossBitrate(bps(NaN))
{
}

Ieee80211HTPreambleMode::Ieee80211HTPreambleMode(const Ieee80211HTSignalMode* highThroughputSignalMode, const Ieee80211OFDMSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream) :
        highThroughputSignalMode(highThroughputSignalMode),
        legacySignalMode(legacySignalMode),
        preambleFormat(preambleFormat),
        numberOfHTLongTrainings(computeNumberOfHTLongTrainings(computeNumberOfSpaceTimeStreams(numberOfSpatialStream)))
{
}

Ieee80211HTSignalMode::Ieee80211HTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation *modulation, const Ieee80211HTCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211HTModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType),
        modulation(modulation),
        code(code)
{
}

Ieee80211HTSignalMode::Ieee80211HTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation* modulation, const Ieee80211ConvolutionalCode *convolutionalCode, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211HTModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType),
        modulation(modulation),
        code(Ieee80211HTCompliantCodes::getCompliantCode(convolutionalCode, modulation, nullptr, nullptr, nullptr, bandwidth, false))
{
}


Ieee80211HTDataMode::Ieee80211HTDataMode(const Ieee80211HTMCS *modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211HTModeBase(modulationAndCodingScheme->getMcsIndex(), computeNumberOfSpatialStreams(modulationAndCodingScheme->getModulation(), modulationAndCodingScheme->getStreamExtension1Modulation(), modulationAndCodingScheme->getStreamExtension2Modulation(), modulationAndCodingScheme->getStreamExtension3Modulation()), bandwidth, guardIntervalType),
        modulationAndCodingScheme(modulationAndCodingScheme),
        numberOfBccEncoders(computeNumberOfBccEncoders())
{
}

Ieee80211HTMCS::Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211HTCode* code, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(stream4Modulation),
    code(code)
{
}

Ieee80211HTMCS::Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(stream4Modulation),
    code(Ieee80211HTCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211HTMCS::Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(nullptr),
    code(Ieee80211HTCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211HTMCS::Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    code(Ieee80211HTCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211HTMCS::Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(nullptr),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    code(Ieee80211HTCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

const simtime_t Ieee80211HTPreambleMode::getFirstHTLongTrainingFieldDuration() const
{
    if (preambleFormat == HT_PREAMBLE_MIXED)
        return simtime_t(4E-6);
    else if (preambleFormat == HT_PREAMBLE_GREENFIELD)
        return simtime_t(8E-6);
    else
        throw cRuntimeError("Unknown preamble format");
}


unsigned int Ieee80211HTPreambleMode::computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const
{
    // Table 20-12—Determining the number of space-time streams
    return numberOfSpatialStreams + highThroughputSignalMode->getSTBC();
}

unsigned int Ieee80211HTPreambleMode::computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const
{
    // If the transmitter is providing training for exactly the space-time
    // streams (spatial mapper inputs) used for the transmission of the PSDU,
    // the number of training symbols, N_LTF, is equal to the number of space-time
    // streams, N STS, except that for three space-time streams, four training symbols
    // are required.
    return numberOfSpaceTimeStreams == 3 ? 4 : numberOfSpaceTimeStreams;
}

const simtime_t Ieee80211HTPreambleMode::getDuration() const
{
    // 20.3.7 Mathematical description of signals
    simtime_t sumOfHTLTFs = getFirstHTLongTrainingFieldDuration() + getSecondAndSubsequentHTLongTrainingFielDuration() * (numberOfHTLongTrainings - 1);
    if (preambleFormat == HT_PREAMBLE_MIXED)
        // L-STF -> L-LTF -> L-SIG -> HT-SIG -> HT-STF -> HT-LTF1 -> HT-LTF2 -> ... -> HT_LTFn
        return getNonHTShortTrainingSequenceDuration() + getNonHTLongTrainingFieldDuration() + legacySignalMode->getDuration() + highThroughputSignalMode->getDuration() + getHTShortTrainingFieldDuration() + sumOfHTLTFs;
    else if (preambleFormat == HT_PREAMBLE_GREENFIELD)
        // HT-GF-STF -> HT-LTF1 -> HT-SIG -> HT-LTF2 -> ... -> HT-LTFn
        return getHTGreenfieldShortTrainingFieldDuration() + highThroughputSignalMode->getDuration() + sumOfHTLTFs;
    else
        throw cRuntimeError("Unknown preamble format");
}

bps Ieee80211HTSignalMode::computeGrossBitrate() const
{
    unsigned int numberOfCodedBitsPerSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    if (guardIntervalType == HT_GUARD_INTERVAL_LONG)
        return bps(numberOfCodedBitsPerSymbol / getSymbolInterval());
    else if (guardIntervalType == HT_GUARD_INTERVAL_SHORT)
        return bps(numberOfCodedBitsPerSymbol / getShortGISymbolInterval());
    else
        throw cRuntimeError("Unknown guard interval type");
}

bps Ieee80211HTSignalMode::computeNetBitrate() const
{
    return computeGrossBitrate() * code->getForwardErrorCorrection()->getCodeRate();
}


int Ieee80211HTSignalMode::getBitLength() const
{
    return
        getMCSLength() +
        getCBWLength() +
        getHTLengthLength() +
        getSmoothingLength() +
        getNotSoundingLength() +
        getReservedLength() +
        getAggregationLength() +
        getSTBCLength() +
        getFECCodingLength() +
        getShortGILength() +
        getNumOfExtensionSpatialStreamsLength() +
        getCRCLength() +
        getTailBitsLength();
}

bps Ieee80211HTDataMode::computeGrossBitrate() const
{
    unsigned int numberOfCodedBitsPerSubcarrierSum = computeNumberOfCodedBitsPerSubcarrierSum();
    unsigned int numberOfCodedBitsPerSymbol = numberOfCodedBitsPerSubcarrierSum * getNumberOfDataSubcarriers();
    if (guardIntervalType == HT_GUARD_INTERVAL_LONG)
        return bps(numberOfCodedBitsPerSymbol / getSymbolInterval());
    else if (guardIntervalType == HT_GUARD_INTERVAL_SHORT)
        return bps(numberOfCodedBitsPerSymbol / getShortGISymbolInterval());
    else
        throw cRuntimeError("Unknown guard interval type");
}

bps Ieee80211HTDataMode::computeNetBitrate() const
{
    return getGrossBitrate() * getCode()->getForwardErrorCorrection()->getCodeRate();
}

bps Ieee80211HTModeBase::getNetBitrate() const
{
    if (std::isnan(netBitrate.get()))
        netBitrate = computeNetBitrate();
    return netBitrate;
}

bps Ieee80211HTModeBase::getGrossBitrate() const
{
    if (std::isnan(grossBitrate.get()))
        grossBitrate = computeGrossBitrate();
    return grossBitrate;
}

int Ieee80211HTModeBase::getNumberOfDataSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 52;
    else if (bandwidth == MHz(40))
        // It is a special case, see Table 20-38—MCS parameters for
        // optional 40 MHz MCS 32 format, N SS = 1, N ES = 1
        return mcsIndex == 32 ? 48 : 108;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

int Ieee80211HTModeBase::getNumberOfPilotSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 4;
    else if (bandwidth == MHz(40))
        // It is a spacial case, see the comment above.
        return mcsIndex == 32 ? 4 : 6;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

int Ieee80211HTDataMode::getBitLength(int dataBitLength) const
{
    return getServiceBitLength() + getTailBitLength() + dataBitLength; // TODO: padding?
}

unsigned int Ieee80211HTDataMode::computeNumberOfSpatialStreams(const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation) const
{
    return (stream1Modulation ? 1 : 0) + (stream2Modulation ? 1 : 0) +
           (stream3Modulation ? 1 : 0) + (stream4Modulation ? 1 : 0);
}

unsigned int Ieee80211HTDataMode::computeNumberOfCodedBitsPerSubcarrierSum() const
{
    return
        (modulationAndCodingScheme->getModulation() ? modulationAndCodingScheme->getModulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension1Modulation() ? modulationAndCodingScheme->getStreamExtension1Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension2Modulation()? modulationAndCodingScheme->getStreamExtension2Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension3Modulation() ? modulationAndCodingScheme->getStreamExtension3Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0);
}

unsigned int Ieee80211HTDataMode::computeNumberOfBccEncoders() const
{
    // When the BCC FEC encoder is used, a single encoder is used, except that two encoders
    // are used when the selected MCS has a PHY rate greater than 300 Mb/s (see 20.6).
    return getGrossBitrate() > Mbps(300) ? 2 : 1;
}

const simtime_t Ieee80211HTDataMode::getDuration(int dataBitLength) const
{
    unsigned int numberOfCodedBitsPerSubcarrierSum = computeNumberOfCodedBitsPerSubcarrierSum();
    unsigned int numberOfCodedBitsPerSymbol = numberOfCodedBitsPerSubcarrierSum * getNumberOfDataSubcarriers();
    const IForwardErrorCorrection *forwardErrorCorrection = getCode() ? getCode()->getForwardErrorCorrection() : nullptr;
    unsigned int dataBitsPerSymbol = forwardErrorCorrection ? forwardErrorCorrection->getDecodedLength(numberOfCodedBitsPerSymbol) : numberOfCodedBitsPerSymbol;
    int numberOfSymbols = lrint(ceil((double)getBitLength(dataBitLength) / dataBitsPerSymbol)); // TODO: getBitLength(dataBitLength) should be divisible by dataBitsPerSymbol
    return numberOfSymbols * getSymbolInterval();
}

const simtime_t Ieee80211HTMode::getSlotTime() const
{
    if (carrierFrequencyMode == BAND_2_4GHZ)
        return 20E-6;
    else if (carrierFrequencyMode  == BAND_5GHZ)
        return 9E-6;
    else
        throw cRuntimeError("Unsupported carrier frequency");
}

inline const simtime_t Ieee80211HTMode::getSifsTime() const
{
    if (carrierFrequencyMode == BAND_2_4GHZ)
        return 10E-6;
    else if (carrierFrequencyMode == BAND_5GHZ)
        return 16E-6;
    else
        throw cRuntimeError("Sifs time is not defined for this carrier frequency"); // TODO
}

const simtime_t Ieee80211HTMode::getShortSlotTime() const
{
    if (carrierFrequencyMode == BAND_2_4GHZ)
        return 9E-6;
    else
        throw cRuntimeError("Short slot time is not defined for this carrier frequency"); // TODO
}

Ieee80211HTCompliantModes::Ieee80211HTCompliantModes()
{
}

Ieee80211HTCompliantModes::~Ieee80211HTCompliantModes()
{
    for (auto & entry : modeCache)
        delete entry.second;
}

const Ieee80211HTMode* Ieee80211HTCompliantModes::getCompliantMode(const Ieee80211HTMCS *mcsMode, Ieee80211HTMode::BandMode carrierFrequencyMode, Ieee80211HTPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211HTModeBase::GuardIntervalType guardIntervalType)
{
    const char *name =""; //TODO
    auto htModeId = std::make_tuple(mcsMode->getBandwidth(), mcsMode->getMcsIndex(), guardIntervalType);
    auto mode = singleton.modeCache.find(htModeId);
    if (mode == std::end(singleton.modeCache))
    {
        const Ieee80211OFDMSignalMode *legacySignal = nullptr;
        const Ieee80211HTSignalMode *htSignal = nullptr;
        if (preambleFormat == Ieee80211HTPreambleMode::HT_PREAMBLE_GREENFIELD)
            htSignal = new Ieee80211HTSignalMode(mcsMode->getMcsIndex(), &Ieee80211OFDMCompliantModulations::bpskModulation, Ieee80211HTCompliantCodes::getCompliantCode(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OFDMCompliantModulations::bpskModulation, nullptr, nullptr, nullptr, mcsMode->getBandwidth(), false), mcsMode->getBandwidth(), guardIntervalType);
        else if (preambleFormat == Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED)
        {
            legacySignal = &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate13;
            htSignal = new Ieee80211HTSignalMode(mcsMode->getMcsIndex(), &Ieee80211OFDMCompliantModulations::qbpskModulation, Ieee80211HTCompliantCodes::getCompliantCode(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OFDMCompliantModulations::qbpskModulation, nullptr, nullptr, nullptr, mcsMode->getBandwidth(), false), mcsMode->getBandwidth(), guardIntervalType);
        }
        else
            throw cRuntimeError("Unknown preamble format");
        const Ieee80211HTDataMode *dataMode = new Ieee80211HTDataMode(mcsMode, mcsMode->getBandwidth(), guardIntervalType);
        const Ieee80211HTPreambleMode *preambleMode = new Ieee80211HTPreambleMode(htSignal, legacySignal, preambleFormat, dataMode->getNumberOfSpatialStreams());
        const Ieee80211HTMode *htMode = new Ieee80211HTMode(name, preambleMode, dataMode, carrierFrequencyMode);
        singleton.modeCache.insert(std::pair<std::tuple<Hz, unsigned int, Ieee80211HTModeBase::GuardIntervalType>, const Ieee80211HTMode *>(htModeId, htMode));
        return htMode;
    }
    return mode->second;
}

Ieee80211HTMCS::~Ieee80211HTMCS()
{
    delete code;
}

Ieee80211HTSignalMode::~Ieee80211HTSignalMode()
{
    delete code;
}

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs0BW20MHz([](){ return new Ieee80211HTMCS(0, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs1BW20MHz([](){ return new Ieee80211HTMCS(1, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs2BW20MHz([](){ return new Ieee80211HTMCS(2, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs3BW20MHz([](){ return new Ieee80211HTMCS(3, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs4BW20MHz([](){ return new Ieee80211HTMCS(4, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs5BW20MHz([](){ return new Ieee80211HTMCS(5, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs6BW20MHz([](){ return new Ieee80211HTMCS(6, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs7BW20MHz([](){ return new Ieee80211HTMCS(7, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211HTCompliantCodes::htConvolutionalCode5_6, MHz(20));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs8BW20MHz([](){ return new Ieee80211HTMCS(8, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs9BW20MHz([](){ return new Ieee80211HTMCS(9, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs10BW20MHz([](){ return new Ieee80211HTMCS(10, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs11BW20MHz([](){ return new Ieee80211HTMCS(11, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs12BW20MHz([](){ return new Ieee80211HTMCS(12, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs13BW20MHz([](){ return new Ieee80211HTMCS(13, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs14BW20MHz([](){ return new Ieee80211HTMCS(14, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs15BW20MHz([](){ return new Ieee80211HTMCS(15, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211HTCompliantCodes::htConvolutionalCode5_6, MHz(20));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs16BW20MHz([](){ return new Ieee80211HTMCS(16, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs17BW20MHz([](){ return new Ieee80211HTMCS(17, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs18BW20MHz([](){ return new Ieee80211HTMCS(18, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs19BW20MHz([](){ return new Ieee80211HTMCS(19, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs20BW20MHz([](){ return new Ieee80211HTMCS(20, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs21BW20MHz([](){ return new Ieee80211HTMCS(21, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs22BW20MHz([](){ return new Ieee80211HTMCS(22, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs23BW20MHz([](){ return new Ieee80211HTMCS(23, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211HTCompliantCodes::htConvolutionalCode5_6, MHz(20));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs24BW20MHz([](){ return new Ieee80211HTMCS(24, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs25BW20MHz([](){ return new Ieee80211HTMCS(25, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs26BW20MHz([](){ return new Ieee80211HTMCS(26, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs27BW20MHz([](){ return new Ieee80211HTMCS(27, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs28BW20MHz([](){ return new Ieee80211HTMCS(28, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs29BW20MHz([](){ return new Ieee80211HTMCS(29, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs30BW20MHz([](){ return new Ieee80211HTMCS(30, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs31BW20MHz([](){ return new Ieee80211HTMCS(31, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211HTCompliantCodes::htConvolutionalCode5_6, MHz(20));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs0BW40MHz([](){ return new Ieee80211HTMCS(0, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs1BW40MHz([](){ return new Ieee80211HTMCS(1, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs2BW40MHz([](){ return new Ieee80211HTMCS(2, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs3BW40MHz([](){ return new Ieee80211HTMCS(3, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs4BW40MHz([](){ return new Ieee80211HTMCS(4, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs5BW40MHz([](){ return new Ieee80211HTMCS(5, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs6BW40MHz([](){ return new Ieee80211HTMCS(6, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs7BW40MHz([](){ return new Ieee80211HTMCS(7, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211HTCompliantCodes::htConvolutionalCode5_6, MHz(40));});


const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs8BW40MHz([](){ return new Ieee80211HTMCS(8, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs9BW40MHz([](){ return new Ieee80211HTMCS(9, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs10BW40MHz([](){ return new Ieee80211HTMCS(10, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs11BW40MHz([](){ return new Ieee80211HTMCS(11, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs12BW40MHz([](){ return new Ieee80211HTMCS(12, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs13BW40MHz([](){ return new Ieee80211HTMCS(13, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs14BW40MHz([](){ return new Ieee80211HTMCS(14, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs15BW40MHz([](){ return new Ieee80211HTMCS(15, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211HTCompliantCodes::htConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs16BW40MHz([](){ return new Ieee80211HTMCS(16, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs17BW40MHz([](){ return new Ieee80211HTMCS(17, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs18BW40MHz([](){ return new Ieee80211HTMCS(18, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs19BW40MHz([](){ return new Ieee80211HTMCS(19, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs20BW40MHz([](){ return new Ieee80211HTMCS(20, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs21BW40MHz([](){ return new Ieee80211HTMCS(21, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs22BW40MHz([](){ return new Ieee80211HTMCS(22, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs23BW40MHz([](){ return new Ieee80211HTMCS(23, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211HTCompliantCodes::htConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs24BW40MHz([](){ return new Ieee80211HTMCS(24, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs25BW40MHz([](){ return new Ieee80211HTMCS(25, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs26BW40MHz([](){ return new Ieee80211HTMCS(26, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs27BW40MHz([](){ return new Ieee80211HTMCS(27, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs28BW40MHz([](){ return new Ieee80211HTMCS(28, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs29BW40MHz([](){ return new Ieee80211HTMCS(29, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs30BW40MHz([](){ return new Ieee80211HTMCS(30, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs31BW40MHz([](){ return new Ieee80211HTMCS(31, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211HTCompliantCodes::htConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs32BW40MHz([](){ return new Ieee80211HTMCS(32, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs33BW20MHz([](){ return new Ieee80211HTMCS(33, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs34BW20MHz([](){ return new Ieee80211HTMCS(34, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs35BW20MHz([](){ return new Ieee80211HTMCS(35, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs36BW20MHz([](){ return new Ieee80211HTMCS(36, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs37BW20MHz([](){ return new Ieee80211HTMCS(37, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs38BW20MHz([](){ return new Ieee80211HTMCS(38, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs39BW20MHz([](){ return new Ieee80211HTMCS(39, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs40BW20MHz([](){ return new Ieee80211HTMCS(40, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs41BW20MHz([](){ return new Ieee80211HTMCS(41, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs42BW20MHz([](){ return new Ieee80211HTMCS(42, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs43BW20MHz([](){ return new Ieee80211HTMCS(43, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs44BW20MHz([](){ return new Ieee80211HTMCS(44, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs45BW20MHz([](){ return new Ieee80211HTMCS(45, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs46BW20MHz([](){ return new Ieee80211HTMCS(46, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs47BW20MHz([](){ return new Ieee80211HTMCS(47, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs48BW20MHz([](){ return new Ieee80211HTMCS(48, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs49BW20MHz([](){ return new Ieee80211HTMCS(49, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs50BW20MHz([](){ return new Ieee80211HTMCS(50, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs51BW20MHz([](){ return new Ieee80211HTMCS(51, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs52BW20MHz([](){ return new Ieee80211HTMCS(52, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});


const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs53BW20MHz([](){ return new Ieee80211HTMCS(53, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs54BW20MHz([](){ return new Ieee80211HTMCS(54, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs55BW20MHz([](){ return new Ieee80211HTMCS(55, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs56BW20MHz([](){ return new Ieee80211HTMCS(56, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs57BW20MHz([](){ return new Ieee80211HTMCS(57, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs58BW20MHz([](){ return new Ieee80211HTMCS(58, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs59BW20MHz([](){ return new Ieee80211HTMCS(59, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs60BW20MHz([](){ return new Ieee80211HTMCS(60, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs61BW20MHz([](){ return new Ieee80211HTMCS(61, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs62BW20MHz([](){ return new Ieee80211HTMCS(62, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs63BW20MHz([](){ return new Ieee80211HTMCS(63, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs64BW20MHz([](){ return new Ieee80211HTMCS(64, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs65BW20MHz([](){ return new Ieee80211HTMCS(65, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs66BW20MHz([](){ return new Ieee80211HTMCS(66, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs67BW20MHz([](){ return new Ieee80211HTMCS(67, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs68BW20MHz([](){ return new Ieee80211HTMCS(68, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs69BW20MHz([](){ return new Ieee80211HTMCS(69, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs70BW20MHz([](){ return new Ieee80211HTMCS(70, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs71BW20MHz([](){ return new Ieee80211HTMCS(71, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs72BW20MHz([](){ return new Ieee80211HTMCS(72, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs73BW20MHz([](){ return new Ieee80211HTMCS(73, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs74BW20MHz([](){ return new Ieee80211HTMCS(74, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs75BW20MHz([](){ return new Ieee80211HTMCS(75, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs76BW20MHz([](){ return new Ieee80211HTMCS(76, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs33BW40MHz([](){ return new Ieee80211HTMCS(33, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs34BW40MHz([](){ return new Ieee80211HTMCS(34, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs35BW40MHz([](){ return new Ieee80211HTMCS(35, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs36BW40MHz([](){ return new Ieee80211HTMCS(36, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs37BW40MHz([](){ return new Ieee80211HTMCS(37, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs38BW40MHz([](){ return new Ieee80211HTMCS(38, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs39BW40MHz([](){ return new Ieee80211HTMCS(39, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs40BW40MHz([](){ return new Ieee80211HTMCS(40, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs41BW40MHz([](){ return new Ieee80211HTMCS(41, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs42BW40MHz([](){ return new Ieee80211HTMCS(42, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs43BW40MHz([](){ return new Ieee80211HTMCS(43, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs44BW40MHz([](){ return new Ieee80211HTMCS(44, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs45BW40MHz([](){ return new Ieee80211HTMCS(45, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs46BW40MHz([](){ return new Ieee80211HTMCS(46, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs47BW40MHz([](){ return new Ieee80211HTMCS(47, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs48BW40MHz([](){ return new Ieee80211HTMCS(48, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs49BW40MHz([](){ return new Ieee80211HTMCS(49, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs50BW40MHz([](){ return new Ieee80211HTMCS(50, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs51BW40MHz([](){ return new Ieee80211HTMCS(51, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs52BW40MHz([](){ return new Ieee80211HTMCS(52, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs53BW40MHz([](){ return new Ieee80211HTMCS(53, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs54BW40MHz([](){ return new Ieee80211HTMCS(54, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs55BW40MHz([](){ return new Ieee80211HTMCS(55, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs56BW40MHz([](){ return new Ieee80211HTMCS(56, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs57BW40MHz([](){ return new Ieee80211HTMCS(57, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs58BW40MHz([](){ return new Ieee80211HTMCS(58, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs59BW40MHz([](){ return new Ieee80211HTMCS(59, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs60BW40MHz([](){ return new Ieee80211HTMCS(60, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs61BW40MHz([](){ return new Ieee80211HTMCS(61, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs62BW40MHz([](){ return new Ieee80211HTMCS(62, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs63BW40MHz([](){ return new Ieee80211HTMCS(63, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs64BW40MHz([](){ return new Ieee80211HTMCS(64, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs65BW40MHz([](){ return new Ieee80211HTMCS(65, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs66BW40MHz([](){ return new Ieee80211HTMCS(66, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs67BW40MHz([](){ return new Ieee80211HTMCS(67, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs68BW40MHz([](){ return new Ieee80211HTMCS(68, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs69BW40MHz([](){ return new Ieee80211HTMCS(69, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs70BW40MHz([](){ return new Ieee80211HTMCS(70, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs71BW40MHz([](){ return new Ieee80211HTMCS(71, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs72BW40MHz([](){ return new Ieee80211HTMCS(72, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs73BW40MHz([](){ return new Ieee80211HTMCS(73, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs74BW40MHz([](){ return new Ieee80211HTMCS(74, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs75BW40MHz([](){ return new Ieee80211HTMCS(75, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

const DI<Ieee80211HTMCS> Ieee80211HTMCSTable::htMcs76BW40MHz([](){ return new Ieee80211HTMCS(76, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

} /* namespace physicallayer */
} /* namespace inet */
