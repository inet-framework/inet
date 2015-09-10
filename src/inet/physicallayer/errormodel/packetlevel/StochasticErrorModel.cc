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

#include "inet/physicallayer/errormodel/packetlevel/StochasticErrorModel.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"

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

std::ostream& StochasticErrorModel::printToStream(std::ostream& stream, int level) const
{
    if (level >= PRINT_LEVEL_TRACE)
        stream << "StochasticErrorModel"
               << "packetErrorRate = " << packetErrorRate
               << "bitErrorRate = " << bitErrorRate
               << "symbolErrorRate = " << symbolErrorRate;
    return stream;
}

double StochasticErrorModel::computePacketErrorRate(const ISNIR *snir) const
{
    if (!isNaN(packetErrorRate))
        return packetErrorRate;
    else {
        double bitErrorRate = computeBitErrorRate(snir);
        const IReception *reception = snir->getReception();
        const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
        return 1.0 - pow(1.0 - bitErrorRate, flatTransmission->getPayloadBitLength());
    }
}

double StochasticErrorModel::computeBitErrorRate(const ISNIR *snir) const
{
    if (!isNaN(bitErrorRate))
        return bitErrorRate;
    else {
        // TODO: compute bit error rate based on symbol error rate and modulation
//        const IReception *reception = snir->getReception();
//        const NarrowbandTransmissionBase *narrowbandTransmission = check_and_cast<const NarrowbandTransmissionBase *>(reception->getTransmission());
//        const IModulation *modulation = narrowbandTransmission->getModulation();
//        double symbolErrorRate = computeSymbolErrorRate(snir);
        throw cRuntimeError("Not yet implemented");
    }
}

double StochasticErrorModel::computeSymbolErrorRate(const ISNIR *snir) const
{
    return symbolErrorRate;
}

} // namespace physicallayer

} // namespace inet

