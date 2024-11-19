//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee802154/packetlevel/errormodel/Ieee802154ErrorModel.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154Transmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154ErrorModel);

std::ostream& Ieee802154ErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "Ieee802154ErrorModel";
}

double Ieee802154ErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computePacketErrorRate");
    double bitErrorRate = computeBitErrorRate(snir, part);
    if (bitErrorRate == 0.0)
        return 0.0;
    else if (bitErrorRate == 1.0)
        return 1.0;
    else {
        auto ieee802154Transmission = check_and_cast<const Ieee802154Transmission *>(snir->getReception()->getTransmission());
        double headerSuccessRate = pow(1.0 - bitErrorRate, ieee802154Transmission->getHeaderLength().get<b>());
        double dataSuccessRate = pow(1.0 - bitErrorRate, ieee802154Transmission->getDataLength().get<b>());
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

double Ieee802154ErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeBitErrorRate");
    const Ieee802154Transmission *ieee802154Transmission = check_and_cast<const Ieee802154Transmission *>(snir->getReception()->getTransmission());
    const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(ieee802154Transmission->getModulation());
    return modulation->calculateBER(getScalarSnir(snir), ieee802154Transmission->getBandwidth(), ieee802154Transmission->getBitrate());
}

double Ieee802154ErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeSymbolErrorRate");
    auto ieee802154Transmission = check_and_cast<const Ieee802154Transmission *>(snir->getReception()->getTransmission());
    const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(ieee802154Transmission->getModulation());
    return modulation->calculateSER(getScalarSnir(snir), ieee802154Transmission->getBandwidth(), ieee802154Transmission->getBitrate());
}

} // namespace physicallayer

} // namespace inet

