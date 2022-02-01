//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/errormodel/Ieee80211OfdmErrorModel.h"

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"
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

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OfdmErrorModel);

void Ieee80211OfdmErrorModel::initialize(int stage)
{
    Ieee80211NistErrorModel::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
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
    return new ReceptionPacketModel(receivedPacket, transmissionPacketModel->getBitrate(), NaN);
}

const IReceptionBitModel *Ieee80211OfdmErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    const ITransmissionBitModel *transmissionBitModel = transmission->getBitModel();
    int signalBitLength = b(transmissionBitModel->getHeaderLength()).get();
    bps signalBitRate = transmissionBitModel->getHeaderBitRate();
    int dataBitLength = b(transmissionBitModel->getDataLength()).get();
    bps dataBitRate = transmissionBitModel->getDataBitRate();
    ASSERT(transmission->getSymbolModel() != nullptr);
    const IModulation *signalModulation = transmission->getSymbolModel()->getHeaderModulation();
    const IModulation *dataModulation = transmission->getSymbolModel()->getPayloadModulation();
    const BitVector *bits = transmissionBitModel->getBits();
    BitVector *corruptedBits = new BitVector(*bits);
    const ScalarTransmissionSignalAnalogModel *analogModel = check_and_cast<const ScalarTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    if (auto apskSignalModulation = dynamic_cast<const IApskModulation *>(signalModulation)) {
        double signalFieldBer = std::isnan(signalBitErrorRate) ? apskSignalModulation->calculateBER(getScalarSnir(snir), analogModel->getBandwidth(), signalBitRate) : signalBitErrorRate;
        corruptBits(corruptedBits, signalFieldBer, 0, signalBitLength);
    }
    else
        throw cRuntimeError("Unknown signal modulation");
    if (auto apskDataModulation = dynamic_cast<const IApskModulation *>(dataModulation)) {
        double dataFieldBer = std::isnan(dataBitErrorRate) ? apskDataModulation->calculateBER(getScalarSnir(snir), analogModel->getBandwidth(), dataBitRate) : dataBitErrorRate;
        corruptBits(corruptedBits, dataFieldBer, signalBitLength, corruptedBits->getSize());
    }
    else
        throw cRuntimeError("Unknown data modulation");
    return new ReceptionBitModel(b(signalBitLength), signalBitRate, b(dataBitLength), dataBitRate, corruptedBits, NaN);
}

const IReceptionSymbolModel *Ieee80211OfdmErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    auto transmissionSymbolModel = check_and_cast<const Ieee80211OfdmTransmissionSymbolModel *>(transmission->getSymbolModel());
    auto symbols = transmissionSymbolModel->getSymbols();
    auto receivedSymbols = new std::vector<const ISymbol *>();
    // Only the first symbol is signal field symbol
    double scalarSnir = getScalarSnir(snir);
    receivedSymbols->push_back(corruptOfdmSymbol(check_and_cast<const Ieee80211OfdmSymbol *>(symbols->at(0)), &BpskModulation::singleton, scalarSnir));
    // The remaining are all data field symbols
    auto dataModulation = check_and_cast<const MqamModulationBase *>(transmissionSymbolModel->getPayloadModulation());
    for (unsigned int i = 1; i < symbols->size(); i++) {
        Ieee80211OfdmSymbol *corruptedOFDMSymbol = corruptOfdmSymbol(check_and_cast<const Ieee80211OfdmSymbol *>(symbols->at(i)), dataModulation, scalarSnir);
        receivedSymbols->push_back(corruptedOFDMSymbol);
    }
    return new Ieee80211OfdmReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getPayloadSymbolLength(), transmissionSymbolModel->getPayloadSymbolRate(), receivedSymbols);
}

void Ieee80211OfdmErrorModel::corruptBits(BitVector *bits, double ber, int begin, int end) const
{
    for (int i = begin; i != end; i++) {
        double p = uniform(0, 1);
        if (p <= ber)
            bits->toggleBit(i);
    }
}

Ieee80211OfdmSymbol *Ieee80211OfdmErrorModel::corruptOfdmSymbol(const Ieee80211OfdmSymbol *ofdmSymbol, const MqamModulationBase *modulation, double snir) const
{
    std::vector<const ApskSymbol *> subcarrierSymbols = ofdmSymbol->getSubCarrierSymbols();
    for (auto& subcarrierSymbol : subcarrierSymbols) {
        if (subcarrierSymbol != nullptr) {
            double transmittedI = subcarrierSymbol->real();
            double transmittedQ = subcarrierSymbol->imag();
            double sigma = std::sqrt(0.5 / snir);
            double receivedI = normal(transmittedI, sigma);
            double receivedQ = normal(transmittedQ, sigma);
            double bestDistanceSquare = DBL_MAX;
            const ApskSymbol *receivedSubcarrierSymbol = subcarrierSymbol;
            for (auto& symbol : *modulation->getConstellation()) {
                double i = symbol.real();
                double q = symbol.imag();
                double distanceSquare = std::pow((i - receivedI), 2) + std::pow((q - receivedQ), 2);
                if (distanceSquare < bestDistanceSquare) {
                    bestDistanceSquare = distanceSquare;
                    receivedSubcarrierSymbol = &symbol;
                }
            }
            subcarrierSymbol = receivedSubcarrierSymbol;
        }
    }
    return new Ieee80211OfdmSymbol(subcarrierSymbols);
}

const IReceptionSampleModel *Ieee80211OfdmErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    throw cRuntimeError("Unimplemented!");
    // TODO implement sample error model
    const ITransmissionSampleModel *transmissionSampleModel = transmission->getSampleModel();
    int sampleLength = transmissionSampleModel->getSampleLength();
    double sampleRate = transmissionSampleModel->getSampleRate();
    const std::vector<W> *samples = transmissionSampleModel->getSamples();
    W rssi = W(0); // TODO error model
    return new ReceptionSampleModel(sampleLength, sampleRate, samples, rssi);
}

} // namespace physicallayer
} // namespace inet

