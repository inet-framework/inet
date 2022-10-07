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

#include <string>
#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/StringFormat.h"
#include "inet/physicallayer/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"
#include "inet/physicallayer/analogmodel/bitlevel/LayeredDimensionalAnalogModel.h"
#include "inet/physicallayer/analogmodel/bitlevel/LayeredSnir.h"
#include "inet/physicallayer/common/bitlevel/LayeredReception.h"
#include "inet/physicallayer/common/bitlevel/LayeredReceptionResult.h"
#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/common/packetlevel/Interference.h"
#include "inet/physicallayer/errormodel/packetlevel/Ieee80211RadioErrorModelEvaluator.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211RadioErrorModelEvaluator);

static std::string getModulationName(const IModulation *modulation)
{
    std::string s = modulation->getClassName();
    int prefixLength = strlen("inet::physicallayer::");
    return s.substr(prefixLength, s.length() - prefixLength - 10);
}

void Ieee80211RadioErrorModelEvaluator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        repeatCount = par("repeatCount");
        radio = check_and_cast<const Radio *>(getModuleByPath(par("radioModule")));
        radioMedium = check_and_cast<const IRadioMedium *>(getModuleByPath("radioMedium"));
        openFiles();
    }
    else if (stage == INITSTAGE_LAST) {
        evaluateErrorModel();
        closeFiles();
    }
}

void Ieee80211RadioErrorModelEvaluator::openFiles()
{

    //auto transmittedPacket = new Packet(nullptr, makeShared<ByteCountChunk>(B(1)));
    //transmittedPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    //radio->encapsulate(transmittedPacket);
    //auto transmission = check_and_cast<const LayeredTransmission *>(radio->getTransmitter()->createTransmission(radio, transmittedPacket, 0));
    //auto bitrate = transmission->getBitModel()->getDataGrossBitrate();
    //if (auto forwardErrorCorrection = transmission->getBitModel()->getForwardErrorCorrection())
    //    bitrate *= forwardErrorCorrection->getCodeRate();
    //auto modulation = transmission->getSymbolModel()->getDataModulation();
    //auto ofdmModulation = dynamic_cast<const Ieee80211OfdmModulation *>(modulation);
    //auto subcarrierModulation = ofdmModulation != nullptr ? ofdmModulation->getSubcarrierModulation() : nullptr;
    //auto narrowbandSignal = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    //auto centerFrequency = narrowbandSignal->getCenterFrequency();
    //auto bandwidth = narrowbandSignal->getBandwidth();

    std::string snirsFilename = par("snirsFilename");
    snirsFilename.erase(std::remove_if(snirsFilename.begin(), snirsFilename.end(), isspace), snirsFilename.end());
    std::cout << "Opening snirs file '" << snirsFilename << "'" << std::endl;
    inet::utils::makePathForFile(snirsFilename.c_str());
    snirsFile.open(snirsFilename.c_str(), std::ios::in);
    if (!snirsFile.is_open())
        throw cRuntimeError("Cannot open file %s", snirsFilename.c_str());

    std::string persFilename = par("persFilename");
    persFilename.erase(std::remove_if(persFilename.begin(), persFilename.end(), isspace), persFilename.end());
    std::cout << "Opening pers file '" << persFilename << "'" << std::endl;
    inet::utils::makePathForFile(persFilename.c_str());
    persFile.open(persFilename.c_str(), std::ios::out | std::ios::trunc);
    if (!persFile.is_open())
        throw cRuntimeError("Cannot open file %s", persFilename.c_str());

    //delete transmittedPacket;
}

void Ieee80211RadioErrorModelEvaluator::closeFiles()
{
    if (snirsFile.is_open())
        snirsFile.close();
    if (persFile.is_open())
        persFile.close();
}

void Ieee80211RadioErrorModelEvaluator::evaluateErrorModel()
{
    // TODO: separate preamble, header and data parts for generating the noise and the training data
    std::cout << "Generating training dataset" << std::endl;
    // snirsFile << "# index, packetErrorRate, backgroundNoisePowerMean, backgroundNoisePowerStddev, numInterferingSignals, meanInterferingSignalNoisePowerMean, meanInterferingSignalNoisePowerStddev, bitrate, packetLength, modulation, subcarrierModulation, centerFrequency, bandwidth, timeDivision, frequencyDivision, numSymbols, preambleDuration, headerDuration, dataDuration, duration, packetByte+, symbolSnirMean+" << std::endl;
    if (auto analogModel = dynamic_cast<const LayeredDimensionalAnalogModel *>(radioMedium->getAnalogModel())) {
        auto propagation = radioMedium->getPropagation();
        auto transmitter = radio->getTransmitter();
        auto receiver = radio->getReceiver();

        std::string line;
        while (true) {
            std::getline(snirsFile, line);
            if (line.empty())
                break;

            std::vector<double> snirs;

            for (std::string s : cStringTokenizer(line.c_str(), ", ").asVector()) {
                if (!s.empty()) {
                    std::cout << s << std::endl;
                    snirs.push_back(1000000*atof(s.c_str()));
                }
            }

            ASSERT(snirs.size() % 52 == 0);


            B packetLength = B(100); // BIG TODO


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




            // TODO:
            timeDivision = snirs.size() / frequencyDivision;




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
            auto backgroundNoisePowerFunction = assembleNoisePowerFunction(snirs, frequencyDivision, timeDivision, startTime, endTime, startFrequency, endFrequency);
            std::vector<Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>> noisePowerFunctions;
            noisePowerFunctions.push_back(backgroundNoisePowerFunction);
            //W meanInterferingSignalNoisePowerMean = W(0);
            //W meanInterferingSignalNoisePowerStddev = W(0);
            //meanInterferingSignalNoisePowerMean /= numInterferingSignals;
            //meanInterferingSignalNoisePowerStddev /= numInterferingSignals;
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
            std::cout << "receiving..." << std::endl;
            for (int i = 0; i < repeatCount; i++) {
                auto decisions = new std::vector<const IReceptionDecision *>();
                std::cout << "comp rec res" << std::endl;
                auto receptionResult = receiver->computeReceptionResult(listening, reception, interference, snir, decisions);
                std::cout << "done comp rec res" << std::endl;
                auto receivedPacket = receptionResult->getPacket();
                bool isReceptionSuccessful = true;
                std::cout << "checking reception result..." << std::endl;
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
                std::cout << "succ? " << std::boolalpha << isReceptionSuccessful << std::endl;
                if (isReceptionSuccessful)
                    receptionSuccessfulCount++;
                delete receptionResult;
            }
            // print parameters
            double packetErrorRate = (1 - (double)receptionSuccessfulCount / repeatCount);
            persFile << packetErrorRate << std::endl;
            std::cout << "PER: " << packetErrorRate << std::endl;
            //snirsFile << (int)packetIndex << ", " << packetErrorRate << ", " << backgroundNoisePowerMean << ", " << backgroundNoisePowerStddev << ", " << numInterferingSignals << ", " << meanInterferingSignalNoisePowerMean << ", " << meanInterferingSignalNoisePowerStddev << ", " << bitrate << ", " << transmittedPacket->getTotalLength() << ", " << getModulationName(modulation) << ", " << (subcarrierModulation != nullptr ? getModulationName(subcarrierModulation) : "NA") << ", " << centerFrequency << ", " << bandwidth << ", " << timeDivision << ", " << frequencyDivision << ", " << numSymbols << ", "  << preambleDuration << ", " << headerDuration << ", " << dataDuration << ", " << duration << ", ";

            // print symbol SNIR means
            /*
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
                    snirsFile << symbolSnirMean << ", ";
                }
            }
            snirMean /= numSymbols;
            */
            //std::cout << "Generating line: index = " << packetIndex << ", packetErrorRate = " << packetErrorRate << ", packetLength = " << packetLength << ", meanSnir = " << snirMean << ", backgroundNoisePowerMean = " << backgroundNoisePowerMean << ", backgroundNoisePowerStddev = " << backgroundNoisePowerStddev << ", numInterferingSignals = " << numInterferingSignals << ", meanInterferingSignalNoisePowerMean = " << meanInterferingSignalNoisePowerMean << ", meanInterferingSignalNoisePowerStddev = " << meanInterferingSignalNoisePowerStddev << std::endl;
            //snirsFile << std::endl;
            delete snir;
            delete listening;
            delete interference;
            delete arrival;
            delete reception;
            delete transmission;
            delete transmittedPacket;
        }
    }
}

Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> Ieee80211RadioErrorModelEvaluator::assembleNoisePowerFunction(std::vector<double> snirs, int frequencyDivision, int timeDivision, simtime_t startTime, simtime_t endTime, Hz startFrequency, Hz endFrequency)
{
    ASSERT(snirs.size() == frequencyDivision * timeDivision);

    auto bandwidth = endFrequency - startFrequency;

    Ptr<SummedFunction<WpHz, Domain<simsec, Hz>>> noisePowerFunction = makeShared<SummedFunction<WpHz, Domain<simsec, Hz>>>();
    std::cout << "making noise" << std::endl;
    for (int fi = 0; fi < frequencyDivision; fi++) {
        for (int ti = 0; ti < timeDivision; ti++) {
            // TODO
            //WpHz power = WpHz(snirs[j + i * timeDivision] / bandwidth * frequencyDivision);
            //rs.push_back(power);

            auto segment = makeShared<Boxcar2DFunction<WpHz, simsec, Hz>>(
                simsec(startTime + (endTime - startTime) * ti / timeDivision),
                simsec(startTime + (endTime - startTime) * (ti + 1) / timeDivision),
                startFrequency + bandwidth * fi / frequencyDivision,
                startFrequency + bandwidth * (fi + 1) / frequencyDivision,
                WpHz(snirs[ti*frequencyDivision + fi]));
            noisePowerFunction->addElement(segment);
        }
    }
    std::cout << "maketh noise" << std::endl;
    return noisePowerFunction;
}

} // namespace physicallayer
} // namespace inet

