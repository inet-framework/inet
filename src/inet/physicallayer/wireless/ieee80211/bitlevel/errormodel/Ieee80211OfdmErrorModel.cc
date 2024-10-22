//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/errormodel/Ieee80211OfdmErrorModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IApskModulation.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbolModel.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"

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

std::ostream& Ieee80211OfdmErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211OfdmErrorModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(signalSymbolErrorRate)
               << EV_FIELD(dataSymbolErrorRate)
               << EV_FIELD(signalBitErrorRate)
               << EV_FIELD(dataBitErrorRate);
    return stream;
}

const IReceptionPacketModel *Ieee80211OfdmErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
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
    return new ReceptionBitModel(b(signalBitLength), signalBitrate, b(dataBitLength), dataBitrate, corruptedBits, NaN);
}

const IReceptionSymbolModel *Ieee80211OfdmErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
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
    auto analogModel = check_and_cast<const NarrowbandSignalAnalogModel*>(transmission->getAnalogModel());
    // derived quantities

    auto mode = check_and_cast<const Ieee80211OfdmMode*>(check_and_cast<const Ieee80211TransmissionBase*>(transmission)->getMode())->getDataMode();

    // division
    auto transmittedSymbols = symbolModel->getAllSymbols();
    int numOfdmSymbols = transmittedSymbols->size();
    ASSERT(headerSymbolLength + dataSymbolLength == numOfdmSymbols);
    auto numSubcarriers = dataModulation != nullptr ? dataModulation->getNumSubcarriers() : 1;
    int symbolCount = numOfdmSymbols * numSubcarriers;

    // calculate symbol intervals
    std::vector<math::Interval<simsec, Hz>> symbolIntervals;
    symbolIntervals.reserve(symbolCount);
    for (int i = 0; i < numOfdmSymbols; i++) {
        simtime_t symbolStartTime = reception->getHeaderStartTime() + i * 4e-6;
        simtime_t symbolEndTime = reception->getHeaderStartTime() + (i+1) * 4e-6;
        for (int subcarrier = 0; subcarrier < numSubcarriers; subcarrier++) {
            auto startFreq = mode->getSubcarrierStartFrequencyOffset(subcarrier) + analogModel->getCenterFrequency();
            auto endFreq = mode->getSubcarrierEndFrequencyOffset(subcarrier) + analogModel->getCenterFrequency();
            math::Point<simsec, Hz> symbolStartPoint(simsec(symbolStartTime), startFreq);
            math::Point<simsec, Hz> symbolEndPoint(simsec(symbolEndTime), endFreq);
            math::Interval<simsec, Hz> symbolInterval(symbolStartPoint, symbolEndPoint, 0b11, 0b00, 0b00);
            symbolIntervals.push_back(symbolInterval);
        }
    }

    auto startFreq = mode->getSubcarrierStartFrequencyOffset(0) + analogModel->getCenterFrequency();
    auto endFreq = mode->getSubcarrierEndFrequencyOffset(numSubcarriers-1) + analogModel->getCenterFrequency();

    math::Point<simsec, Hz> startPoint(simsec(reception->getHeaderStartTime()), startFreq);
    math::Point<simsec, Hz> endPoint(simsec(reception->getDataEndTime()), endFreq);

    // partition SNIR function and sum SNIR values per symbol
    math::Interval<simsec, Hz> interval(startPoint, endPoint, 0b11, 0b00, 0b00);
    std::vector<double> data(symbolCount);
    auto snirFunction = check_and_cast<const LayeredSnir *>(snir)->getSnir();

    snirFunction->partition(interval, [&] (const math::Interval<simsec, Hz>& i1, const math::IFunction<double, math::Domain<simsec, Hz>> *f1) {

        auto intervalStartTime = std::get<0>(i1.getLower()).get();
        auto intervalEndTime = std::get<0>(i1.getUpper()).get();
        // sum SNIR

        int startTimeSlot = (int)std::floor((intervalStartTime - reception->getHeaderStartTime()) / SimTime(4, SIMTIME_US));
        int endTimeSlot = (int)std::ceil((intervalEndTime - reception->getHeaderStartTime()) / SimTime(4, SIMTIME_US)) + 1;

        startTimeSlot = std::max(0, startTimeSlot);
        endTimeSlot = std::min(numOfdmSymbols, endTimeSlot);

        for (int timeslot = startTimeSlot; timeslot < endTimeSlot; timeslot++) {
            // TODO: optimize, only iterate on subcarriers that have
            // a chance of intersecting the interval
            for (int subcarrier = 0; subcarrier < numSubcarriers; subcarrier++) {
                int index = timeslot * numSubcarriers + subcarrier;
                ASSERT(0 <= index && index < symbolCount);

                auto i2 = i1.getIntersected(symbolIntervals[index]);
                if (i2.isEmpty())
                    continue;

                double v = i2.getVolume() * f1->getMean(i2);
                ASSERT(!std::isnan(v));
                data[index] += v;
            }
        }
    });

    auto symbolFrequencyBandwidth = mode->getSubcarrierFrequencySpacing();
    auto symbolDuration = mode->getSymbolInterval();
    auto symbolArea = symbolDuration.dbl() * symbolFrequencyBandwidth.get();

    // average symbol SNIR values
    for (int i = 0; i < numOfdmSymbols; i++) {
        for (int j = 0; j < numSubcarriers; j++) {
            int index = i * numSubcarriers + j;
            data[index] /= symbolArea;
        }
    }


    // calulate symbol error rate and corrupt symbols
    auto receivedOfdmSymbols = new std::vector<const ISymbol *>();
    for (int i = 0; i < numOfdmSymbols; i++) {
        auto modulation = i == 0 ? headerSubcarrierModulation : dataSubcarrierModulation;
        auto grossBitrate = i == 0 ? headerGrossBitrate : dataGrossBitrate;
        auto transmittedOfdmSymbol = check_and_cast<const Ieee80211OfdmSymbol *>(transmittedSymbols->at(i));
        std::vector<const ApskSymbol *> receivedSymbols;
        for (int j = 0; j < numSubcarriers; j++) {
            int index = i * numSubcarriers + j;
            double snirMean = data[index];
            auto transmittedSymbol = transmittedOfdmSymbol->getSubCarrierSymbols().at(j >= 26 ? j + 1 : j);
            double symbolErrorRate = modulation->calculateSER(snirMean, symbolFrequencyBandwidth, grossBitrate / numSubcarriers);
            auto receivedSymbol = computeReceivedSymbol(modulation, transmittedSymbol, snirMean, symbolErrorRate);
            receivedSymbols.push_back(new ApskSymbol(*receivedSymbol));
            if (j == 25)
                receivedSymbols.push_back(nullptr); // see Ieee80211OfdmSymbol
        }
        auto receivedOfdmSymbol = new Ieee80211OfdmSymbol(receivedSymbols);
        receivedOfdmSymbols->push_back(receivedOfdmSymbol);
    }
    return new ReceptionSymbolModel(symbolModel->getHeaderSymbolLength(), symbolModel->getHeaderSymbolRate(), symbolModel->getDataSymbolLength(), symbolModel->getDataSymbolRate(), receivedOfdmSymbols, NaN);
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
    // TODO implement sample error model
    const ITransmissionSampleModel *transmissionSampleModel = transmission->getSampleModel();
    int headerSampleLength = transmissionSampleModel->getHeaderSampleLength();
    double headerSampleRate = transmissionSampleModel->getHeaderSampleRate();
    int dataSampleLength = transmissionSampleModel->getDataSampleLength();
    double dataSampleRate = transmissionSampleModel->getDataSampleRate();
    const std::vector<W> *samples = transmissionSampleModel->getSamples();
    // TODO error model?
    return new ReceptionSampleModel(headerSampleLength, headerSampleRate, dataSampleLength, dataSampleRate, samples);
}

// TODO: This is duplicated from LayeredErrorModelBase
const ApskSymbol *Ieee80211OfdmErrorModel::computeReceivedSymbol(const ApskModulationBase *modulation, const ApskSymbol *transmittedSymbol, double snir, double symbolErrorRate) const
{
    auto constellation = modulation->getConstellation();

    if (!strcmp("vectorial", symbolCorruptionMode)) {
        double transmittedI = transmittedSymbol->real();
        double transmittedQ = transmittedSymbol->imag();
        double sigma = std::sqrt(0.5 / snir);
        double receivedI = normal(transmittedI, sigma);
        double receivedQ = normal(transmittedQ, sigma);
        double bestDistanceSquare = DBL_MAX;
        const ApskSymbol *receivedSubcarrierSymbol = transmittedSymbol;
        for (auto& symbol : *modulation->getConstellation()) {
            double i = symbol.real();
            double q = symbol.imag();
            double distanceSquare = std::pow((i - receivedI), 2) + std::pow((q - receivedQ), 2);
            if (distanceSquare < bestDistanceSquare) {
                bestDistanceSquare = distanceSquare;
                receivedSubcarrierSymbol = &symbol;
            }
        }
        return receivedSubcarrierSymbol;
    }
    else {
        bool isCorruptSymbol = symbolErrorRate == 1 || (symbolErrorRate != 0 && uniform(0, 1) < symbolErrorRate);

        if (isCorruptSymbol) {
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
        else
            return transmittedSymbol;
    }
}

} // namespace physicallayer
} // namespace inet

