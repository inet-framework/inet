//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/base/bitlevel/LayeredErrorModelBase.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKSymbol.h"

namespace inet {

namespace physicallayer {

const IReceptionPacketModel *LayeredErrorModelBase::computePacketModel(const LayeredTransmission *transmission, double packetErrorRate) const
{
    const TransmissionPacketModel *transmissionPacketModel = check_and_cast<const TransmissionPacketModel *>(transmission->getPacketModel());
    const cPacket* packet = transmissionPacketModel->getPacket();
    if (packetErrorRate == 0)
        return new ReceptionPacketModel(packet, new BitVector(*transmissionPacketModel->getSerializedPacket()), transmissionPacketModel->getBitrate(), packetErrorRate, true);
    else {
        if (uniform(0, 1) < packetErrorRate)
            return new const ReceptionPacketModel(packet, new BitVector(), transmissionPacketModel->getBitrate(), packetErrorRate, false);
        else
            return new const ReceptionPacketModel(packet, new BitVector(*transmissionPacketModel->getSerializedPacket()), transmissionPacketModel->getBitrate(), packetErrorRate, true);
    }
}

const IReceptionBitModel *LayeredErrorModelBase::computeBitModel(const LayeredTransmission *transmission, double bitErrorRate) const
{
    const TransmissionBitModel *transmissionBitModel = check_and_cast<const TransmissionBitModel *>(transmission->getBitModel());
    if (bitErrorRate == 0)
        return new const ReceptionBitModel(transmissionBitModel->getHeaderBitLength(), transmissionBitModel->getHeaderBitRate(), transmissionBitModel->getPayloadBitLength(), transmissionBitModel->getPayloadBitRate(), new BitVector(*transmissionBitModel->getBits()));
    else {
        BitVector *receivedBits = new BitVector(*transmissionBitModel->getBits());
        for (unsigned int i = 0; i < receivedBits->getSize(); i++) {
            if (uniform(0, 1) < bitErrorRate)
                receivedBits->toggleBit(i);
        }
        return new const ReceptionBitModel(transmissionBitModel->getHeaderBitLength(), transmissionBitModel->getHeaderBitRate(), transmissionBitModel->getPayloadBitLength(), transmissionBitModel->getPayloadBitRate(), receivedBits);
    }
}

const IReceptionSymbolModel *LayeredErrorModelBase::computeSymbolModel(const LayeredTransmission *transmission, double symbolErrorRate) const
{
    if (symbolErrorRate == 0) {
        const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
        return new ReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getPayloadSymbolLength(), transmissionSymbolModel->getPayloadSymbolRate(), new std::vector<const ISymbol*>(*transmissionSymbolModel->getSymbols()));
    }
    else {
        const TransmissionSymbolModel *transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
        const APSKModulationBase *modulation = check_and_cast<const APSKModulationBase *>(transmissionSymbolModel->getPayloadModulation());
        const std::vector<const ISymbol*> *transmittedSymbols = transmissionSymbolModel->getSymbols();
        std::vector<const ISymbol*> *receivedSymbols = new std::vector<const ISymbol *>();
        for (unsigned int i = 0; i < transmittedSymbols->size(); i++) {
            if (uniform(0, 1) < symbolErrorRate) {
                const APSKSymbol *transmittedSymbol = check_and_cast<const APSKSymbol *>(transmittedSymbols->at(i));
                ShortBitVector bits = modulation->demapToBitRepresentation(transmittedSymbol);
                int errorIndex = intuniform(0, bits.getSize() - 1);
                bits.setBit(errorIndex, !bits.getBit(errorIndex));
                const APSKSymbol *receivedSymbol = modulation->mapToConstellationDiagram(bits);
                receivedSymbols->push_back(receivedSymbol);
            }
            else
                receivedSymbols->push_back(transmittedSymbols->at(i));
        }
        return new ReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getPayloadSymbolLength(), transmissionSymbolModel->getPayloadSymbolRate(), receivedSymbols);
    }
}

} // namespace physicallayer

} // namespace inet

