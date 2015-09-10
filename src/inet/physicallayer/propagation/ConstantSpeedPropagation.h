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

#ifndef __INET_CONSTANTSPEEDPROPAGATION_H
#define __INET_CONSTANTSPEEDPROPAGATION_H

#include "inet/physicallayer/base/packetlevel/PropagationBase.h"

namespace inet {

namespace physicallayer {

class INET_API ConstantSpeedPropagation : public PropagationBase
{
  protected:
    bool ignoreMovementDuringTransmission;
    bool ignoreMovementDuringPropagation;
    bool ignoreMovementDuringReception;

  protected:
    virtual void initialize(int stage) override;
    virtual const Coord computeArrivalPosition(const simtime_t startTime, const Coord startPosition, IMobility *mobility) const;

  public:
    ConstantSpeedPropagation();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual const IArrival *computeArrival(const ITransmission *transmission, IMobility *mobility) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_CONSTANTSPEEDPROPAGATION_H

