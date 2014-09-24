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

#include "inet/physicallayer/common/FlatErrorModel.h"
#include "inet/physicallayer/base/FlatTransmissionBase.h"

namespace inet {

namespace physicallayer {

void FlatErrorModel::printToStream(std::ostream& stream) const
{
    stream << "Flat error model";
}

double FlatErrorModel::computePacketErrorRate(const IReception *reception, const IInterference *interference) const
{
    double bitErrorRate = computeBitErrorRate(reception, interference);
    if (bitErrorRate == 0.0)
        return 0.0;
    else {
        const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
        return 1.0 - pow(1.0 - bitErrorRate, flatTransmission->getPayloadBitLength());
    }
}

double FlatErrorModel::computeBitErrorRate(const IReception *reception, const IInterference *interference) const
{
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
    const IModulation *modulation = flatTransmission->getModulation();
    // TODO: compute SNIR
    double minSNIR = 0;
    throw cRuntimeError("TODO");
    return modulation->calculateBER(minSNIR, flatTransmission->getBandwidth().get(), flatTransmission->getBitrate().get());
}

double FlatErrorModel::computeSymbolErrorRate(const IReception *reception, const IInterference *interference) const
{
    throw cRuntimeError("Not implemented");
}

} // namespace physicallayer

} // namespace inet

