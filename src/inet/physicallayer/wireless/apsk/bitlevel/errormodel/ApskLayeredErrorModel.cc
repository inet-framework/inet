//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/bitlevel/errormodel/ApskLayeredErrorModel.h"

#include "inet/physicallayer/wireless/common/modulation/ApskSymbol.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarTransmissionAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskLayeredErrorModel);

ApskLayeredErrorModel::ApskLayeredErrorModel()
{
}

std::ostream& ApskLayeredErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "LayeredApskErrorModel";
}

const IReceptionPacketModel *ApskLayeredErrorModel::computePacketModel(const ISnir *snir) const
{
    auto transmission = snir->getReception()->getTransmission();
    const ITransmissionBitModel *bitModel = transmission->getBitModel();
    const ScalarTransmissionAnalogModel *analogModel = check_and_cast<const ScalarTransmissionAnalogModel *>(transmission->getAnalogModel());
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

const IReceptionBitModel *ApskLayeredErrorModel::computeBitModel(const ISnir *snir) const
{
    auto transmission = snir->getReception()->getTransmission();
    const ITransmissionBitModel *bitModel = transmission->getBitModel();
    const ScalarTransmissionAnalogModel *analogModel = check_and_cast<const ScalarTransmissionAnalogModel *>(transmission->getAnalogModel());
    const IModulation *modulation = transmission->getSymbolModel()->getDataModulation();
    double bitErrorRate = modulation->calculateBER(snir->getMin(), analogModel->getBandwidth(), bitModel->getDataGrossBitrate());
    return LayeredErrorModelBase::computeBitModel(transmission, bitErrorRate);
}

const IReceptionSymbolModel *ApskLayeredErrorModel::computeSymbolModel(const ISnir *snir) const
{
    auto transmission = snir->getReception()->getTransmission();
    const IModulation *modulation = transmission->getSymbolModel()->getDataModulation();
    const ScalarTransmissionAnalogModel *analogModel = check_and_cast<const ScalarTransmissionAnalogModel *>(transmission->getAnalogModel());
    const ITransmissionBitModel *bitModel = transmission->getBitModel();
    double symbolErrorRate = modulation->calculateSER(snir->getMin(), analogModel->getBandwidth(), bitModel->getDataGrossBitrate());
    return LayeredErrorModelBase::computeSymbolModel(transmission, symbolErrorRate);
}

const IReceptionSampleModel *ApskLayeredErrorModel::computeSampleModel(const ISnir *snir) const
{
    throw cRuntimeError("Not yet implemented");
}

} // namespace physicallayer

} // namespace inet

