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

#include "inet/physicallayer/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"
#include "inet/physicallayer/analogmodel/bitlevel/LayeredSnir.h"
#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/contract/packetlevel/IApskModulation.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/ieee80211/bitlevel/errormodel/Ieee80211OfdmErrorModel.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmSymbolModel.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OfdmErrorModel);

void Ieee80211OfdmErrorModel::initialize(int stage)
{
    Ieee80211NistErrorModel::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        symbolCorruptionMode = par("symbolCorruptionMode");
        dataSymbolErrorRate = par("dataSymbolErrorRate");
        dataBitErrorRate = par("dataBitErrorRate");
        signalSymbolErrorRate = par("signalSymbolErrorRate");
        signalBitErrorRate = par("signalBitErrorRate");
    }
}

std::ostream& Ieee80211OfdmErrorModel::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OfdmErrorModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", signalSymbolErrorRate = " << signalSymbolErrorRate
               << ", dataSymbolErrorRate = " << dataSymbolErrorRate
               << ", signalBitErrorRate = " << signalBitErrorRate
               << ", dataBitErrorRate = " << dataBitErrorRate;
    return stream;
}

const IReceptionPacketModel *Ieee80211OfdmErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    Enter_Method_Silent();
    double packetErrorRate = computePacketErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE);
    auto transmissionPacketModel = check_and_cast<const TransmissionPacketModel *>(transmission->getPacketModel());
    auto transmittedPacket = transmissionPacketModel->getPacket();
    auto receivedPacket = transmittedPacket->dup();
    if (packetErrorRate != 0 && uniform(0, 1) < packetErrorRate)
        receivedPacket->setBitError(true);
    receivedPacket->addTagIfAbsent<ErrorRateInd>()->setPacketErrorRate(packetErrorRate);
    return new ReceptionPacketModel(receivedPacket, transmissionPacketModel->getHeaderNetBitrate(), transmissionPacketModel->getDataNetBitrate());
}

const IReceptionBitModel *Ieee80211OfdmErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    const ITransmissionBitModel *transmissionBitModel = transmission->getBitModel();
    int signalBitLength = b(transmissionBitModel->getHeaderLength()).get();
    bps signalBitrate = transmissionBitModel->getHeaderGrossBitrate();
    int dataBitLength = b(transmissionBitModel->getDataLength()).get();
    bps dataBitrate = transmissionBitModel->getDataGrossBitrate();
    ASSERT(transmission->getSymbolModel() != nullptr);
    const IModulation *signalModulation = check_and_cast<const Ieee80211OfdmModulation *>(transmission->getSymbolModel()->getHeaderModulation())->getSubcarrierModulation();
    const IModulation *dataModulation = check_and_cast<const Ieee80211OfdmModulation *>(transmission->getSymbolModel()->getDataModulation())->getSubcarrierModulation();
    const BitVector *bits = transmissionBitModel->getAllBits();
    BitVector *corruptedBits = new BitVector(*bits);
    auto analogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    if (auto apskSignalModulation = dynamic_cast<const IApskModulation *>(signalModulation)) {
        double signalFieldBer = std::isnan(signalBitErrorRate) ? apskSignalModulation->calculateBER(getScalarSnir(snir), analogModel->getBandwidth(), signalBitrate) : signalBitErrorRate;
        corruptBits(corruptedBits, signalFieldBer, 0, signalBitLength);
    }
    else
        throw cRuntimeError("Unknown signal modulation");
    if (auto apskDataModulation = dynamic_cast<const IApskModulation *>(dataModulation)) {
        double dataFieldBer = std::isnan(dataBitErrorRate) ? apskDataModulation->calculateBER(getScalarSnir(snir), analogModel->getBandwidth(), dataBitrate) : dataBitErrorRate;
        corruptBits(corruptedBits, dataFieldBer, signalBitLength, corruptedBits->getSize());
    }
    else
        throw cRuntimeError("Unknown data modulation");
    return new ReceptionBitModel(b(signalBitLength), signalBitrate, b(dataBitLength), dataBitrate, corruptedBits);
}

const IReceptionSymbolModel *Ieee80211OfdmErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    //std::cout << "bleh" << std::endl;
    auto reception = snir->getReception();
    // bit model
    auto bitModel = transmission->getBitModel();
    auto headerGrossBitrate = bitModel->getHeaderGrossBitrate();
    auto dataGrossBitrate = bitModel->getDataGrossBitrate();
    // symbol model
    auto symbolModel = transmission->getSymbolModel();
    auto headerSymbolLength = symbolModel->getHeaderSymbolLength();
    auto dataSymbolLength = symbolModel->getDataSymbolLength();
    auto headerModulation = check_and_cast<const Ieee80211OfdmModulation *>(symbolModel->getHeaderModulation());
    auto headerSubcarrierModulation = headerModulation->getSubcarrierModulation();
    auto dataModulation = check_and_cast<const Ieee80211OfdmModulation *>(symbolModel->getDataModulation());
    auto dataSubcarrierModulation = dataModulation->getSubcarrierModulation();
    // analog model
    auto analogModel = transmission->getAnalogModel();
    auto preambleDuration = analogModel->getPreambleDuration();
    auto headerDuration = analogModel->getHeaderDuration();
    auto dataDuration = analogModel->getDataDuration();
    auto centerFrequency = check_and_cast<const INarrowbandSignal *>(analogModel)->getCenterFrequency();
    auto bandwidth = check_and_cast<const INarrowbandSignal *>(analogModel)->getBandwidth();
    // derived quantities
    auto startFrequency = centerFrequency - bandwidth / 2;
    auto endFrequency = centerFrequency + bandwidth / 2;
    auto startTime = reception->getStartTime();
    auto endTime = reception->getEndTime();
    auto headerStartTime = startTime + preambleDuration;
    auto headerEndTime = headerStartTime + headerDuration;
    auto dataStartTime = headerEndTime;
    auto dataEndTime = endTime;
    // division
    auto transmittedSymbols = symbolModel->getAllSymbols();
    int timeDivision = transmittedSymbols->size();
    ASSERT(headerSymbolLength + dataSymbolLength == timeDivision);
    auto frequencyDivision = dataModulation != nullptr ? dataModulation->getNumSubcarriers() : 1;
    int symbolCount = timeDivision * frequencyDivision;
    // calculate symbol intervals
    std::vector<math::Interval<simsec, Hz>> symbolIntervals;
    symbolIntervals.reserve(symbolCount);
    //std::cout << "blih" << std::endl;
    for (int i = 0; i < timeDivision; i++) {
        simtime_t symbolStartTime = i < headerSymbolLength ?
                LinearInterpolator<double, simtime_t>::singleton.getValue(0, headerStartTime, headerSymbolLength, headerEndTime, i) :
                LinearInterpolator<double, simtime_t>::singleton.getValue(1, dataStartTime, dataSymbolLength + 1, dataEndTime, i);
        simtime_t symbolEndTime = i < headerSymbolLength ?
                LinearInterpolator<double, simtime_t>::singleton.getValue(0, headerStartTime, headerSymbolLength, headerEndTime, i + 1) :
                LinearInterpolator<double, simtime_t>::singleton.getValue(1, dataStartTime, dataSymbolLength + 1, dataEndTime, i + 1);
        for (int j = 0; j < frequencyDivision; j++) {
            Hz symbolStartFrequency = (startFrequency * (frequencyDivision - j) + endFrequency * j) / frequencyDivision;
            Hz symbolEndFrequency = (startFrequency * (frequencyDivision - j - 1) + endFrequency * (j + 1)) / frequencyDivision;
            math::Point<simsec, Hz> symbolStartPoint(simsec(symbolStartTime), symbolStartFrequency);
            math::Point<simsec, Hz> symbolEndPoint(simsec(symbolEndTime), symbolEndFrequency);
            math::Interval<simsec, Hz> symbolInterval(symbolStartPoint, symbolEndPoint, 0b11, 0b00, 0b00);
            symbolIntervals.push_back(symbolInterval);
        }
    }
    //std::cout << "bloh" << std::endl;
    // partition SNIR function and sum SNIR values per symbol
    math::Point<simsec, Hz> startPoint(simsec(startTime), startFrequency);
    math::Point<simsec, Hz> endPoint(simsec(endTime), endFrequency);
    math::Interval<simsec, Hz> interval(startPoint, endPoint, 0b11, 0b00, 0b00);
    std::vector<double> data(symbolCount);
    auto snirFunction = check_and_cast<const LayeredSnir *>(snir)->getSnir();
    //std::cout << "bloh 2" << std::endl;
    snirFunction->partition(interval, [&] (const math::Interval<simsec, Hz>& i1, const math::IFunction<double, math::Domain<simsec, Hz>> *f1) {
        //std::cout << "parti" << std::endl;
        auto intervalStartTime = std::get<0>(i1.getLower()).get();
        auto intervalEndTime = std::get<0>(i1.getUpper()).get();
        auto intervalStartFrequency = std::get<1>(i1.getLower());
        auto intervalEndFrequency = std::get<1>(i1.getUpper());
        auto startTimeIndex = std::max(0, intervalStartTime < headerEndTime ?
                (int)std::floor((intervalStartTime - headerStartTime).dbl() / headerDuration.dbl() * headerSymbolLength) :
                headerSymbolLength + (int)std::floor((intervalStartTime - headerDuration - dataStartTime).dbl() / dataDuration.dbl() * dataSymbolLength));
        auto endTimeIndex = std::min(timeDivision - 1, intervalStartTime < headerEndTime ?
                (int)std::ceil((intervalEndTime - headerStartTime).dbl() / headerDuration.dbl() * headerSymbolLength) :
                headerSymbolLength + (int)std::ceil((intervalEndTime - headerDuration - dataStartTime).dbl() / dataDuration.dbl() * dataSymbolLength));
        auto startFrequencyIndex = std::max(0, (int)std::floor(Hz(intervalStartFrequency - startFrequency).get() / Hz(endFrequency - startFrequency).get() * frequencyDivision));
        auto endFrequencyIndex = std::min(frequencyDivision - 1, (int)std::ceil(Hz(intervalEndFrequency - startFrequency).get() / Hz(endFrequency - startFrequency).get() * frequencyDivision));
        // sum SNIR
        for (int i = startTimeIndex; i <= endTimeIndex; i++) {
            for (int j = startFrequencyIndex; j <= endFrequencyIndex; j++) {
                int index = i * frequencyDivision + j;
                ASSERT(0 <= index && index < symbolCount);
                auto i2 = i1.getIntersected(symbolIntervals[index]);
                double v = i2.getVolume() * f1->getMean(i2);
                ASSERT(!std::isnan(v));
                data[index] += v;
            }
        }
    });
    //std::cout << "bluh" << std::endl;
    // average symbol SNIR values
    auto symbolFrequencyBandwidth = bandwidth / frequencyDivision;
    for (int i = 0; i < timeDivision; i++) {
        std::vector<const ApskSymbol *> receivedSymbols;
        auto symbolDuration = i < headerSymbolLength ? headerDuration / headerSymbolLength : dataDuration / dataSymbolLength;
        for (int j = 0; j < frequencyDivision; j++) {
            int index = i * frequencyDivision + j;
            auto symbolArea = symbolDuration.dbl() * symbolFrequencyBandwidth.get();
            data[index] /= symbolArea;
        }
    }
    // calulate symbol error rate and corrupt symbols
    auto receivedOfdmSymbols = new std::vector<const ISymbol *>();
    for (int i = 0; i < timeDivision; i++) {
        auto modulation = i == 0 ? headerSubcarrierModulation : dataSubcarrierModulation;
        auto grossBitrate = i == 0 ? headerGrossBitrate : dataGrossBitrate;
        auto transmittedOfdmSymbol = check_and_cast<const Ieee80211OfdmSymbol *>(transmittedSymbols->at(i));
        std::vector<const ApskSymbol *> receivedSymbols;
        for (int j = 0; j < frequencyDivision; j++) {
            int index = i * frequencyDivision + j;
            double snirMean = data[index];
            double symbolErrorRate = modulation->calculateSER(snirMean, bandwidth / frequencyDivision, grossBitrate / frequencyDivision);
            bool isCorruptSymbol = symbolErrorRate == 1 || (symbolErrorRate != 0 && uniform(0, 1) < symbolErrorRate);
            auto transmittedSymbol = transmittedOfdmSymbol->getSubCarrierSymbols().at(j >= 26 ? j + 1 : j);
            auto receivedSymbol = isCorruptSymbol ? computeCorruptSymbol(modulation, transmittedSymbol) : transmittedSymbol;
            receivedSymbols.push_back(receivedSymbol);
            if (j == 25)
                receivedSymbols.push_back(nullptr); // see Ieee80211OfdmSymbol
        }
        auto receivedOfdmSymbol = new Ieee80211OfdmSymbol(receivedSymbols);
        receivedOfdmSymbols->push_back(receivedOfdmSymbol);
    }
    //std::cout << "blyh" << std::endl;
    return new ReceptionSymbolModel(symbolModel->getHeaderSymbolLength(), symbolModel->getHeaderSymbolRate(), symbolModel->getDataSymbolLength(), symbolModel->getDataSymbolRate(), receivedOfdmSymbols);
}

void Ieee80211OfdmErrorModel::corruptBits(BitVector *bits, double ber, int begin, int end) const
{
    for (int i = begin; i != end; i++) {
        double p = uniform(0, 1);
        if (p <= ber)
            bits->toggleBit(i);
    }
}

const IReceptionSampleModel *Ieee80211OfdmErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    throw cRuntimeError("Unimplemented!");
    // TODO: implement sample error model
    const ITransmissionSampleModel *transmissionSampleModel = transmission->getSampleModel();
    int headerSampleLength = transmissionSampleModel->getHeaderSampleLength();
    double headerSampleRate = transmissionSampleModel->getHeaderSampleRate();
    int dataSampleLength = transmissionSampleModel->getDataSampleLength();
    double dataSampleRate = transmissionSampleModel->getDataSampleRate();
    auto samples = transmissionSampleModel->getSamples();
    return new ReceptionSampleModel(headerSampleLength, headerSampleRate, dataSampleLength, dataSampleRate, samples);
}

const ApskSymbol *Ieee80211OfdmErrorModel::computeCorruptSymbol(const ApskModulationBase *modulation, const ApskSymbol *transmittedSymbol) const
{
    auto constellation = modulation->getConstellation();
    if (!strcmp("uniform", symbolCorruptionMode)) {
        auto index = intuniform(0, constellation->size() - 2);
        auto receivedSymbol = &constellation->at(index);
        if (receivedSymbol != transmittedSymbol)
            return receivedSymbol;
        else
            return &constellation->at(constellation->size() - 1);
    }
    else if (!strcmp("1bit", symbolCorruptionMode)) {
        auto bits = modulation->demapToBitRepresentation(transmittedSymbol);
        int errorIndex = intuniform(0, bits.getSize() - 1);
        bits.setBit(errorIndex, !bits.getBit(errorIndex));
        return modulation->mapToConstellationDiagram(bits);
    }
    else
        throw cRuntimeError("Unknown symbolCorruptionMode parameter value");
}

} // namespace physicallayer
} // namespace inet

