//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/bitlevel/LayeredErrorModelBase.h"

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {
namespace physicallayer {

const IReceptionPacketModel *LayeredErrorModelBase::computePacketModel(const LayeredTransmission *transmission, double packetErrorRate) const
{
    auto transmissionPacketModel = check_and_cast<const TransmissionPacketModel *>(transmission->getPacketModel());
    auto transmittedPacket = transmissionPacketModel->getPacket();
    auto receivedPacket = transmittedPacket->dup();
    if (packetErrorRate != 0 && uniform(0, 1) < packetErrorRate)
        receivedPacket->setBitError(true);
    receivedPacket->addTagIfAbsent<ErrorRateInd>()->setPacketErrorRate(packetErrorRate);
    return new ReceptionPacketModel(receivedPacket, transmissionPacketModel->getBitrate(), packetErrorRate);
}

const IReceptionBitModel *LayeredErrorModelBase::computeBitModel(const LayeredTransmission *transmission, double bitErrorRate) const
{
    const TransmissionBitModel *transmissionBitModel = check_and_cast<const TransmissionBitModel *>(transmission->getBitModel());
    if (bitErrorRate == 0)
        return new ReceptionBitModel(transmissionBitModel->getHeaderLength(), transmissionBitModel->getHeaderBitRate(), transmissionBitModel->getDataLength(), transmissionBitModel->getDataBitRate(), new BitVector(*transmissionBitModel->getBits()), bitErrorRate);
    else {
        BitVector *receivedBits = new BitVector(*transmissionBitModel->getBits());
        for (unsigned int i = 0; i < receivedBits->getSize(); i++) {
            if (uniform(0, 1) < bitErrorRate)
                receivedBits->toggleBit(i);
        }
        return new ReceptionBitModel(transmissionBitModel->getHeaderLength(), transmissionBitModel->getHeaderBitRate(), transmissionBitModel->getDataLength(), transmissionBitModel->getDataBitRate(), receivedBits, bitErrorRate);
    }
}

const IReceptionSymbolModel *LayeredErrorModelBase::computeSymbolModel(const LayeredTransmission *transmission, double symbolErrorRate) const
{
    if (symbolErrorRate == 0) {
        const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
        return new ReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getPayloadSymbolLength(), transmissionSymbolModel->getPayloadSymbolRate(), new std::vector<const ISymbol *>(*transmissionSymbolModel->getSymbols()), symbolErrorRate);
    }
    else {
        const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
        const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(transmissionSymbolModel->getPayloadModulation());
        const std::vector<const ISymbol *> *transmittedSymbols = transmissionSymbolModel->getSymbols();
        std::vector<const ISymbol *> *receivedSymbols = new std::vector<const ISymbol *>();
        for (auto& transmittedSymbols_i : *transmittedSymbols) {
            if (uniform(0, 1) < symbolErrorRate) {
                const ApskSymbol *transmittedSymbol = check_and_cast<const ApskSymbol *>(transmittedSymbols_i);
                ShortBitVector bits = modulation->demapToBitRepresentation(transmittedSymbol);
                int errorIndex = intuniform(0, bits.getSize() - 1);
                bits.setBit(errorIndex, !bits.getBit(errorIndex));
                const ApskSymbol *receivedSymbol = modulation->mapToConstellationDiagram(bits);
                receivedSymbols->push_back(receivedSymbol);
            }
            else
                receivedSymbols->push_back(transmittedSymbols_i);
        }
        return new ReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getPayloadSymbolLength(), transmissionSymbolModel->getPayloadSymbolRate(), receivedSymbols, symbolErrorRate);
    }
}

} // namespace physicallayer
} // namespace inet

