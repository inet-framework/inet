//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h"

#include <tuple>

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtCode.h"
#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam256Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QbpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"

namespace inet {
namespace physicallayer {

OPP_THREAD_LOCAL const Ieee80211VhtCompliantModes Ieee80211VhtCompliantModes::singleton;

Ieee80211VhtMode::Ieee80211VhtMode(const char *name, const Ieee80211VhtPreambleMode *preambleMode, const Ieee80211VhtDataMode *dataMode, const BandMode centerFrequencyMode) :
    Ieee80211ModeBase(name),
    preambleMode(preambleMode),
    dataMode(dataMode),
    centerFrequencyMode(centerFrequencyMode)
{
}

Ieee80211VhtModeBase::Ieee80211VhtModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType) :
    bandwidth(bandwidth),
    guardIntervalType(guardIntervalType),
    mcsIndex(modulationAndCodingScheme),
    numberOfSpatialStreams(numberOfSpatialStreams),
    netBitrate(bps(NaN)),
    grossBitrate(bps(NaN))
{
}

Ieee80211VhtPreambleMode::Ieee80211VhtPreambleMode(const Ieee80211VhtSignalMode *highThroughputSignalMode, const Ieee80211OfdmSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream) :
    highThroughputSignalMode(highThroughputSignalMode),
    legacySignalMode(legacySignalMode),
    preambleFormat(preambleFormat),
    numberOfHTLongTrainings(computeNumberOfHTLongTrainings(computeNumberOfSpaceTimeStreams(numberOfSpatialStream)))
{
}

Ieee80211VhtSignalMode::Ieee80211VhtSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OfdmModulation *modulation, const Ieee80211VhtCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType) :
    Ieee80211VhtModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType),
    modulation(modulation),
    code(code)
{
}

Ieee80211VhtSignalMode::Ieee80211VhtSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OfdmModulation *modulation, const Ieee80211ConvolutionalCode *convolutionalCode, const Hz bandwidth, GuardIntervalType guardIntervalType) :
    Ieee80211VhtModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType),
    modulation(modulation),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, modulation, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, bandwidth, false))
{
}

Ieee80211VhtDataMode::Ieee80211VhtDataMode(const Ieee80211Vhtmcs *modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType) :
    Ieee80211VhtModeBase(modulationAndCodingScheme->getMcsIndex(), computeNumberOfSpatialStreams(modulationAndCodingScheme), bandwidth, guardIntervalType),
    modulationAndCodingScheme(modulationAndCodingScheme),
    numberOfBccEncoders(computeNumberOfBccEncoders())
{
}

Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const ApskModulationBase *stream2SubcarrierModulation, const ApskModulationBase *stream3SubcarrierModulation, const ApskModulationBase *stream4SubcarrierModulation, const ApskModulationBase *stream5SubcarrierModulation, const ApskModulationBase *stream6SubcarrierModulation, const ApskModulationBase *stream7SubcarrierModulation, const ApskModulationBase *stream8SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
     mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream2SubcarrierModulation)),
    stream3Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream3SubcarrierModulation)),
    stream4Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream4SubcarrierModulation)),
    stream5Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream5SubcarrierModulation)),
    stream6Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream6SubcarrierModulation)),
    stream7Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream7SubcarrierModulation)),
    stream8Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream8SubcarrierModulation)),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const ApskModulationBase *stream2SubcarrierModulation, const ApskModulationBase *stream3SubcarrierModulation, const ApskModulationBase *stream4SubcarrierModulation, const ApskModulationBase *stream5SubcarrierModulation, const ApskModulationBase *stream6SubcarrierModulation, const ApskModulationBase *stream7SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream2SubcarrierModulation)),
    stream3Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream3SubcarrierModulation)),
    stream4Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream4SubcarrierModulation)),
    stream5Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream5SubcarrierModulation)),
    stream6Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream6SubcarrierModulation)),
    stream7Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream7SubcarrierModulation)),
    stream8Modulation(nullptr),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const ApskModulationBase *stream2SubcarrierModulation, const ApskModulationBase *stream3SubcarrierModulation, const ApskModulationBase *stream4SubcarrierModulation, const ApskModulationBase *stream5SubcarrierModulation, const ApskModulationBase *stream6SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream2SubcarrierModulation)),
    stream3Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream3SubcarrierModulation)),
    stream4Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream4SubcarrierModulation)),
    stream5Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream5SubcarrierModulation)),
    stream6Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream6SubcarrierModulation)),
    stream7Modulation(nullptr),
    stream8Modulation(nullptr),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const ApskModulationBase *stream2SubcarrierModulation, const ApskModulationBase *stream3SubcarrierModulation, const ApskModulationBase *stream4SubcarrierModulation, const ApskModulationBase *stream5SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
     mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream2SubcarrierModulation)),
    stream3Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream3SubcarrierModulation)),
    stream4Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream4SubcarrierModulation)),
    stream5Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream5SubcarrierModulation)),
    stream6Modulation(nullptr),
    stream7Modulation(nullptr),
    stream8Modulation(nullptr),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const ApskModulationBase *stream2SubcarrierModulation, const ApskModulationBase *stream3SubcarrierModulation, const ApskModulationBase *stream4SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
     mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream2SubcarrierModulation)),
    stream3Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream3SubcarrierModulation)),
    stream4Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream4SubcarrierModulation)),
    stream5Modulation(nullptr),
    stream6Modulation(nullptr),
    stream7Modulation(nullptr),
    stream8Modulation(nullptr),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const ApskModulationBase *stream2SubcarrierModulation, const ApskModulationBase *stream3SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream2SubcarrierModulation)),
    stream3Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream3SubcarrierModulation)),
    stream4Modulation(nullptr),
    stream5Modulation(nullptr),
    stream6Modulation(nullptr),
    stream7Modulation(nullptr),
    stream8Modulation(nullptr),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}


Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const ApskModulationBase *stream2SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream2SubcarrierModulation)),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    stream5Modulation(nullptr),
    stream6Modulation(nullptr),
    stream7Modulation(nullptr),
    stream8Modulation(nullptr),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(nullptr),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    stream5Modulation(nullptr),
    stream6Modulation(nullptr),
    stream7Modulation(nullptr),
    stream8Modulation(nullptr),
    code(Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}


Ieee80211Vhtmcs::Ieee80211Vhtmcs(unsigned int mcsIndex, const ApskModulationBase *stream1SubcarrierModulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth, int nss) :
    mcsIndex(mcsIndex),
    stream1Modulation(new Ieee80211OfdmModulation(getNumberOfTotalSubcarriers(bandwidth), stream1SubcarrierModulation)),
    stream2Modulation(nullptr),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    stream5Modulation(nullptr),
    stream6Modulation(nullptr),
    stream7Modulation(nullptr),
    stream8Modulation(nullptr),
    bandwidth(bandwidth)
{
    if (nss > 1)
        stream2Modulation = stream1Modulation;
    if (nss > 2)
        stream3Modulation = stream1Modulation;
    if (nss > 3)
        stream4Modulation = stream1Modulation;
    if (nss > 4)
        stream5Modulation = stream1Modulation;
    if (nss > 5)
        stream6Modulation = stream1Modulation;
    if (nss > 6)
        stream7Modulation = stream1Modulation;
    if (nss > 7)
        stream8Modulation = stream1Modulation;
    code = Ieee80211VhtCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, stream5Modulation, stream6Modulation, stream7Modulation, stream8Modulation, bandwidth);
}

const simtime_t Ieee80211VhtPreambleMode::getFirstHTLongTrainingFieldDuration() const
{
    return simtime_t(8E-6);
}

unsigned int Ieee80211VhtPreambleMode::computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const
{
    // Table 20-12—Determining the number of space-time streams
    return numberOfSpatialStreams + highThroughputSignalMode->getSTBC();
}

unsigned int Ieee80211VhtPreambleMode::computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const
{
    // If the transmitter is providing training for exactly the space-time
    // streams (spatial mapper inputs) used for the transmission of the PSDU,
    // the number of training symbols, N_LTF, is equal to the number of space-time
    // streams, N STS, except that for three space-time streams, four training symbols
    // are required.
    return numberOfSpaceTimeStreams == 3 ? 4 : numberOfSpaceTimeStreams;
}

const simtime_t Ieee80211VhtPreambleMode::getDuration() const
{
    // 21.3.4 Mathematical description of signals
    simtime_t sumOfHTLTFs = getSecondAndSubsequentHTLongTrainingFielDuration() * numberOfHTLongTrainings;
    return getNonHTShortTrainingSequenceDuration() + getNonHTLongTrainingFieldDuration() + getLSIGDuration() + getVHTSignalFieldA() + getVHTShortTrainingFieldDuration() + sumOfHTLTFs + getVHTSignalFieldB();
}

bps Ieee80211VhtSignalMode::computeGrossBitrate() const
{
    unsigned int numberOfCodedBitsPerSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    if (guardIntervalType == HT_GUARD_INTERVAL_LONG)
        return bps(numberOfCodedBitsPerSymbol / getSymbolInterval());
    else if (guardIntervalType == HT_GUARD_INTERVAL_SHORT)
        return bps(numberOfCodedBitsPerSymbol / getShortGISymbolInterval());
    else
        throw cRuntimeError("Unknown guard interval type");
}

bps Ieee80211VhtSignalMode::computeNetBitrate() const
{
    return computeGrossBitrate() * code->getForwardErrorCorrection()->getCodeRate();
}

b Ieee80211VhtSignalMode::getLength() const
{
    return getMCSLength() +
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

bps Ieee80211VhtDataMode::computeGrossBitrate() const
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

bps Ieee80211VhtDataMode::computeNetBitrate() const
{
    return getGrossBitrate() * getCode()->getForwardErrorCorrection()->getCodeRate();
}

bps Ieee80211VhtModeBase::getNetBitrate() const
{
    if (std::isnan(netBitrate.get()))
        netBitrate = computeNetBitrate();
    return netBitrate;
}

bps Ieee80211VhtModeBase::getGrossBitrate() const
{
    if (std::isnan(grossBitrate.get()))
        grossBitrate = computeGrossBitrate();
    return grossBitrate;
}

int Ieee80211VhtModeBase::getNumberOfDataSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 52;
    else if (bandwidth == MHz(40))
        return 108;
    else if (bandwidth == MHz(80))
        return 234;
    else if (bandwidth == MHz(160))
        return 468;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

int Ieee80211VhtModeBase::getNumberOfPilotSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 4;
    else if (bandwidth == MHz(40))
        // It is a spacial case, see the comment above.
        return 6;
    else if (bandwidth == MHz(80))
        // It is a spacial case, see the comment above.
        return 8;
    else if (bandwidth == MHz(160))
        // It is a spacial case, see the comment above.
        return 16;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

b Ieee80211VhtDataMode::getCompleteLength(b dataLength) const
{
    return getServiceFieldLength() + getTailFieldLength() + dataLength; // TODO padding?
}

unsigned int Ieee80211VhtDataMode::computeNumberOfSpatialStreams(const Ieee80211Vhtmcs *vhtmcs) const
{
    if (vhtmcs == nullptr)
        throw cRuntimeError("Invalid MCS mode");
    return vhtmcs->getNumNss();
}

unsigned int Ieee80211VhtDataMode::computeNumberOfCodedBitsPerSubcarrierSum() const
{
    return (modulationAndCodingScheme->getModulation() ? modulationAndCodingScheme->getModulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
           (modulationAndCodingScheme->getStreamExtension1Modulation() ? modulationAndCodingScheme->getStreamExtension1Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
           (modulationAndCodingScheme->getStreamExtension2Modulation() ? modulationAndCodingScheme->getStreamExtension2Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
           (modulationAndCodingScheme->getStreamExtension3Modulation() ? modulationAndCodingScheme->getStreamExtension3Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
           (modulationAndCodingScheme->getStreamExtension4Modulation() ? modulationAndCodingScheme->getStreamExtension4Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
           (modulationAndCodingScheme->getStreamExtension5Modulation() ? modulationAndCodingScheme->getStreamExtension5Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
           (modulationAndCodingScheme->getStreamExtension6Modulation() ? modulationAndCodingScheme->getStreamExtension6Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
           (modulationAndCodingScheme->getStreamExtension7Modulation() ? modulationAndCodingScheme->getStreamExtension7Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0);
}

unsigned int Ieee80211VhtDataMode::getNumberOfBccEncoders20MHz() const
{
    if (getNumberOfSpatialStreams() < 7)
        return 1;
    else {
        if (getMcsIndex() > 7)
            return 2;
        else
            return 1;
    }
}

unsigned int Ieee80211VhtDataMode::getNumberOfBccEncoders40MHz() const
{
    switch (getNumberOfSpatialStreams()) {
        case 1:
        case 2:
        case 3:
            return 1;
            break;
        case 4:
            if (getMcsIndex() > 7)
                return 2;
            else
                return 1;
            break;
        case 5:
            if (getMcsIndex() > 5)
                return 2;
            else
                return 1;

            break;
        case 6:
            if (getMcsIndex() > 4)
                return 2;
            else
                return 1;
            break;
        case 7:
        case 8:
            if (getMcsIndex() < 4)
                return 1;
            else if (getMcsIndex() < 8)
                return 2;
            else
                return 3;
            break;
        default:
            throw cRuntimeError("Invalid MCS mode");
    }
}

unsigned int Ieee80211VhtDataMode::getNumberOfBccEncoders80MHz() const
{
    switch (getNumberOfSpatialStreams()) {
        case 1:
            return 1;
            break;
        case 2:
            if (getMcsIndex() > 6)
                return 2;
            else
                return 1;
            break;
        case 3:
            if (getMcsIndex() < 5)
                return 1;
            else if (getMcsIndex() < 9)
                return 2;
            else
                return 3;
            break;
        case 4:
            if (getMcsIndex() < 4)
                return 1;
            else if (getMcsIndex() < 7)
                return 2;
            else
                return 3;
            break;
        case 5:
            if (getMcsIndex() < 3)
                return 1;
            else if (getMcsIndex() < 5)
                return 2;
            else if (getMcsIndex() < 8)
                return 3;
            else
                return 4;
            break;
        case 6:
            if (getMcsIndex() < 3)
                return 1;
            else if (getMcsIndex() < 4)
                return 2;
            else if (getMcsIndex() < 7)
                return 3;
            else
                return 4;
            break;
        case 7:
            if (getMcsIndex() < 2)
                return 1;
            else if (getMcsIndex() == 3)
                return 2;
            else if (getMcsIndex() < 5)
                return 3;
            else if (getMcsIndex() == 5)
                return 4;
            else
                return 6;
            break;
        case 8:
            if (getMcsIndex() < 3)
                return 1;
            else if (getMcsIndex() < 4)
                return 2;
            else if (getMcsIndex() < 5)
                return 3;
            else if (getMcsIndex() < 7)
                return 4;
            else
                return 6;
            break;
        default:
            throw cRuntimeError("Invalid MCS mode");
    }
}

unsigned int Ieee80211VhtDataMode::getNumberOfBccEncoders160MHz() const
{
    switch (getNumberOfSpatialStreams()) {
        case 1:
            if (getMcsIndex() > 6)
                return 2;
            else
                return 1;
            break;
            break;
        case 2:
            if (getMcsIndex() < 4)
                return 1;
            else if (getMcsIndex() < 7)
                return 2;
            else
                return 3;
            break;
        case 3:
            if (getMcsIndex() < 3)
                return 1;
            else if (getMcsIndex() < 5)
                return 2;
            else if (getMcsIndex() < 7)
                return 3;
            else
                return 4;
            break;
        case 4:
            if (getMcsIndex() < 2)
                return 1;
            else if (getMcsIndex() < 4)
                return 2;
            else if (getMcsIndex() < 5)
                return 3;
            else if (getMcsIndex() < 7)
                return 4;
            else
                return 6;
            break;
        case 5:
            if (getMcsIndex() < 1)
                return 1;
            else if (getMcsIndex() < 3)
                return 2;
            else if (getMcsIndex() < 4)
                return 3;
            else if (getMcsIndex() < 5)
                return 4;
            else if (getMcsIndex() < 7)
                return 5;
            else if (getMcsIndex() < 8)
                return 6;
            else
                return 8;
            break;
        case 6:
            if (getMcsIndex() < 1)
                return 1;
            else if (getMcsIndex() < 3)
                return 2;
            else if (getMcsIndex() < 4)
                return 3;
            else if (getMcsIndex() < 5)
                return 4;
            else if (getMcsIndex() < 7)
                return 6;
            else if (getMcsIndex() < 9)
                return 8;
            else
                return 9;
            break;
        case 7:
            if (getMcsIndex() < 1)
                return 1;
            else if (getMcsIndex() < 2)
                return 2;
            else if (getMcsIndex() < 3)
                return 3;
            else if (getMcsIndex() < 4)
                return 4;
            else if (getMcsIndex() < 5)
                return 6;
            else if (getMcsIndex() < 7)
                return 7;
            else if (getMcsIndex() < 8)
                return 9;
            else
                return 12;
            break;
        case 8:
            if (getMcsIndex() < 1)
                return 1;
            else if (getMcsIndex() < 2)
                return 2;
            else if (getMcsIndex() < 3)
                return 3;
            else if (getMcsIndex() < 4)
                return 4;
            else if (getMcsIndex() < 5)
                return 6;
            else if (getMcsIndex() < 7)
                return 8;
            else if (getMcsIndex() < 8)
                return 9;
            else
                return 12;
        default:
            throw cRuntimeError("Invalid MCS mode");
    }
}

unsigned int Ieee80211VhtDataMode::computeNumberOfBccEncoders() const
{
    // When the BCC FEC encoder is used, a single encoder is used, except that two encoders
    // are used when the selected MCS has a PHY rate greater than 300 Mb/s (see 20.6).
    if (getBandwidth() == MHz(20)) {
        return getNumberOfBccEncoders20MHz();
    }
    else if (getBandwidth() == MHz(40)) {
        return getNumberOfBccEncoders40MHz();
    }
    else if (getBandwidth() == MHz(80)) {
        return getNumberOfBccEncoders80MHz();
    }
    else if (getBandwidth() == MHz(160)) {
        return getNumberOfBccEncoders160MHz();
    }
    else
        throw cRuntimeError("Invalid bandwidth evaluating  NumberOfBccEncoders");
}

const simtime_t Ieee80211VhtDataMode::getDuration(b dataLength) const
{
    unsigned int numberOfCodedBitsPerSubcarrierSum = computeNumberOfCodedBitsPerSubcarrierSum();
    unsigned int numberOfCodedBitsPerSymbol = numberOfCodedBitsPerSubcarrierSum * getNumberOfDataSubcarriers();
    const IForwardErrorCorrection *forwardErrorCorrection = getCode() ? getCode()->getForwardErrorCorrection() : nullptr;
    unsigned int dataBitsPerSymbol = forwardErrorCorrection ? forwardErrorCorrection->getDecodedLength(numberOfCodedBitsPerSymbol) : numberOfCodedBitsPerSymbol;
    int numberOfSymbols = lrint(ceil((double)getCompleteLength(dataLength).get() / dataBitsPerSymbol)); // TODO getBitLength(dataBitLength) should be divisible by dataBitsPerSymbol
    return numberOfSymbols * getSymbolInterval();
}

const simtime_t Ieee80211VhtMode::getSlotTime() const
{
    if (centerFrequencyMode == BAND_5GHZ)
        return 9E-6;
    else
        throw cRuntimeError("Unsupported carrier frequency");
}

const simtime_t Ieee80211VhtMode::getSifsTime() const
{
    if (centerFrequencyMode == BAND_5GHZ)
        return 16E-6;
    else
        throw cRuntimeError("Sifs time is not defined for this carrier frequency"); // TODO
}

const simtime_t Ieee80211VhtMode::getShortSlotTime() const
{
    return 9E-6;
}

Ieee80211VhtCompliantModes::Ieee80211VhtCompliantModes()
{
}

Ieee80211VhtCompliantModes::~Ieee80211VhtCompliantModes()
{
    for (auto& entry : modeCache)
        delete entry.second;
}

const Ieee80211VhtMode *Ieee80211VhtCompliantModes::getCompliantMode(const Ieee80211Vhtmcs *mcsMode, Ieee80211VhtMode::BandMode centerFrequencyMode, Ieee80211VhtPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211VhtModeBase::GuardIntervalType guardIntervalType)
{
    const char *name = ""; // TODO
    unsigned int nss = mcsMode->getNumNss();
    auto htModeId = std::make_tuple(mcsMode->getBandwidth(), mcsMode->getMcsIndex(), guardIntervalType, nss);
    auto mode = singleton.modeCache.find(htModeId);
    if (mode == singleton.modeCache.end()) {
        const Ieee80211OfdmSignalMode *legacySignal = nullptr;
        const Ieee80211VhtSignalMode *htSignal = nullptr;
        switch (preambleFormat) {
            case Ieee80211VhtPreambleMode::HT_PREAMBLE_GREENFIELD:
                htSignal = new Ieee80211VhtSignalMode(mcsMode->getMcsIndex(), &Ieee80211OfdmCompliantModulations::subcarriers52BpskModulation, Ieee80211VhtCompliantCodes::getCompliantCode(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OfdmCompliantModulations::subcarriers52BpskModulation, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mcsMode->getBandwidth(), false), mcsMode->getBandwidth(), guardIntervalType);
                break;
            case Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED:
                legacySignal = &Ieee80211OfdmCompliantModes::ofdmHeaderMode6MbpsRate13;
                htSignal = new Ieee80211VhtSignalMode(mcsMode->getMcsIndex(), &Ieee80211OfdmCompliantModulations::subcarriers52QbpskModulation, Ieee80211VhtCompliantCodes::getCompliantCode(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OfdmCompliantModulations::subcarriers52QbpskModulation, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mcsMode->getBandwidth(), false), mcsMode->getBandwidth(), guardIntervalType);
                break;
            default:
                throw cRuntimeError("Unknown preamble format");
        }
        const Ieee80211VhtDataMode *dataMode = new Ieee80211VhtDataMode(mcsMode, mcsMode->getBandwidth(), guardIntervalType);
        const Ieee80211VhtPreambleMode *preambleMode = new Ieee80211VhtPreambleMode(htSignal, legacySignal, preambleFormat, dataMode->getNumberOfSpatialStreams());
        const Ieee80211VhtMode *htMode = new Ieee80211VhtMode(name, preambleMode, dataMode, centerFrequencyMode);
        singleton.modeCache.insert(std::pair<std::tuple<Hz, unsigned int, Ieee80211VhtModeBase::GuardIntervalType, unsigned int>, const Ieee80211VhtMode *>(htModeId, htMode));
        return htMode;
    }
    return mode->second;
}

Ieee80211Vhtmcs::~Ieee80211Vhtmcs()
{
    delete code;
}

Ieee80211VhtSignalMode::~Ieee80211VhtSignalMode()
{
    delete code;
}

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss1([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss1([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss1([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss1([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss1([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss1([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss1([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss1([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss1([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss1([](){ return nullptr;});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss2([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss2([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss2([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss2([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss2([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss2([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss2([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss2([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss2([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss2([](){ return nullptr;});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss3([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss3([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss3([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss3([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss3([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss3([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss3([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss3([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss3([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss3([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 3);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss4([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss4([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss4([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss4([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss4([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss4([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss4([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss4([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss4([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss4([](){ return nullptr;});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss5([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss5([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss5([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss5([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss5([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss5([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss5([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss5([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss5([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss5([](){ return nullptr;});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss6([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss6([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss6([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss6([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss6([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss6([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss6([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss6([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss6([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss6([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 6);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss7([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss7([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss7([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss7([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss7([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss7([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss7([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss7([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss7([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss7([](){ return nullptr;});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss8([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss8([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss8([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss8([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss8([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss8([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss8([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss8([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss8([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(20), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss8([](){ return nullptr;});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss1([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss1([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss1([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss1([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss1([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss1([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss1([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss1([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss1([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss1([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 1);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss2([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss2([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss2([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss2([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss2([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss2([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss2([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss2([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss2([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss2([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 2);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss3([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss3([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss3([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss3([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss3([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss3([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss3([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss3([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss3([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss3([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 3);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss4([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss4([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss4([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss4([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss4([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss4([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss4([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss4([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss4([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss4([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 4);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss5([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss5([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss5([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss5([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss5([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss5([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss5([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss5([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss5([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss5([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 5);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss6([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss6([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss6([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss6([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss6([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss6([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss6([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss6([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss6([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss6([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 6);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss7([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss7([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss7([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss7([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss7([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss7([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss7([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss7([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss7([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss7([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 7);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss8([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss8([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss8([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss8([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss8([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss8([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss8([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss8([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss8([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(40), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss8([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(40), 8);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss1([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss1([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss1([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss1([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss1([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss1([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss1([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss1([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss1([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss1([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 1);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss2([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss2([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss2([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss2([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss2([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss2([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss2([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss2([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss2([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss2([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 2);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss3([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss3([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss3([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss3([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss3([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss3([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(80), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss3([](){ return nullptr;});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss3([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss3([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss3([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 3);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss4([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss4([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss4([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss4([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss4([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss4([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss4([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss4([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss4([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss4([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 4);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss5([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss5([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss5([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss5([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss5([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss5([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss5([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss5([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss5([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss5([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 5);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss6([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss6([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss6([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss6([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss6([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss6([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss6([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss6([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss6([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss6([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 6);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss7([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss7([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss7([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss7([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss7([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss7([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(80), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss7([](){ return nullptr;});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss7([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss7([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss7([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 7);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss8([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss8([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss8([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss8([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss8([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss8([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss8([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss8([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss8([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(80), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss8([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(80), 8);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss1([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss1([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss1([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss1([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss1([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss1([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss1([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss1([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss1([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 1);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss1([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 1);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss2([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss2([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss2([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss2([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss2([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss2([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss2([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss2([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss2([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 2);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss2([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 2);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss3([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss3([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss3([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss3([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss3([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss3([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss3([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss3([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss3([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 3);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss3([](){ return nullptr;});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss4([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss4([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss4([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss4([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss4([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss4([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss4([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss4([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss4([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 4);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss4([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 4);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss5([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss5([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss5([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss5([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss5([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss5([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss5([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss5([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss5([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 5);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss5([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 5);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss6([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss6([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss6([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss6([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss6([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss6([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss6([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss6([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss6([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 6);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss6([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 6);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss7([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss7([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss7([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss7([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss7([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss7([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss7([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss7([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss7([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 7);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss7([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 7);});

const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss8([](){ return new Ieee80211Vhtmcs(0, &BpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss8([](){ return new Ieee80211Vhtmcs(1, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss8([](){ return new Ieee80211Vhtmcs(2, &QpskModulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss8([](){ return new Ieee80211Vhtmcs(3, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss8([](){ return new Ieee80211Vhtmcs(4, &Qam16Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss8([](){ return new Ieee80211Vhtmcs(5, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss8([](){ return new Ieee80211Vhtmcs(6, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss8([](){ return new Ieee80211Vhtmcs(7, &Qam64Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss8([](){ return new Ieee80211Vhtmcs(8, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, MHz(160), 8);});
const DI<Ieee80211Vhtmcs> Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss8([](){ return new Ieee80211Vhtmcs(9, &Qam256Modulation::singleton, &Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, MHz(160), 8);});

} /* namespace physicallayer */
} /* namespace inet */

