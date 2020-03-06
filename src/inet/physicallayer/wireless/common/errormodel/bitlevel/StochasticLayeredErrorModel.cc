//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/errormodel/bitlevel/StochasticLayeredErrorModel.h"

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {
namespace physicallayer {

Define_Module(StochasticLayeredErrorModel);

StochasticLayeredErrorModel::StochasticLayeredErrorModel() :
    packetErrorRate(NaN),
    bitErrorRate(NaN),
    symbolErrorRate(NaN)
{
}

void StochasticLayeredErrorModel::initialize(int stage)
{
    LayeredErrorModelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetErrorRate = par("packetErrorRate");
        bitErrorRate = par("bitErrorRate");
        symbolErrorRate = par("symbolErrorRate");
    }
}

std::ostream& StochasticLayeredErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "StochasticLayeredErrorModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << "packetErrorRate = " << packetErrorRate
               << "bitErrorRate = " << bitErrorRate
               << "symbolErrorRate = " << symbolErrorRate;
    return stream;
}

const IReceptionPacketModel *StochasticLayeredErrorModel::computePacketModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    throw cRuntimeError("Not yet implemented");
}

const IReceptionBitModel *StochasticLayeredErrorModel::computeBitModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    return LayeredErrorModelBase::computeBitModel(transmission, bitErrorRate);
}

const IReceptionSymbolModel *StochasticLayeredErrorModel::computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    return LayeredErrorModelBase::computeSymbolModel(transmission, symbolErrorRate);
}

const IReceptionSampleModel *StochasticLayeredErrorModel::computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    throw cRuntimeError("Not yet implemented");
}

} // namespace physicallayer
} // namespace inet

