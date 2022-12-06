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
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalAnalogModel.h"
#include "inet/physicallayer/analogmodel/bitlevel/LayeredSnir.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/common/bitlevel/LayeredReception.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalReception.h"
#include "inet/physicallayer/common/bitlevel/LayeredReceptionResult.h"
#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/common/packetlevel/Interference.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/errormodel/packetlevel/Ieee80211RadioErrorModelEvaluator.h"
#include "inet/physicallayer/errormodel/packetlevel/NeuralNetworkErrorModel.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211DimensionalTransmission.h"

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


LeftInterpolator<simsec, WpHz> intX;
LeftInterpolator<Hz, WpHz> intY;

Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> assembleNoisePowerFunction(std::vector<double> snirs, int frequencyDivision, int timeDivision, simtime_t startTime, simtime_t endTime, Hz startFrequency, Hz endFrequency)
{
    ASSERT(snirs.size() == frequencyDivision * timeDivision);

    auto bandwidth = endFrequency - startFrequency;
    auto duration = endTime - startTime;

    //return makeShared<ConstantFunction<WpHz, Domain<simsec, Hz>>>(WpHz(0.1 / 20000000.0 / 200.0));


    std::vector<WpHz> noisePowers;
    for (int i = 0; i < snirs.size(); ++i) {
        double s = snirs[i];

        noisePowers.push_back(WpHz(1 / s)); // signal should be 1 WpHz (lol 20MW WiFi)
        if ((i % 52) == 51)
            noisePowers.push_back(WpHz(0.1));
    }

    for (int i = 0; i < 53; ++i) {
        noisePowers.push_back(WpHz(0.1));
    }

    auto preambleNoise = makeShared<Boxcar2DFunction<WpHz, simsec, Hz>>(simsec(0), simsec(startTime), startFrequency, endFrequency, WpHz(0.01));
    auto dataNoise = makeShared<PeriodicallyInterpolated2DFunction<WpHz, simsec, Hz>>(simsec(startTime), simsec(endTime), timeDivision+1, startFrequency, endFrequency, frequencyDivision+1, intX, intY, noisePowers);

    return preambleNoise->add(dataNoise);
/*

    Ptr<SummedFunction<WpHz, Domain<simsec, Hz>>> noisePowerFunction = makeShared<SummedFunction<WpHz, Domain<simsec, Hz>>>();
    std::cout << "making noise" << std::endl;
    for (int fi = 0; fi < frequencyDivision; fi++) {
        for (int ti = 0; ti < timeDivision; ti++) {
            // TODO
            //WpHz power = WpHz(snirs[j + i * timeDivision] / bandwidth * frequencyDivision);
            //rs.push_back(power);

            auto segment = makeShared<Boxcar2DFunction<WpHz, simsec, Hz>>(
                simsec(startTime + duration * ti / timeDivision),
                simsec(startTime + duration * (ti + 1) / timeDivision),
                startFrequency + bandwidth * fi / frequencyDivision,
                startFrequency + bandwidth * (fi + 1) / frequencyDivision,
                WpHz(snirs[ti*frequencyDivision + fi]));
            noisePowerFunction->addElement(segment);
        }
    }
    std::cout << "maketh noise" << std::endl;

    for (Hz f = startFrequency - bandwidth*0.2; f < endFrequency + bandwidth * 0.2; f += kHz(100)) {
        for (simtime_t t = startTime - duration * 0.2; t < endTime + duration * 0.2; t += SimTime(1, SIMTIME_US)) {
            std::cout << t << " " << f << " " << noisePowerFunction->getValue({simsec(t), f}) << std::endl;
        }
    }


    return noisePowerFunction;*/
}

DimensionalNoise *createNoise(std::vector<double> snirs, const IReception *reception, int frequencyDivision, int timeDivision, SimTime startTime, SimTime endTime) {
        // create noise
    auto narrowbandSignal = check_and_cast<const INarrowbandSignal *>(reception->getAnalogModel());
    auto centerFrequency = narrowbandSignal->getCenterFrequency();
    auto bandwidth = kHz(312.5) * 53;
    auto startFrequency = centerFrequency - bandwidth / 2;
    auto endFrequency = centerFrequency + bandwidth / 2;
    auto noisePowerFunction = assembleNoisePowerFunction(snirs, frequencyDivision, timeDivision, startTime, endTime, startFrequency, endFrequency);
    auto noise = new DimensionalNoise(startTime, endTime, centerFrequency, bandwidth, noisePowerFunction);
    return noise;
}



const LayeredTransmission *createLayeredTransmission(const IRadio *radio, const ITransmitter *transmitter, Packet *transmittedPacket) {

    // create transmission
    simtime_t startTime = 0;
    auto transmission = check_and_cast<const LayeredTransmission *>(transmitter->createTransmission(radio, transmittedPacket, startTime));
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


    /*
    // TODO:
    timeDivision = snirs.size() / frequencyDivision;


    int numSymbols = timeDivision * frequencyDivision;


    std::cout << "timeDivision = " << timeDivision << std::endl;
    std::cout << "numSymbols = " << numSymbols << std::endl;
    */

    return transmission;
}

const ISnir *createLayeredSnir(const LayeredReception *reception, const DimensionalNoise *noise) {
    // create snir
    const ISnir *snir = new LayeredSnir(reception, noise);
    return snir;
}

const ITransmission *createNonLayeredTransmission(const IRadio *radio, const ITransmitter *transmitter, Packet *transmittedPacket) {

    // create transmission
    simtime_t startTime = 0;
    auto transmission = check_and_cast<const Ieee80211DimensionalTransmission *>(transmitter->createTransmission(radio, transmittedPacket, startTime));
    auto transmissionAnalogModel = transmission->getAnalogModel();
    auto preambleDuration = transmissionAnalogModel->getPreambleDuration();
    auto headerDuration = transmissionAnalogModel->getHeaderDuration();
    auto dataDuration = transmissionAnalogModel->getDataDuration();
    auto duration = transmissionAnalogModel->getDuration();

    //auto bitrate = transmission->getBitModel()->getDataGrossBitrate();
    //if (auto forwardErrorCorrection = transmission->getBitModel()->getForwardErrorCorrection())
    //    bitrate *= forwardErrorCorrection->getCodeRate();
    auto modulation = transmission->getModulation();
    auto ofdmModulation = dynamic_cast<const Ieee80211OfdmModulation *>(modulation);
    auto subcarrierModulation = ofdmModulation != nullptr ? ofdmModulation->getSubcarrierModulation() : nullptr;
    //auto transmittedSymbols = transmission-> getSymbolModel()->getAllSymbols();
    // TODO: time division doesn't take into account that the preamble doesn't contain symbols
    int timeDivision = transmission->getDuration() / transmission->getSymbolTime();
    int frequencyDivision = ofdmModulation != nullptr ? ofdmModulation->getNumSubcarriers() : 1;
    if (frequencyDivision != 52)
        throw cRuntimeError("wrong number of OFDM subcarriers!");


    return transmission;
    /*
    // TODO:
    timeDivision = snirs.size() / frequencyDivision;


    int numSymbols = timeDivision * frequencyDivision;

    std::cout << "timeDivision = " << timeDivision << std::endl;
    std::cout << "numSymbols = " << numSymbols << std::endl;
    */
}

const DimensionalSnir *createNonLayeredSnir(const DimensionalReception *reception, const DimensionalNoise *noise) {
    // create snir
    //auto signalAnalogModel = check_and_cast<const DimensionalReception *>(reception->getAnalogModel());
    //auto receptionPowerFunction = signalAnalogModel->getPower();
    return new DimensionalSnir(reception, noise);
}

template<typename T>
double getval(T v) {
    return v.get();
}

template<>
double getval(double v) {
    return v;
}

template<typename R>
void dump(const char *name, inet::Ptr<const IFunction<R, Domain<simsec,Hz>>> fn, Interval<simsec, Hz> range) {

    std::ofstream ofs(name + std::string(".csv"), std::ios::trunc);

    bool header = true;
    for (double t = range.getLower().get(0); t < range.getUpper().get(0); t += 0.000'001) {
        if (header) {
            ofs << name;
            for (double f = range.getLower().get(1); f < range.getUpper().get(1); f += 100'000) {
                ofs << " " << std::setprecision(9) << f;
            }
            ofs << std::endl;
        }

        header = false;

        ofs << std::setprecision(9) << t;
        for (double f = range.getLower().get(1); f < range.getUpper().get(1); f += 100'000) {
            ofs << " " << std::setprecision(9) << getval(fn->getValue({simsec(t), Hz(f)}));
        }

        ofs << std::endl;
    }

}

void Ieee80211RadioErrorModelEvaluator::evaluateErrorModel()
{
    // TODO: separate preamble, header and data parts for generating the noise and the training data
    std::cout << "Evaluating error model" << std::endl;

    std::cerr << "radiomedium is: " << dynamic_cast<const AnalogModelBase*>(radioMedium->getAnalogModel())->getClassName() << std::endl;


    std::cerr << "got the layered analog model" << std::endl;

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
                //std::cout << s << std::endl;
                snirs.push_back(atof(s.c_str()));
            }
        }

        ASSERT(snirs.size() % 52 == 0);

        int frequencyDivision = 52;
        int timeDivision = snirs.size() / frequencyDivision;

        // 48 usable subcarriers, 4 bits per symbol, 1/2 coding rate, 6 tail bits, 16 service bits, 24 signal bits
        b dataLength = b((timeDivision-1) * 48 * 4 / 2 - 6 - 16 - 24); // BIG TODO

        std::cout << "to get " << snirs.size() << " SNIRS, I think we need " << dataLength.get() << " bits of data" << std::endl;

        // create packet, including 30 bytes MAC header, and 4 bytes of FCS
        std::vector<uint8_t> bytes;
        for (b i = b(0); (i + b(8)) <= dataLength; i += b(8))
            bytes.push_back(intuniform(0, 255));
        auto data = makeShared<BytesChunk>(bytes);
        auto transmittedPacket = new Packet(nullptr, data);
        transmittedPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
        std::cout << transmittedPacket->getByteLength() << " bytes long packet is ";

        // adds the following:
        //  - tail bits (6b)
        //  - service bits (16b)
        //  - plus padding to whole ofdm symbol (variable bits)
        //  - one extra ofdm symbol for phy header (always 3 bytes)
        radio->encapsulate(transmittedPacket);

        std::cout << transmittedPacket->getByteLength() << " bytes long packet after encapsulation" << std::endl;

        const ITransmission *transmission = nullptr;
        if (auto analogModel = dynamic_cast<const LayeredDimensionalAnalogModel *>(radioMedium->getAnalogModel()))
            transmission = createLayeredTransmission(radio, transmitter, transmittedPacket);
        if (auto analogModel = dynamic_cast<const DimensionalAnalogModel *>(radioMedium->getAnalogModel()))
            transmission = createNonLayeredTransmission(radio, transmitter, transmittedPacket);

        std::cerr << "durations: " << transmission->getPreambleDuration() <<
                " " << transmission->getHeaderDuration() <<
                " " << transmission->getDataDuration() << std::endl;

        std::cerr << "preamble: " << transmission->getPreambleStartTime() << " - " << transmission->getPreambleEndTime() << std::endl;
        std::cerr << "header: " << transmission->getHeaderStartTime() << " - " << transmission->getHeaderEndTime() << std::endl;
        std::cerr << "data: " << transmission->getDataStartTime() << " - " << transmission->getDataEndTime() << std::endl;
        std::cerr << "ALL: " <<  transmission->getStartTime() << " - " << transmission->getEndTime() << std::endl;

        std::cerr << "transm dur is " << transmission->getDuration() << std::endl;
        std::cerr << "expected: " << SimTime(4, SIMTIME_US) * timeDivision << std::endl;
        //ASSERT(transmission->getDuration() == SimTime(4, SIMTIME_US) * timeDivision);

        SimTime startTime = transmission->getHeaderStartTime();
        SimTime endTime = transmission->getDataEndTime();
        auto listening = receiver->createListening(nullptr, startTime, endTime, Coord::ZERO, Coord::ZERO);


        // create reception
        auto arrival = propagation->computeArrival(transmission, radio->getAntenna()->getMobility());
        const IReception *reception = radioMedium->getAnalogModel()->computeReception(radio, transmission, arrival);

        const LayeredReception *layeredReception = dynamic_cast<const LayeredReception *>(reception);
        const DimensionalReception *dimensionalReception = dynamic_cast<const DimensionalReception *>(reception);


        auto noise = createNoise(snirs, reception, frequencyDivision, timeDivision, startTime, endTime);
        auto interference = new Interference(noise, new std::vector<const IReception *>());

        if (dimensionalReception) {
            auto powerFn = dimensionalReception->getPower();
            dump("power", powerFn, {{simsec(0.000'000'5), Hz(2'400'000'000.0)}, {simsec(0.0002), Hz(2'425'000'000.0)}, true, true, 0});
        }
        auto noiseFn = noise->getPower();


        dump("noise", noiseFn, {{simsec(0.000'000'5), Hz(2'400'000'000.0)}, {simsec(0.0002), Hz(2'425'000'000.0)}, true, true, 0});

        //auto snirFunction = receptionPowerFunction->divide(noisePowerFunction);

        const ISnir *snir = nullptr;
        if (auto analogModel = dynamic_cast<const LayeredDimensionalAnalogModel *>(radioMedium->getAnalogModel()))
            snir = createLayeredSnir(layeredReception, noise);
        if (auto analogModel = dynamic_cast<const DimensionalAnalogModel *>(radioMedium->getAnalogModel())) {
            auto dsnir = createNonLayeredSnir(dimensionalReception, noise);
            auto dsnirf = dsnir->getSnir();
            dump("snir", dsnirf, {{simsec(0.000'000'5), Hz(2'400'000'000.0)}, {simsec(0.0002), Hz(2'425'000'000.0)}, true, true, 0});
            snir = dsnir;
        }

        double packetErrorRate = NAN;

        int receptionSuccessfulCount = 0;
        std::cout << "receiving..." << std::endl;
        for (int i = 0; i < repeatCount; i++) {
            auto decisions = new std::vector<const IReceptionDecision *>();
            std::cout << "comp rec res" << std::endl;
            const IReceptionDecision *receptionDecision = //new ReceptionDecision(reception, IRadioSignal::SIGNAL_PART_WHOLE, true, true, true);
            receiver->computeReceptionDecision(listening, reception, IRadioSignal::SIGNAL_PART_WHOLE, interference, snir);
            decisions->push_back(receptionDecision);

            auto receptionResult = receiver->computeReceptionResult(listening, reception, interference, snir, decisions);
            std::cout << "done comp rec res" << std::endl;
            auto receivedPacket = receptionResult->getPacket();
            if (ErrorRateInd *errorRateInd = receivedPacket->findTag<ErrorRateInd>()) {
                std::cerr << "EROR RATE IND " << errorRateInd->getPacketErrorRate() << std::endl;
                if (!std::isnan(errorRateInd->getPacketErrorRate())) {
                    packetErrorRate = errorRateInd->getPacketErrorRate();
                    break;
                }
            }

            bool isReceptionSuccessful = !receivedPacket->hasBitError();
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

        if (std::isnan(packetErrorRate))
            packetErrorRate = (1 - (double)receptionSuccessfulCount / repeatCount);

        // print parameters
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

    }

}

} // namespace physicallayer
} // namespace inet

