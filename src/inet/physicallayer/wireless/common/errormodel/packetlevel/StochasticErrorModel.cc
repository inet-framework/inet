//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/errormodel/packetlevel/StochasticErrorModel.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"

namespace inet {

namespace physicallayer {

Define_Module(StochasticErrorModel);

StochasticErrorModel::StochasticErrorModel() :
    packetErrorRate(NaN),
    bitErrorRate(NaN),
    symbolErrorRate(NaN)
{
}

void StochasticErrorModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        packetErrorRate = par("packetErrorRate");
        bitErrorRate = par("bitErrorRate");
        symbolErrorRate = par("symbolErrorRate");
    }
}

std::ostream& StochasticErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << "StochasticErrorModel"
               << "packetErrorRate = " << packetErrorRate
               << "bitErrorRate = " << bitErrorRate
               << "symbolErrorRate = " << symbolErrorRate;
    return stream;
}

double StochasticErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computePacketErrorRate");
    const IReception *reception = snir->getReception();
    if (!std::isnan(packetErrorRate)) {
        double factor = reception->getDuration(part) / reception->getDuration();
        return pow(packetErrorRate, factor);
    }
    else {
        const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
        double bitErrorRate = computeBitErrorRate(snir, part);
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

double StochasticErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeBitErrorRate");
    if (!std::isnan(bitErrorRate)) {
        return bitErrorRate;
    }
    else {
        // TODO compute bit error rate based on symbol error rate and modulation
//        const IReception *reception = snir->getReception();
//        const NarrowbandTransmissionBase *narrowbandTransmission = check_and_cast<const NarrowbandTransmissionBase *>(reception->getTransmission());
//        const IModulation *modulation = narrowbandTransmission->getModulation();
//        double symbolErrorRate = computeSymbolErrorRate(snir);
        throw cRuntimeError("Not yet implemented");
    }
}

double StochasticErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeSymbolErrorRate");
    return symbolErrorRate;
}

} // namespace physicallayer

} // namespace inet

