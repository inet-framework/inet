//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_UNITDISKANALOGMODEL_H
#define __INET_UNITDISKANALOGMODEL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IAnalogModel.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the UnitDiskAnalogModel model, see the NED file for details.
 */
class INET_API UnitDiskAnalogModel : public cModule, public IAnalogModel
{
  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const IReception *computeReception(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const override;
    virtual const INoise *computeNoise(const IListening *listening, const IInterference *interference) const override;
    virtual const INoise *computeNoise(const IReception *reception, const INoise *noise) const override;
    virtual const ISnir *computeSNIR(const IReception *reception, const INoise *noise) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

