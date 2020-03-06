//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/apsk/bitlevel/errormodel/ApskLayeredErrorModel.h"

// TODO after merge: fix includes here
#include "inet/physicallayer/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"
#include "inet/physicallayer/analogmodel/bitlevel/LayeredSnir.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/apskradio/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskLayeredErrorModel);

std::ostream& ApskLayeredErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "LayeredApskErrorModel";
}

const IReceptionPacketModel *ApskLayeredErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    const ITransmissionBitModel *bitModel = transmission->getBitModel();
    const DimensionalTransmissionSignalAnalogModel *analogModel = check_and_cast<const DimensionalTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    const IModulation *modulation = transmission->getSymbolModel()->getDataModulation();
    double grossBitErrorRate = modulation->calculateBER(snir->getMin(), analogModel->getBandwidth(), bitModel->getDataGrossBitrate());
    int bitLength = transmission->getPacketModel()->getPacket()->getBitLength();
    double packetErrorRate;
    const IForwardErrorCorrection *forwardErrorCorrection = transmission->getBitModel()->getForwardErrorCorrection();
    if (forwardErrorCorrection == nullptr)
        packetErrorRate = 1.0 - pow(1.0 - grossBitErrorRate, bitLength);
    else {
        double netBitErrorRate = forwardErrorCorrection->computeNetBitErrorRate(grossBitErrorRate);
        packetErrorRate = 1.0 - pow(1.0 - netBitErrorRate, bitLength);
    }
    return LayeredErrorModelBase::computePacketModel(transmission, packetErrorRate);
}

const IReceptionBitModel *ApskLayeredErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    const ITransmissionBitModel *bitModel = transmission->getBitModel();
    const DimensionalTransmissionSignalAnalogModel *analogModel = check_and_cast<const DimensionalTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    const IModulation *modulation = transmission->getSymbolModel()->getDataModulation();
    double bitErrorRate = modulation->calculateBER(snir->getMin(), analogModel->getBandwidth(), bitModel->getDataGrossBitrate());
    return LayeredErrorModelBase::computeBitModel(transmission, bitErrorRate);
}

const IReceptionSymbolModel *ApskLayeredErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    auto layeredSnir = check_and_cast<const LayeredSnir *>(snir);
    auto analogModel = check_and_cast<const DimensionalTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    auto bitModel = transmission->getBitModel();
    auto transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
    auto transmittedSymbols = transmissionSymbolModel->getAllSymbols();
    auto modulation = check_and_cast<const ApskModulationBase *>(transmissionSymbolModel->getDataModulation());
    auto bitrate = bitModel->getDataGrossBitrate();
    auto centerFrequency = analogModel->getCenterFrequency();
    auto bandwidth = analogModel->getBandwidth();
    auto reception = snir->getReception();
    auto startTime = reception->getStartTime();
    auto endTime = reception->getEndTime();
    auto duration = endTime - startTime;
    int symbolCount = transmittedSymbols->size();
    std::vector<math::Interval<simsec, Hz>> symbolIntervals;
    symbolIntervals.reserve(symbolCount);
    for (int symbolIndex = 0; symbolIndex < symbolCount; symbolIndex++) {
        simtime_t symbolStartTime = (startTime * (symbolCount - symbolIndex) + endTime * symbolIndex) / symbolCount;
        simtime_t symbolEndTime = (startTime * (symbolCount - symbolIndex - 1) + endTime * (symbolIndex + 1)) / symbolCount;
        math::Point<simsec, Hz> symbolStartPoint(simsec(symbolStartTime), centerFrequency - bandwidth / 2);
        math::Point<simsec, Hz> symbolEndPoint(simsec(symbolEndTime), centerFrequency + bandwidth / 2);
        math::Interval<simsec, Hz> symbolInterval(symbolStartPoint, symbolEndPoint, 0b11, 0b00, 0b00);
        symbolIntervals.push_back(symbolInterval);
    }
    math::Point<simsec, Hz> startPoint(simsec(startTime), centerFrequency - bandwidth / 2);
    math::Point<simsec, Hz> endPoint(simsec(endTime), centerFrequency + bandwidth / 2);
    math::Interval<simsec, Hz> interval(startPoint, endPoint, 0b11, 0b00, 0b00);
    std::vector<double> data(symbolCount);
    auto snirFunction = layeredSnir->getSnir();
    snirFunction->partition(interval, [&] (const math::Interval<simsec, Hz>& i1, const math::IFunction<double, math::Domain<simsec, Hz>> *f1) {
        auto intervalStartTime = std::get<0>(i1.getLower()).get();
        auto intervalEndTime = std::get<0>(i1.getUpper()).get();
        auto startIndex = std::max(0, (int)std::floor((intervalStartTime - startTime).dbl() / (endTime - startTime).dbl() * symbolCount));
        auto endIndex = std::min(symbolCount - 1, (int)std::ceil((intervalEndTime - startTime).dbl() / (endTime - startTime).dbl() * symbolCount));
        for (int symbolIndex = startIndex; symbolIndex <= endIndex; symbolIndex++) {
            ASSERT(0 <= symbolIndex && symbolIndex < symbolCount);
            auto i2 = i1.getIntersected(symbolIntervals[symbolIndex]);
            double v = i2.getVolume() * f1->getMean(i2);
            ASSERT(!std::isnan(v));
            data[symbolIndex] += v;
        }
    });
    auto symbolDuration = duration / symbolCount;
    auto symbolArea = symbolDuration * bandwidth.get();
    auto receivedSymbols = new std::vector<const ISymbol *>(symbolCount);
    for (int i = 0; i < symbolCount; i++) {
        double snirMean = data[i] / symbolArea;
        double symbolErrorRate = modulation->calculateSER(snirMean, bandwidth, bitrate);
        auto transmittedSymbol = check_and_cast<const ApskSymbol *>(transmittedSymbols->at(i));
        bool isCorruptSymbol = symbolErrorRate == 1 || (symbolErrorRate != 0 && uniform(0, 1) < symbolErrorRate);
        auto receivedSymbol = isCorruptSymbol ? computeCorruptSymbol(modulation, transmittedSymbol) : transmittedSymbol;
        receivedSymbols->at(i) = receivedSymbol;
    }
    return new ReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getDataSymbolLength(), transmissionSymbolModel->getDataSymbolRate(), receivedSymbols);
}

const IReceptionSampleModel *ApskLayeredErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    throw cRuntimeError("Not yet implemented");
}

} // namespace physicallayer

} // namespace inet

