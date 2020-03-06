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

#include "inet/physicallayer/apskradio/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/base/bitlevel/LayeredErrorModelBase.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"

namespace inet {
namespace physicallayer {

void LayeredErrorModelBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        symbolCorruptionMode = par("symbolCorruptionMode");
}

const IReceptionPacketModel *LayeredErrorModelBase::computePacketModel(const LayeredTransmission *transmission, double packetErrorRate) const
{
    auto transmissionPacketModel = check_and_cast<const TransmissionPacketModel *>(transmission->getPacketModel());
    auto transmittedPacket = transmissionPacketModel->getPacket();
    auto receivedPacket = transmittedPacket->dup();
    if (packetErrorRate != 0 && uniform(0, 1) < packetErrorRate)
        receivedPacket->setBitError(true);
    receivedPacket->addTagIfAbsent<ErrorRateInd>()->setPacketErrorRate(packetErrorRate);
    return new ReceptionPacketModel(receivedPacket, transmissionPacketModel->getHeaderNetBitrate(), transmissionPacketModel->getDataNetBitrate());
}

const IReceptionBitModel *LayeredErrorModelBase::computeBitModel(const LayeredTransmission *transmission, double bitErrorRate) const
{
    const TransmissionBitModel *transmissionBitModel = check_and_cast<const TransmissionBitModel *>(transmission->getBitModel());
    if (bitErrorRate == 0)
        return new ReceptionBitModel(transmissionBitModel->getHeaderLength(), transmissionBitModel->getHeaderGrossBitrate(), transmissionBitModel->getDataLength(), transmissionBitModel->getDataGrossBitrate(), new BitVector(*transmissionBitModel->getAllBits()));
    else {
        BitVector *receivedBits = new BitVector(*transmissionBitModel->getAllBits());
        for (unsigned int i = 0; i < receivedBits->getSize(); i++)
            if (uniform(0, 1) < bitErrorRate)
                receivedBits->toggleBit(i);
        return new ReceptionBitModel(transmissionBitModel->getHeaderLength(), transmissionBitModel->getHeaderGrossBitrate(), transmissionBitModel->getDataLength(), transmissionBitModel->getDataGrossBitrate(), receivedBits);
    }
}

const IReceptionSymbolModel *LayeredErrorModelBase::computeSymbolModel(const LayeredTransmission *transmission, double symbolErrorRate) const
{
    auto transmissionSymbolModel = check_and_cast<const TransmissionSymbolModel *>(transmission->getSymbolModel());
    if (symbolErrorRate == 0)
        return new ReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getDataSymbolLength(), transmissionSymbolModel->getDataSymbolRate(), new std::vector<const ISymbol*>(*transmissionSymbolModel->getAllSymbols()));
    else {
        auto modulation = check_and_cast<const ApskModulationBase *>(transmissionSymbolModel->getDataModulation());
        auto transmittedSymbols = transmissionSymbolModel->getAllSymbols();
        int symbolCount = transmittedSymbols->size();
        auto receivedSymbols = new std::vector<const ISymbol *>(symbolCount);
        for (int i = 0; i < symbolCount; i++) {
            auto transmittedSymbol = check_and_cast<const ApskSymbol *>(transmittedSymbols->at(i));
            bool isCorruptSymbol = symbolErrorRate == 1 || (symbolErrorRate != 0 && uniform(0, 1) < symbolErrorRate);
            auto receivedSymbol = isCorruptSymbol ? computeCorruptSymbol(modulation, transmittedSymbol) : transmittedSymbol;
            receivedSymbols->at(i) = receivedSymbol;
        }
        return new ReceptionSymbolModel(transmissionSymbolModel->getHeaderSymbolLength(), transmissionSymbolModel->getHeaderSymbolRate(), transmissionSymbolModel->getDataSymbolLength(), transmissionSymbolModel->getDataSymbolRate(), receivedSymbols);
    }
}

const ISymbol *LayeredErrorModelBase::computeCorruptSymbol(const ApskModulationBase *modulation, const ApskSymbol *transmittedSymbol) const
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

