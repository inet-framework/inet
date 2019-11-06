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

#include "inet/physicallayer/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/contract/packetlevel/IApskModulation.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmSymbolModel.h"
#include "inet/physicallayer/ieee80211/bitlevel/errormodel/Ieee80211OfdmErrorModel.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"
#include "inet/physicallayer/modulation/BpskModulation.h"

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
    double packetErrorRate = computePacketErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE);
    auto transmissionPacketModel = check_and_cast<const TransmissionPacketModel *>(transmission->getPacketModel());
    auto transmittedPacket = transmissionPacketModel->getPacket();
    auto receivedPacket = transmittedPacket->dup();
    if (packetErrorRate != 0 && uniform(0, 1) < packetErrorRate)
        receivedPacket->setBitError(true);
    receivedPacket->addTagIfAbsent<ErrorRateInd>()->setPacketErrorRate(packetErrorRate);
    return new ReceptionPacketModel(receivedPacket, transmissionPacketModel->getBitrate());
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
    return new ReceptionBitModel(b(signalBitLength), signalBitRate, b(dataBitLength), dataBitRate, corruptedBits);
}

const IReceptionSymbolModel *Ieee80211OfdmErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    const ITransmissionBitModel *transmissionBitModel = transmission->getBitModel();
    const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const Ieee80211OfdmTransmissionSymbolModel *>(transmission->getSymbolModel());
    const IModulation *modulation = transmissionSymbolModel->getPayloadModulation();
    const ApskModulationBase *dataModulation = check_and_cast<const ApskModulationBase *>(modulation);
    unsigned int dataFieldConstellationSize = dataModulation->getConstellationSize();
    unsigned int signalFieldConstellationSize = BpskModulation::singleton.getConstellationSize();
    const std::vector<ApskSymbol> *constellationForDataField = dataModulation->getConstellation();
    const std::vector<ApskSymbol> *constellationForSignalField = BpskModulation::singleton.getConstellation();
    const ScalarTransmissionSignalAnalogModel *analogModel = check_and_cast<const ScalarTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    double signalSER = std::isnan(signalSymbolErrorRate) ? BpskModulation::singleton.calculateSER(getScalarSnir(snir), analogModel->getBandwidth(), transmissionBitModel->getHeaderBitRate()) : signalSymbolErrorRate;
    double dataSER = std::isnan(dataSymbolErrorRate) ? dataModulation->calculateSER(getScalarSnir(snir), analogModel->getBandwidth(), transmissionBitModel->getDataBitRate()) : dataSymbolErrorRate;
    const std::vector<const ISymbol *> *symbols = transmissionSymbolModel->getSymbols();
    std::vector<const ISymbol *> *corruptedSymbols = new std::vector<const ISymbol *>();
    // Only the first symbol is signal field symbol
    corruptedSymbols->push_back(corruptOFDMSymbol(check_and_cast<const Ieee80211OfdmSymbol *>(symbols->at(0)), signalSER, signalFieldConstellationSize, constellationForSignalField));
    // The remaining are all data field symbols
    for (unsigned int i = 1; i < symbols->size(); i++) {
        Ieee80211OfdmSymbol *corruptedOFDMSymbol = corruptOFDMSymbol(check_and_cast<const Ieee80211OfdmSymbol *>(symbols->at(i)), dataSER,
                    dataFieldConstellationSize, constellationForDataField);
        corruptedSymbols->push_back(corruptedOFDMSymbol);
    }
    return new Ieee80211OfdmReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getPayloadSymbolLength(), transmissionSymbolModel->getPayloadSymbolRate(), corruptedSymbols);
}

void Ieee80211OfdmErrorModel::corruptBits(BitVector *bits, double ber, int begin, int end) const
{
    for (int i = begin; i != end; i++) {
        double p = uniform(0, 1);
        if (p <= ber)
            bits->toggleBit(i);
    }
}

Ieee80211OfdmSymbol *Ieee80211OfdmErrorModel::corruptOFDMSymbol(const Ieee80211OfdmSymbol *symbol, double ser, int constellationSize, const std::vector<ApskSymbol> *constellation) const
{
    std::vector<const ApskSymbol *> subcarrierSymbols = symbol->getSubCarrierSymbols();
    for (int j = 0; j < symbol->symbolSize(); j++) {
        double p = uniform(0, 1);
        if (p <= ser) {
            int corruptedSubcarrierSymbolIndex = intuniform(0, constellationSize - 1); // TODO: it can be equal to the current symbol
            const ApskSymbol *corruptedSubcarrierSymbol = &constellation->at(corruptedSubcarrierSymbolIndex);
            subcarrierSymbols[j] = corruptedSubcarrierSymbol;
        }
    }
    return new Ieee80211OfdmSymbol(subcarrierSymbols);
}

const IReceptionSampleModel *Ieee80211OfdmErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    throw cRuntimeError("Unimplemented!");
    // TODO: implement sample error model
    const ITransmissionSampleModel *transmissionSampleModel = transmission->getSampleModel();
    int sampleLength = transmissionSampleModel->getSampleLength();
    double sampleRate = transmissionSampleModel->getSampleRate();
    const std::vector<W> *samples = transmissionSampleModel->getSamples();
    W rssi = W(0); // TODO: error model
    return new ReceptionSampleModel(sampleLength, sampleRate, samples, rssi);
}

} // namespace physicallayer
} // namespace inet

