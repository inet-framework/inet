//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_LAYEREDSCALARANALOGMODEL_H
#define __INET_LAYEREDSCALARANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ScalarAnalogModelBase.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredScalarAnalogModel : public ScalarAnalogModelBase
{
  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReception *computeReception(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

