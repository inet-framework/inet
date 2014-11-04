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

#include "inet/physicallayer/errormodel/FlatErrorModel.h"
#include "inet/physicallayer/base/FlatTransmissionBase.h"

namespace inet {

namespace physicallayer {

Define_Module(FlatErrorModel);

void FlatErrorModel::printToStream(std::ostream& stream) const
{
    stream << "FlatErrorModel";
}

double FlatErrorModel::computePacketErrorRate(const ISNIR *snir) const
{
    double bitErrorRate = computeBitErrorRate(snir);
    if (bitErrorRate == 0.0)
        return 0.0;
    else {
        const IReception *reception = snir->getReception();
        const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
        return 1.0 - pow(1.0 - bitErrorRate, flatTransmission->getPayloadBitLength());
    }
}

double FlatErrorModel::computeBitErrorRate(const ISNIR *snir) const
{
    const IReception *reception = snir->getReception();
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(reception->getTransmission());
    const IModulation *modulation = flatTransmission->getModulation();
    return modulation->calculateBER(snir->getMin(), flatTransmission->getBandwidth().get(), flatTransmission->getBitrate().get());
}

double FlatErrorModel::computeSymbolErrorRate(const ISNIR *snir) const
{
    return NaN;
}

} // namespace physicallayer

} // namespace inet

