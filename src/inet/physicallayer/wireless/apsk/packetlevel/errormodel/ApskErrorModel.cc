//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/errormodel/ApskErrorModel.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskErrorModel);

std::ostream& ApskErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "ApskErrorModel";
}

double ApskErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computePacketErrorRate");
    double bitErrorRate = computeBitErrorRate(snir, part);
    if (bitErrorRate == 0.0)
        return 0.0;
    else if (bitErrorRate == 1.0)
        return 1.0;
    else {
        auto apskTransmission = check_and_cast<const ApskTransmission *>(snir->getReception()->getTransmission());
        double headerSuccessRate = pow(1.0 - bitErrorRate, apskTransmission->getHeaderLength().get<b>());
        double dataSuccessRate = pow(1.0 - bitErrorRate, apskTransmission->getDataLength().get<b>());
        switch (part) {
            case IRadioSignal::SIGNAL_PART_WHOLE:
                return 1.0 - headerSuccessRate * dataSuccessRate;
            case IRadioSignal::SIGNAL_PART_PREAMBLE:
                return 0;
            case IRadioSignal::SIGNAL_PART_HEADER:
                return 1.0 - headerSuccessRate;
            case IRadioSignal::SIGNAL_PART_DATA:
                return 1.0 - dataSuccessRate;
            default:
                throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
        }
    }
}

double ApskErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeBitErrorRate");
    const ApskTransmission *apskTransmission = check_and_cast<const ApskTransmission *>(snir->getReception()->getTransmission());
    const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(apskTransmission->getModulation());
    return modulation->calculateBER(getScalarSnir(snir), apskTransmission->getBandwidth(), apskTransmission->getBitrate());
}

double ApskErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeSymbolErrorRate");
    auto apskTransmission = check_and_cast<const ApskTransmission *>(snir->getReception()->getTransmission());
    const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(apskTransmission->getModulation());
    return modulation->calculateSER(getScalarSnir(snir), apskTransmission->getBandwidth(), apskTransmission->getBitrate());
}

} // namespace physicallayer

} // namespace inet

