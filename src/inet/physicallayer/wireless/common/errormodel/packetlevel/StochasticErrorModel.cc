//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/errormodel/packetlevel/StochasticErrorModel.h"

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
    ErrorModelBase::initialize(stage);
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
        auto transmission = reception->getTransmission();
        auto bitModel = transmission->getBitModel();
        double bitErrorRate = computeBitErrorRate(snir, part);
        double headerSuccessRate = pow(1.0 - bitErrorRate, bitModel->getHeaderLength().get<b>());
        double dataSuccessRate = pow(1.0 - bitErrorRate, bitModel->getDataLength().get<b>());
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

double StochasticErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const
{
    Enter_Method("computeBitErrorRate");
    checkRegionPart(part);
    if (!std::isnan(packetErrorRate)) {
        // the parameter is the packet error rate of the header and the data together:
        // a region has the share of its time, and each of its bits the same share of the region
        auto reception = snir->getReception();
        double share = (endTime - startTime) / (reception->getDuration(IRadioSignal::SIGNAL_PART_HEADER) + reception->getDuration(IRadioSignal::SIGNAL_PART_DATA));
        return -std::expm1(std::log1p(-packetErrorRate) * share / length.get<b>());
    }
    else
        return computeBitErrorRate(snir, part);
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

