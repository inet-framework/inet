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
#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/errormodel/bitlevel/StochasticLayeredErrorModel.h"

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
    if (stage == INITSTAGE_LOCAL) {
        packetErrorRate = par("packetErrorRate");
        bitErrorRate = par("bitErrorRate");
        symbolErrorRate = par("symbolErrorRate");
    }
}

std::ostream& StochasticLayeredErrorModel::printToStream(std::ostream& stream, int level) const
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

