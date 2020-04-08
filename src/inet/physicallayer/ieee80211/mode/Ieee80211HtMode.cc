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

#include <tuple>

#include "inet/physicallayer/ieee80211/mode/Ieee80211HtCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/modulation/QbpskModulation.h"

namespace inet {
namespace physicallayer {

Ieee80211HtCompliantModes Ieee80211HtCompliantModes::singleton;

Ieee80211HtMode::Ieee80211HtMode(const char *name, const Ieee80211HtPreambleMode* preambleMode, const Ieee80211HtDataMode* dataMode, const BandMode centerFrequencyMode) :
        Ieee80211ModeBase(name),
        preambleMode(preambleMode),
        dataMode(dataMode),
        centerFrequencyMode(centerFrequencyMode)
{
}

Ieee80211HtModeBase::Ieee80211HtModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        bandwidth(bandwidth),
        guardIntervalType(guardIntervalType),
        mcsIndex(modulationAndCodingScheme),
        numberOfSpatialStreams(numberOfSpatialStreams),
        netBitrate(bps(NaN)),
        grossBitrate(bps(NaN))
{
}

Ieee80211HtPreambleMode::Ieee80211HtPreambleMode(const Ieee80211HtSignalMode* highThroughputSignalMode, const Ieee80211OfdmSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream) :
        highThroughputSignalMode(highThroughputSignalMode),
        legacySignalMode(legacySignalMode),
        preambleFormat(preambleFormat),
        numberOfHTLongTrainings(computeNumberOfHTLongTrainings(computeNumberOfSpaceTimeStreams(numberOfSpatialStream)))
{
}

Ieee80211HtSignalMode::Ieee80211HtSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OfdmModulation *modulation, const Ieee80211HtCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211HtModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType),
        modulation(modulation),
        code(code)
{
}

Ieee80211HtSignalMode::Ieee80211HtSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OfdmModulation* modulation, const Ieee80211ConvolutionalCode *convolutionalCode, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211HtModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType),
        modulation(modulation),
        code(Ieee80211HtCompliantCodes::getCompliantCode(convolutionalCode, modulation, nullptr, nullptr, nullptr, bandwidth, false))
{
}


Ieee80211HtDataMode::Ieee80211HtDataMode(const Ieee80211Htmcs *modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211HtModeBase(modulationAndCodingScheme->getMcsIndex(), computeNumberOfSpatialStreams(modulationAndCodingScheme->getModulation(), modulationAndCodingScheme->getStreamExtension1Modulation(), modulationAndCodingScheme->getStreamExtension2Modulation(), modulationAndCodingScheme->getStreamExtension3Modulation()), bandwidth, guardIntervalType),
        modulationAndCodingScheme(modulationAndCodingScheme),
        numberOfBccEncoders(computeNumberOfBccEncoders())
{
}

Ieee80211Htmcs::Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211HtCode* code, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(stream4Modulation),
    code(code)
{
}

Ieee80211Htmcs::Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(stream4Modulation),
    code(Ieee80211HtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Htmcs::Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(nullptr),
    code(Ieee80211HtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Htmcs::Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    code(Ieee80211HtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Htmcs::Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(nullptr),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    code(Ieee80211HtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

const simtime_t Ieee80211HtPreambleMode::getFirstHTLongTrainingFieldDuration() const
{
    if (preambleFormat == HT_PREAMBLE_MIXED)
        return simtime_t(4E-6);
    else if (preambleFormat == HT_PREAMBLE_GREENFIELD)
        return simtime_t(8E-6);
    else
        throw cRuntimeError("Unknown preamble format");
}


unsigned int Ieee80211HtPreambleMode::computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const
{
    // Table 20-12—Determining the number of space-time streams
    return numberOfSpatialStreams + highThroughputSignalMode->getSTBC();
}

unsigned int Ieee80211HtPreambleMode::computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const
{
    // If the transmitter is providing training for exactly the space-time
    // streams (spatial mapper inputs) used for the transmission of the PSDU,
    // the number of training symbols, N_LTF, is equal to the number of space-time
    // streams, N STS, except that for three space-time streams, four training symbols
    // are required.
    return numberOfSpaceTimeStreams == 3 ? 4 : numberOfSpaceTimeStreams;
}

const simtime_t Ieee80211HtPreambleMode::getDuration() const
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

bps Ieee80211HtSignalMode::computeGrossBitrate() const
{
    unsigned int numberOfCodedBitsPerSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    if (guardIntervalType == HT_GUARD_INTERVAL_LONG)
        return bps(numberOfCodedBitsPerSymbol / getSymbolInterval());
    else if (guardIntervalType == HT_GUARD_INTERVAL_SHORT)
        return bps(numberOfCodedBitsPerSymbol / getShortGISymbolInterval());
    else
        throw cRuntimeError("Unknown guard interval type");
}

bps Ieee80211HtSignalMode::computeNetBitrate() const
{
    return computeGrossBitrate() * code->getForwardErrorCorrection()->getCodeRate();
}


b Ieee80211HtSignalMode::getLength() const
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

bps Ieee80211HtDataMode::computeGrossBitrate() const
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

bps Ieee80211HtDataMode::computeNetBitrate() const
{
    return getGrossBitrate() * getCode()->getForwardErrorCorrection()->getCodeRate();
}

bps Ieee80211HtModeBase::getNetBitrate() const
{
    if (std::isnan(netBitrate.get()))
        netBitrate = computeNetBitrate();
    return netBitrate;
}

bps Ieee80211HtModeBase::getGrossBitrate() const
{
    if (std::isnan(grossBitrate.get()))
        grossBitrate = computeGrossBitrate();
    return grossBitrate;
}

int Ieee80211HtModeBase::getNumberOfDataSubcarriers() const
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

int Ieee80211HtModeBase::getNumberOfPilotSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 4;
    else if (bandwidth == MHz(40))
        // It is a spacial case, see the comment above.
        return mcsIndex == 32 ? 4 : 6;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

b Ieee80211HtDataMode::getCompleteLength(b dataLength) const
{
    return getServiceFieldLength() + getTailFieldLength() + dataLength; // TODO: padding?
}

unsigned int Ieee80211HtDataMode::computeNumberOfSpatialStreams(const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation) const
{
    return (stream1Modulation ? 1 : 0) + (stream2Modulation ? 1 : 0) +
           (stream3Modulation ? 1 : 0) + (stream4Modulation ? 1 : 0);
}

unsigned int Ieee80211HtDataMode::computeNumberOfCodedBitsPerSubcarrierSum() const
{
    return
        (modulationAndCodingScheme->getModulation() ? modulationAndCodingScheme->getModulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension1Modulation() ? modulationAndCodingScheme->getStreamExtension1Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension2Modulation()? modulationAndCodingScheme->getStreamExtension2Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension3Modulation() ? modulationAndCodingScheme->getStreamExtension3Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0);
}

unsigned int Ieee80211HtDataMode::computeNumberOfBccEncoders() const
{
    // When the BCC FEC encoder is used, a single encoder is used, except that two encoders
    // are used when the selected MCS has a PHY rate greater than 300 Mb/s (see 20.6).
    return getGrossBitrate() > Mbps(300) ? 2 : 1;
}

const simtime_t Ieee80211HtDataMode::getDuration(b dataLength) const
{
    unsigned int numberOfCodedBitsPerSubcarrierSum = computeNumberOfCodedBitsPerSubcarrierSum();
    unsigned int numberOfCodedBitsPerSymbol = numberOfCodedBitsPerSubcarrierSum * getNumberOfDataSubcarriers();
    const IForwardErrorCorrection *forwardErrorCorrection = getCode() ? getCode()->getForwardErrorCorrection() : nullptr;
    unsigned int dataBitsPerSymbol = forwardErrorCorrection ? forwardErrorCorrection->getDecodedLength(numberOfCodedBitsPerSymbol) : numberOfCodedBitsPerSymbol;
    int numberOfSymbols = lrint(ceil((double)getCompleteLength(dataLength).get() / dataBitsPerSymbol)); // TODO: getBitLength(dataLength) should be divisible by dataBitsPerSymbol
    return numberOfSymbols * getSymbolInterval();
}

const simtime_t Ieee80211HtMode::getSlotTime() const
{
    if (centerFrequencyMode == BAND_2_4GHZ)
        return 20E-6;
    else if (centerFrequencyMode  == BAND_5GHZ)
        return 9E-6;
    else
        throw cRuntimeError("Unsupported carrier frequency");
}

inline const simtime_t Ieee80211HtMode::getSifsTime() const
{
    if (centerFrequencyMode == BAND_2_4GHZ)
        return 10E-6;
    else if (centerFrequencyMode == BAND_5GHZ)
        return 16E-6;
    else
        throw cRuntimeError("Sifs time is not defined for this carrier frequency"); // TODO
}

const simtime_t Ieee80211HtMode::getShortSlotTime() const
{
    if (centerFrequencyMode == BAND_2_4GHZ)
        return 9E-6;
    else
        throw cRuntimeError("Short slot time is not defined for this carrier frequency"); // TODO
}

Ieee80211HtCompliantModes::Ieee80211HtCompliantModes()
{
}

Ieee80211HtCompliantModes::~Ieee80211HtCompliantModes()
{
    for (auto & entry : modeCache)
        delete entry.second;
}

const Ieee80211HtMode* Ieee80211HtCompliantModes::getCompliantMode(const Ieee80211Htmcs *mcsMode, Ieee80211HtMode::BandMode centerFrequencyMode, Ieee80211HtPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211HtModeBase::GuardIntervalType guardIntervalType)
{
    const char *name =""; //TODO
    auto htModeId = std::make_tuple(mcsMode->getBandwidth(), mcsMode->getMcsIndex(), guardIntervalType);
    auto mode = singleton.modeCache.find(htModeId);
    if (mode == std::end(singleton.modeCache))
    {
        const Ieee80211OfdmSignalMode *legacySignal = nullptr;
        const Ieee80211HtSignalMode *htSignal = nullptr;
        switch (preambleFormat) {
            case Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD:
                htSignal = new Ieee80211HtSignalMode(mcsMode->getMcsIndex(), &Ieee80211OfdmCompliantModulations::bpskModulation, Ieee80211HtCompliantCodes::getCompliantCode(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OfdmCompliantModulations::bpskModulation, nullptr, nullptr, nullptr, mcsMode->getBandwidth(), false), mcsMode->getBandwidth(), guardIntervalType);
                break;
            case Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED:
                legacySignal = &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate13;
                htSignal = new Ieee80211HtSignalMode(mcsMode->getMcsIndex(), &Ieee80211OfdmCompliantModulations::qbpskModulation, Ieee80211HtCompliantCodes::getCompliantCode(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OfdmCompliantModulations::qbpskModulation, nullptr, nullptr, nullptr, mcsMode->getBandwidth(), false), mcsMode->getBandwidth(), guardIntervalType);
                break;
            default:
                throw cRuntimeError("Unknown preamble format");
        }
        const Ieee80211HtDataMode *dataMode = new Ieee80211HtDataMode(mcsMode, mcsMode->getBandwidth(), guardIntervalType);
        const Ieee80211HtPreambleMode *preambleMode = new Ieee80211HtPreambleMode(htSignal, legacySignal, preambleFormat, dataMode->getNumberOfSpatialStreams());
        const Ieee80211HtMode *htMode = new Ieee80211HtMode(name, preambleMode, dataMode, centerFrequencyMode);
        singleton.modeCache.insert(std::pair<std::tuple<Hz, unsigned int, Ieee80211HtModeBase::GuardIntervalType>, const Ieee80211HtMode *>(htModeId, htMode));
        return htMode;
    }
    return mode->second;
}

Ieee80211Htmcs::~Ieee80211Htmcs()
{
    delete code;
}

Ieee80211HtSignalMode::~Ieee80211HtSignalMode()
{
    delete code;
}

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs0BW20MHz([](){ return new Ieee80211Htmcs(0, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs1BW20MHz([](){ return new Ieee80211Htmcs(1, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs2BW20MHz([](){ return new Ieee80211Htmcs(2, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs3BW20MHz([](){ return new Ieee80211Htmcs(3, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs4BW20MHz([](){ return new Ieee80211Htmcs(4, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs5BW20MHz([](){ return new Ieee80211Htmcs(5, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs6BW20MHz([](){ return new Ieee80211Htmcs(6, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs7BW20MHz([](){ return new Ieee80211Htmcs(7, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211HtCompliantCodes::htConvolutionalCode5_6, MHz(20));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs8BW20MHz([](){ return new Ieee80211Htmcs(8, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs9BW20MHz([](){ return new Ieee80211Htmcs(9, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs10BW20MHz([](){ return new Ieee80211Htmcs(10, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs11BW20MHz([](){ return new Ieee80211Htmcs(11, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs12BW20MHz([](){ return new Ieee80211Htmcs(12, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs13BW20MHz([](){ return new Ieee80211Htmcs(13, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs14BW20MHz([](){ return new Ieee80211Htmcs(14, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs15BW20MHz([](){ return new Ieee80211Htmcs(15, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211HtCompliantCodes::htConvolutionalCode5_6, MHz(20));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs16BW20MHz([](){ return new Ieee80211Htmcs(16, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs17BW20MHz([](){ return new Ieee80211Htmcs(17, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs18BW20MHz([](){ return new Ieee80211Htmcs(18, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs19BW20MHz([](){ return new Ieee80211Htmcs(19, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs20BW20MHz([](){ return new Ieee80211Htmcs(20, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs21BW20MHz([](){ return new Ieee80211Htmcs(21, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs22BW20MHz([](){ return new Ieee80211Htmcs(22, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs23BW20MHz([](){ return new Ieee80211Htmcs(23, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211HtCompliantCodes::htConvolutionalCode5_6, MHz(20));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs24BW20MHz([](){ return new Ieee80211Htmcs(24, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs25BW20MHz([](){ return new Ieee80211Htmcs(25, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs26BW20MHz([](){ return new Ieee80211Htmcs(26, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs27BW20MHz([](){ return new Ieee80211Htmcs(27, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs28BW20MHz([](){ return new Ieee80211Htmcs(28, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs29BW20MHz([](){ return new Ieee80211Htmcs(29, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs30BW20MHz([](){ return new Ieee80211Htmcs(30, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs31BW20MHz([](){ return new Ieee80211Htmcs(31, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211HtCompliantCodes::htConvolutionalCode5_6, MHz(20));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs0BW40MHz([](){ return new Ieee80211Htmcs(0, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs1BW40MHz([](){ return new Ieee80211Htmcs(1, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs2BW40MHz([](){ return new Ieee80211Htmcs(2, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs3BW40MHz([](){ return new Ieee80211Htmcs(3, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs4BW40MHz([](){ return new Ieee80211Htmcs(4, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs5BW40MHz([](){ return new Ieee80211Htmcs(5, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs6BW40MHz([](){ return new Ieee80211Htmcs(6, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs7BW40MHz([](){ return new Ieee80211Htmcs(7, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211HtCompliantCodes::htConvolutionalCode5_6, MHz(40));});


const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs8BW40MHz([](){ return new Ieee80211Htmcs(8, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs9BW40MHz([](){ return new Ieee80211Htmcs(9, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs10BW40MHz([](){ return new Ieee80211Htmcs(10, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs11BW40MHz([](){ return new Ieee80211Htmcs(11, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs12BW40MHz([](){ return new Ieee80211Htmcs(12, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs13BW40MHz([](){ return new Ieee80211Htmcs(13, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs14BW40MHz([](){ return new Ieee80211Htmcs(14, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs15BW40MHz([](){ return new Ieee80211Htmcs(15, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211HtCompliantCodes::htConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs16BW40MHz([](){ return new Ieee80211Htmcs(16, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs17BW40MHz([](){ return new Ieee80211Htmcs(17, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs18BW40MHz([](){ return new Ieee80211Htmcs(18, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs19BW40MHz([](){ return new Ieee80211Htmcs(19, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs20BW40MHz([](){ return new Ieee80211Htmcs(20, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs21BW40MHz([](){ return new Ieee80211Htmcs(21, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs22BW40MHz([](){ return new Ieee80211Htmcs(22, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs23BW40MHz([](){ return new Ieee80211Htmcs(23, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211HtCompliantCodes::htConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs24BW40MHz([](){ return new Ieee80211Htmcs(24, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs25BW40MHz([](){ return new Ieee80211Htmcs(25, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs26BW40MHz([](){ return new Ieee80211Htmcs(26, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs27BW40MHz([](){ return new Ieee80211Htmcs(27, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs28BW40MHz([](){ return new Ieee80211Htmcs(28, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs29BW40MHz([](){ return new Ieee80211Htmcs(29, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs30BW40MHz([](){ return new Ieee80211Htmcs(30, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs31BW40MHz([](){ return new Ieee80211Htmcs(31, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211HtCompliantCodes::htConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs32BW40MHz([](){ return new Ieee80211Htmcs(32, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantModulations::bpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs33BW20MHz([](){ return new Ieee80211Htmcs(33, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs34BW20MHz([](){ return new Ieee80211Htmcs(34, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs35BW20MHz([](){ return new Ieee80211Htmcs(35, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs36BW20MHz([](){ return new Ieee80211Htmcs(36, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs37BW20MHz([](){ return new Ieee80211Htmcs(37, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs38BW20MHz([](){ return new Ieee80211Htmcs(38, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs39BW20MHz([](){ return new Ieee80211Htmcs(39, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs40BW20MHz([](){ return new Ieee80211Htmcs(40, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs41BW20MHz([](){ return new Ieee80211Htmcs(41, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs42BW20MHz([](){ return new Ieee80211Htmcs(42, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs43BW20MHz([](){ return new Ieee80211Htmcs(43, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs44BW20MHz([](){ return new Ieee80211Htmcs(44, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs45BW20MHz([](){ return new Ieee80211Htmcs(45, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs46BW20MHz([](){ return new Ieee80211Htmcs(46, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs47BW20MHz([](){ return new Ieee80211Htmcs(47, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs48BW20MHz([](){ return new Ieee80211Htmcs(48, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs49BW20MHz([](){ return new Ieee80211Htmcs(49, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs50BW20MHz([](){ return new Ieee80211Htmcs(50, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs51BW20MHz([](){ return new Ieee80211Htmcs(51, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs52BW20MHz([](){ return new Ieee80211Htmcs(52, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});


const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs53BW20MHz([](){ return new Ieee80211Htmcs(53, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs54BW20MHz([](){ return new Ieee80211Htmcs(54, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs55BW20MHz([](){ return new Ieee80211Htmcs(55, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs56BW20MHz([](){ return new Ieee80211Htmcs(56, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs57BW20MHz([](){ return new Ieee80211Htmcs(57, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs58BW20MHz([](){ return new Ieee80211Htmcs(58, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs59BW20MHz([](){ return new Ieee80211Htmcs(59, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs60BW20MHz([](){ return new Ieee80211Htmcs(60, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs61BW20MHz([](){ return new Ieee80211Htmcs(61, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs62BW20MHz([](){ return new Ieee80211Htmcs(62, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs63BW20MHz([](){ return new Ieee80211Htmcs(63, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs64BW20MHz([](){ return new Ieee80211Htmcs(64, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs65BW20MHz([](){ return new Ieee80211Htmcs(65, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs66BW20MHz([](){ return new Ieee80211Htmcs(66, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs67BW20MHz([](){ return new Ieee80211Htmcs(67, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs68BW20MHz([](){ return new Ieee80211Htmcs(68, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs69BW20MHz([](){ return new Ieee80211Htmcs(69, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs70BW20MHz([](){ return new Ieee80211Htmcs(70, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs71BW20MHz([](){ return new Ieee80211Htmcs(71, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs72BW20MHz([](){ return new Ieee80211Htmcs(72, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs73BW20MHz([](){ return new Ieee80211Htmcs(73, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs74BW20MHz([](){ return new Ieee80211Htmcs(74, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs75BW20MHz([](){ return new Ieee80211Htmcs(75, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs76BW20MHz([](){ return new Ieee80211Htmcs(76, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs33BW40MHz([](){ return new Ieee80211Htmcs(33, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs34BW40MHz([](){ return new Ieee80211Htmcs(34, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs35BW40MHz([](){ return new Ieee80211Htmcs(35, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs36BW40MHz([](){ return new Ieee80211Htmcs(36, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs37BW40MHz([](){ return new Ieee80211Htmcs(37, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs38BW40MHz([](){ return new Ieee80211Htmcs(38, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs39BW40MHz([](){ return new Ieee80211Htmcs(39, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs40BW40MHz([](){ return new Ieee80211Htmcs(40, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs41BW40MHz([](){ return new Ieee80211Htmcs(41, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs42BW40MHz([](){ return new Ieee80211Htmcs(42, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs43BW40MHz([](){ return new Ieee80211Htmcs(43, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs44BW40MHz([](){ return new Ieee80211Htmcs(44, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs45BW40MHz([](){ return new Ieee80211Htmcs(45, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs46BW40MHz([](){ return new Ieee80211Htmcs(46, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs47BW40MHz([](){ return new Ieee80211Htmcs(47, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs48BW40MHz([](){ return new Ieee80211Htmcs(48, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs49BW40MHz([](){ return new Ieee80211Htmcs(49, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs50BW40MHz([](){ return new Ieee80211Htmcs(50, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs51BW40MHz([](){ return new Ieee80211Htmcs(51, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs52BW40MHz([](){ return new Ieee80211Htmcs(52, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs53BW40MHz([](){ return new Ieee80211Htmcs(53, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs54BW40MHz([](){ return new Ieee80211Htmcs(54, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs55BW40MHz([](){ return new Ieee80211Htmcs(55, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs56BW40MHz([](){ return new Ieee80211Htmcs(56, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs57BW40MHz([](){ return new Ieee80211Htmcs(57, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs58BW40MHz([](){ return new Ieee80211Htmcs(58, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs59BW40MHz([](){ return new Ieee80211Htmcs(59, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs60BW40MHz([](){ return new Ieee80211Htmcs(60, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs61BW40MHz([](){ return new Ieee80211Htmcs(61, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs62BW40MHz([](){ return new Ieee80211Htmcs(62, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs63BW40MHz([](){ return new Ieee80211Htmcs(63, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs64BW40MHz([](){ return new Ieee80211Htmcs(64, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs65BW40MHz([](){ return new Ieee80211Htmcs(65, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs66BW40MHz([](){ return new Ieee80211Htmcs(66, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs67BW40MHz([](){ return new Ieee80211Htmcs(67, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs68BW40MHz([](){ return new Ieee80211Htmcs(68, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs69BW40MHz([](){ return new Ieee80211Htmcs(69, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs70BW40MHz([](){ return new Ieee80211Htmcs(70, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs71BW40MHz([](){ return new Ieee80211Htmcs(71, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs72BW40MHz([](){ return new Ieee80211Htmcs(72, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs73BW40MHz([](){ return new Ieee80211Htmcs(73, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs74BW40MHz([](){ return new Ieee80211Htmcs(74, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs75BW40MHz([](){ return new Ieee80211Htmcs(75, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qpskModulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

const DI<Ieee80211Htmcs> Ieee80211HtmcsTable::htMcs76BW40MHz([](){ return new Ieee80211Htmcs(76, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam64Modulation, &Ieee80211OfdmCompliantModulations::qam16Modulation, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});

} /* namespace physicallayer */
} /* namespace inet */
