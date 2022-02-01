//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/errormodel/ApskErrorModel.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"

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
        const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(snir->getReception()->getTransmission());
        double headerSuccessRate = pow(1.0 - bitErrorRate, b(flatTransmission->getHeaderLength()).get());
        double dataSuccessRate = pow(1.0 - bitErrorRate, b(flatTransmission->getDataLength()).get());
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
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(snir->getReception()->getTransmission());
    const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(flatTransmission->getModulation());
    return modulation->calculateBER(getScalarSnir(snir), flatTransmission->getBandwidth(), flatTransmission->getBitrate());
}

double ApskErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeSymbolErrorRate");
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(snir->getReception()->getTransmission());
    const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(flatTransmission->getModulation());
    return modulation->calculateSER(getScalarSnir(snir), flatTransmission->getBandwidth(), flatTransmission->getBitrate());
}

} // namespace physicallayer

} // namespace inet

