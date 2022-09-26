//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/StringFormat.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredDimensionalAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/LayeredReceptionResult.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/wireless/common/signal/Interference.h"
#include "inet/physicallayer/wireless/common/errormodel/packetlevel/NeuralNetworkErrorModelTrainingDatasetGenerator.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {

Define_Module(NeuralNetworkErrorModelTrainingDatasetGenerator);

static std::string getModulationName(const IModulation *modulation)
{
    std::string s = modulation->getClassName();
    int prefixLength = strlen("inet::physicallayer::");
    return s.substr(prefixLength, s.length() - prefixLength - 10);
}

void NeuralNetworkErrorModelTrainingDatasetGenerator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        packetNameFormat = par("filenameFormat");
        packetCount = par("packetCount");
        repeatCount = par("repeatCount");
        radio = check_and_cast<const Radio *>(getModuleByPath(par("radioModule")));
        radioMedium = check_and_cast<const IRadioMedium *>(getModuleByPath("radioMedium"));
        openTrainingDataset();
    }
    else if (stage == INITSTAGE_LAST) {
        generateTrainingDataset();
        closeTrainingDataset();
    }
}

void NeuralNetworkErrorModelTrainingDatasetGenerator::openTrainingDataset()
{
    auto transmittedPacket = new Packet(nullptr, makeShared<ByteCountChunk>(B(1)));
    transmittedPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    radio->encapsulate(transmittedPacket);
    auto transmission = check_and_cast<const LayeredTransmission *>(radio->getTransmitter()->createTransmission(radio, transmittedPacket, 0));
    auto bitrate = transmission->getBitModel()->getDataGrossBitrate();
    if (auto forwardErrorCorrection = transmission->getBitModel()->getForwardErrorCorrection())
        bitrate *= forwardErrorCorrection->getCodeRate();
    auto modulation = transmission->getSymbolModel()->getDataModulation();
    auto ofdmModulation = dynamic_cast<const Ieee80211OfdmModulation *>(modulation);
    auto subcarrierModulation = ofdmModulation != nullptr ? ofdmModulation->getSubcarrierModulation() : nullptr;
    auto narrowbandSignal = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    auto centerFrequency = narrowbandSignal->getCenterFrequency();
    auto bandwidth = narrowbandSignal->getBandwidth();
    std::string filename = StringFormat::formatString(packetNameFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'c':
                result = std::string(strrchr(radio->getClassName(), ':') + 1);
                break;
            case 'r':
                result = bitrate.str();
                break;
            case 'm':
                result = getModulationName(modulation);
                break;
            case 'M':
                result = getModulationName(subcarrierModulation);
                break;
            case 'l':
                result = par("packetLength").str();
                break;
            case 'f':
                result = centerFrequency.str();
                break;
            case 'b':
                result = bandwidth.str();
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    delete transmittedPacket;
    filename.erase(std::remove_if(filename.begin(), filename.end(), isspace), filename.end());
    std::cout << "Opening training dataset file '" << filename << "'" << std::endl;
    inet::utils::makePathForFile(filename.c_str());
    traningDataset.open(filename.c_str(), std::ios::out | std::ios::binary);
    if (!traningDataset.is_open())
        throw cRuntimeError("Cannot open file %s", filename.c_str());
}

void NeuralNetworkErrorModelTrainingDatasetGenerator::closeTrainingDataset()
{
    if (traningDataset.is_open())
        traningDataset.close();
}

void NeuralNetworkErrorModelTrainingDatasetGenerator::generateTrainingDataset()
{
    // TODO: separate preamble, header and data parts for generating the noise and the training data
    std::cout << "Generating training dataset, count = " << packetCount << std::endl;
    traningDataset << "# index, packetErrorRate, backgroundNoisePowerMean, backgroundNoisePowerStddev, numInterferingSignals, meanInterferingSignalNoisePowerMean, meanInterferingSignalNoisePowerStddev, bitrate, packetLength, modulation, subcarrierModulation, centerFrequency, bandwidth, timeDivision, frequencyDivision, numSymbols, preambleDuration, headerDuration, dataDuration, duration, packetByte+, symbolSnirMean+" << std::endl;
    auto analogModel = check_and_cast<const LayeredDimensionalAnalogModel *>(radioMedium->getAnalogModel());
    auto propagation = radioMedium->getPropagation();
    auto transmitter = radio->getTransmitter();
    auto receiver = radio->getReceiver();
    for (int packetIndex = 0; packetIndex < packetCount; packetIndex++) {
        B packetLength = B(par("packetLength"));
        W backgroundNoisePowerMean = W(par("backgroundNoisePowerMean"));
        W backgroundNoisePowerStddev = W(par("backgroundNoisePowerStddev"));
        int numInterferingSignals = par("numInterferingSignals");
        // create packet
        std::vector<uint8_t> bytes;
        for (B i = B(0); i < packetLength; i++)
            bytes.push_back(uniform(0, 255));
        auto data = makeShared<BytesChunk>(bytes);
        auto transmittedPacket = new Packet(nullptr, data);
        transmittedPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
        // adds the following:
        //  - tail bits
        //  - service bits
        //  - plus padding to whole ofdm symbol
        //  - one extra ofdm symbol for phy header
        radio->encapsulate(transmittedPacket);
        // create transmission
        simtime_t startTime = 0;
        auto transmission = check_and_cast<const LayeredTransmission *>(transmitter->createTransmission(radio, transmittedPacket, startTime));
        auto endTime = transmission->getEndTime();
        auto transmissionAnalogModel = transmission->getAnalogModel();
        auto preambleDuration = transmissionAnalogModel->getPreambleDuration();
        auto headerDuration = transmissionAnalogModel->getHeaderDuration();
        auto dataDuration = transmissionAnalogModel->getDataDuration();
        auto duration = transmissionAnalogModel->getDuration();
        auto bitrate = transmission->getBitModel()->getDataGrossBitrate();
        if (auto forwardErrorCorrection = transmission->getBitModel()->getForwardErrorCorrection())
            bitrate *= forwardErrorCorrection->getCodeRate();
        auto modulation = transmission->getSymbolModel()->getDataModulation();
        auto ofdmModulation = dynamic_cast<const Ieee80211OfdmModulation *>(modulation);
        auto subcarrierModulation = ofdmModulation != nullptr ? ofdmModulation->getSubcarrierModulation() : nullptr;
        auto transmittedSymbols = transmission->getSymbolModel()->getAllSymbols();
        // TODO: time division doesn't take into account that the preamble doesn't contain symbols
        int timeDivision = transmittedSymbols->size();
        int frequencyDivision = ofdmModulation != nullptr ? ofdmModulation->getNumSubcarriers() : 1;
        if (frequencyDivision != 52)
            throw cRuntimeError("wrong number of OFDM subcarriers!");
        int numSymbols = timeDivision * frequencyDivision;
        // create reception
        auto arrival = propagation->computeArrival(transmission, radio->getAntenna()->getMobility());
        auto reception = check_and_cast<const LayeredReception *>(analogModel->computeReception(radio, transmission, arrival));
        // create noise
        auto narrowbandSignal = check_and_cast<const INarrowbandSignal *>(reception->getAnalogModel());
        auto centerFrequency = narrowbandSignal->getCenterFrequency();
        auto bandwidth = narrowbandSignal->getBandwidth();
        auto startFrequency = centerFrequency - bandwidth / 2;
        auto endFrequency = centerFrequency + bandwidth / 2;
        auto backgroundNoisePowerFunction = createNoisePowerFunction(backgroundNoisePowerMean, backgroundNoisePowerStddev, frequencyDivision, timeDivision, startTime, endTime, startFrequency, endFrequency);
        std::vector<Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>> noisePowerFunctions;
        noisePowerFunctions.push_back(backgroundNoisePowerFunction);
        W meanInterferingSignalNoisePowerMean = W(0);
        W meanInterferingSignalNoisePowerStddev = W(0);
        for (int i = 0; i < numInterferingSignals; i++) {
            W interferingSignalNoisePowerMean = W(par("interferingSignalNoisePowerMean"));
            W interferingSignalNoisePowerStddev = W(par("interferingSignalNoisePowerStddev"));
            meanInterferingSignalNoisePowerMean += interferingSignalNoisePowerMean;
            meanInterferingSignalNoisePowerStddev += interferingSignalNoisePowerStddev;
            int interferingSignalTimeDivision = intuniform(1, timeDivision);
            int interferingSignalFrequencyDivision = intuniform(1, frequencyDivision);
            simtime_t interferingSignalStartTime = startTime + duration * intuniform(1 - interferingSignalTimeDivision, timeDivision - 1) / timeDivision;
            simtime_t interferingSignalEndTime = interferingSignalStartTime + duration / timeDivision * interferingSignalTimeDivision;
            Hz interferingSignalStartFrequency = startFrequency + bandwidth * intuniform(1 - interferingSignalFrequencyDivision, frequencyDivision - 1) / frequencyDivision;
            Hz interferingSignalEndFrequency = interferingSignalStartFrequency + bandwidth / frequencyDivision * interferingSignalFrequencyDivision;
            auto interferingSignalNoisePowerFunction = createNoisePowerFunction(interferingSignalNoisePowerMean, interferingSignalNoisePowerStddev, interferingSignalFrequencyDivision, interferingSignalTimeDivision, interferingSignalStartTime, interferingSignalEndTime, interferingSignalStartFrequency, interferingSignalEndFrequency);
            noisePowerFunctions.push_back(interferingSignalNoisePowerFunction);
        }
        meanInterferingSignalNoisePowerMean /= numInterferingSignals;
        meanInterferingSignalNoisePowerStddev /= numInterferingSignals;
        auto noisePowerFunction = makeShared<SummedFunction<WpHz, Domain<simsec, Hz>>>(noisePowerFunctions);
        auto noise = new DimensionalNoise(startTime, endTime, centerFrequency, bandwidth, noisePowerFunction);
        // create snir
        auto signalAnalogModel = check_and_cast<const DimensionalReceptionSignalAnalogModel *>(reception->getAnalogModel());
        auto receptionPowerFunction = signalAnalogModel->getPower();
        auto snirFunction = receptionPowerFunction->divide(noisePowerFunction);
        auto listening = receiver->createListening(nullptr, startTime, endTime, Coord::ZERO, Coord::ZERO);
        auto interference = new Interference(noise, new std::vector<const IReception *>());
        const ISnir *snir = new LayeredSnir(reception, noise);
        int receptionSuccessfulCount = 0;
        for (int i = 0; i < repeatCount; i++) {
            auto decisions = new std::vector<const IReceptionDecision *>();
            auto receptionResult = receiver->computeReceptionResult(listening, reception, interference, snir, decisions);
            auto receivedPacket = receptionResult->getPacket();
            bool isReceptionSuccessful = true;
            if (receivedPacket->getTotalLength() != transmittedPacket->getTotalLength())
                isReceptionSuccessful = false;
            else {
                auto transmittedData = transmittedPacket->peekAllAsBytes();
                auto receivedData = receivedPacket->peekAllAsBytes();
                // TODO: this is not a good way to compare the data:
                // - the physical header has a parity bit (covering only the header, not the data)
                //    - that should be checked first
                // - should not compare the padding bytes, the MAC doesn't care about that
                //    - we assume that the MAC-level FCS is accurate (detects corruption iff corruption happened)
                for (int j = 0; j < receivedPacket->getByteLength(); j++) {
                    if (receivedData->getBytes()[j] != transmittedData->getBytes()[j]) {
                        isReceptionSuccessful = false;
                        break;
                    }
                }
            }
            if (isReceptionSuccessful)
                receptionSuccessfulCount++;
            delete receptionResult;
        }
        // print parameters
        double packetErrorRate = (1 - (double)receptionSuccessfulCount / repeatCount);
        traningDataset << (int)packetIndex << ", " << packetErrorRate << ", " << backgroundNoisePowerMean << ", " << backgroundNoisePowerStddev << ", " << numInterferingSignals << ", " << meanInterferingSignalNoisePowerMean << ", " << meanInterferingSignalNoisePowerStddev << ", " << bitrate << ", " << transmittedPacket->getTotalLength() << ", " << getModulationName(modulation) << ", " << (subcarrierModulation != nullptr ? getModulationName(subcarrierModulation) : "NA") << ", " << centerFrequency << ", " << bandwidth << ", " << timeDivision << ", " << frequencyDivision << ", " << numSymbols << ", "  << preambleDuration << ", " << headerDuration << ", " << dataDuration << ", " << duration << ", ";
        // print packet bytes
        auto transmittedData = transmittedPacket->peekAllAsBytes();
        for (auto byte : transmittedData->getBytes())
            traningDataset << (int)byte << ", ";
        // print symbol SNIR means
        double snirMean = 0;
        startTime += preambleDuration;
        for (int i = 0; i < timeDivision; i++) {
            for (int j = 0; j < frequencyDivision; j++) {
                simtime_t symbolStartTime = (startTime * (timeDivision - i) + endTime * i) / timeDivision;
                //std::cout << symbolStartTime << std::endl;
                simtime_t symbolEndTime = (startTime * (timeDivision - i - 1) + endTime * (i + 1)) / timeDivision;
                Hz symbolStartFrequency = (startFrequency * (frequencyDivision - j) + endFrequency * j) / frequencyDivision;
                Hz symbolEndFrequency = (startFrequency * (frequencyDivision - j - 1) + endFrequency * (j + 1)) / frequencyDivision;
                Point<simsec, Hz> startPoint(simsec(symbolStartTime), symbolStartFrequency);
                Point<simsec, Hz> endPoint(simsec(symbolEndTime), symbolEndFrequency);
                Interval<simsec, Hz> interval(startPoint, endPoint, 0b11, 0b00, 0b00);
                double symbolSnirMean = snirFunction->getMean(interval);
                snirMean += symbolSnirMean;
                traningDataset << symbolSnirMean << ", ";
            }
        }
        snirMean /= numSymbols;
        std::cout << "Generating line: index = " << packetIndex << ", packetErrorRate = " << packetErrorRate << ", packetLength = " << packetLength << ", meanSnir = " << snirMean << ", backgroundNoisePowerMean = " << backgroundNoisePowerMean << ", backgroundNoisePowerStddev = " << backgroundNoisePowerStddev << ", numInterferingSignals = " << numInterferingSignals << ", meanInterferingSignalNoisePowerMean = " << meanInterferingSignalNoisePowerMean << ", meanInterferingSignalNoisePowerStddev = " << meanInterferingSignalNoisePowerStddev << std::endl;
        traningDataset << std::endl;
        delete snir;
        delete listening;
        delete interference;
        delete arrival;
        delete reception;
        delete transmission;
        delete transmittedPacket;
    }
}

Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> NeuralNetworkErrorModelTrainingDatasetGenerator::createNoisePowerFunction(W noisePowerMean, W noisePowerStddev, int frequencyDivision, int timeDivision, simtime_t startTime, simtime_t endTime, Hz startFrequency, Hz endFrequency)
{
    auto bandwidth = endFrequency - startFrequency;
    Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> noisePowerFunction;
    if (frequencyDivision == 1) {
        std::vector<WpHz> rs;
        for (int i = 0; i < timeDivision + 1; i++) {
            WpHz power = W(truncnormal(noisePowerMean.get(), noisePowerStddev.get())) / bandwidth;
            rs.push_back(power);
        }
        auto timeGainFunction = makeShared<PeriodicallyInterpolated1DFunction<WpHz, simsec>>(simsec(startTime), simsec(endTime), AverageInterpolator<simsec, WpHz>::singleton, rs);
        auto frequencyGainFunction = makeShared<Boxcar1DFunction<double, Hz>>(startFrequency, endFrequency, 1);
        noisePowerFunction = makeShared<Combined2DFunction<WpHz, simsec, Hz>>(timeGainFunction, frequencyGainFunction);
    }
    else {
        std::vector<WpHz> rs;
        for (int i = 0; i < timeDivision + 1; i++) {
            for (int j = 0; j < frequencyDivision + 1; j++) {
                WpHz power = W(truncnormal(noisePowerMean.get(), noisePowerStddev.get())) / bandwidth * frequencyDivision;
                rs.push_back(power);
            }
        }
        noisePowerFunction = makeShared<PeriodicallyInterpolated2DFunction<WpHz, simsec, Hz>>(
                simsec(startTime), simsec(endTime), timeDivision + 1, startFrequency, endFrequency, frequencyDivision + 1,
                AverageInterpolator<simsec, WpHz>::singleton,
                AverageInterpolator<Hz, WpHz>::singleton, rs);
    }
    return noisePowerFunction;
}

} // namespace physicallayer
} // namespace inet

