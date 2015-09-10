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

#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKSymbol.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKPhyFrame_m.h"
#include "inet/physicallayer/apskradio/bitlevel/errormodel/APSKLayeredErrorModel.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKLayeredErrorModel);

APSKLayeredErrorModel::APSKLayeredErrorModel()
{
}

std::ostream& APSKLayeredErrorModel::printToStream(std::ostream& stream, int level) const
{
    return stream << "LayeredAPSKErrorModel";
}

const IReceptionPacketModel *APSKLayeredErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    const ITransmissionBitModel* bitModel = transmission->getBitModel();
    const ScalarTransmissionSignalAnalogModel *analogModel = check_and_cast<const ScalarTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    const IModulation* modulation = transmission->getSymbolModel()->getPayloadModulation();
    double grossBitErrorRate = modulation->calculateBER(snir->getMin(), analogModel->getBandwidth(), bitModel->getPayloadBitRate());
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

const IReceptionBitModel *APSKLayeredErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    const ITransmissionBitModel* bitModel = transmission->getBitModel();
    const ScalarTransmissionSignalAnalogModel *analogModel = check_and_cast<const ScalarTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    const IModulation* modulation = transmission->getSymbolModel()->getPayloadModulation();
    double bitErrorRate = modulation->calculateBER(snir->getMin(), analogModel->getBandwidth(), bitModel->getPayloadBitRate());
    return LayeredErrorModelBase::computeBitModel(transmission, bitErrorRate);
}

const IReceptionSymbolModel *APSKLayeredErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    const IModulation* modulation = transmission->getSymbolModel()->getPayloadModulation();
    const ScalarTransmissionSignalAnalogModel *analogModel = check_and_cast<const ScalarTransmissionSignalAnalogModel *>(transmission->getAnalogModel());
    const ITransmissionBitModel* bitModel = transmission->getBitModel();
    double symbolErrorRate = modulation->calculateSER(snir->getMin(), analogModel->getBandwidth(), bitModel->getPayloadBitRate());
    return LayeredErrorModelBase::computeSymbolModel(transmission, symbolErrorRate);
}

const IReceptionSampleModel *APSKLayeredErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    throw cRuntimeError("Not yet implemented");
}

} // namespace physicallayer

} // namespace inet

