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

#include "ImmediatePropagation.h"
#include "Arrival.h"

namespace inet {

namespace physicallayer {
Define_Module(ImmediatePropagation);

ImmediatePropagation::ImmediatePropagation() :
    PropagationBase()
{}

const IArrival *ImmediatePropagation::computeArrival(const ITransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;
    const Coord position = mobility->getCurrentPosition();
    const EulerAngles orientation = mobility->getCurrentAngularPosition();
    return new Arrival(0.0, 0.0, transmission->getStartTime(), transmission->getEndTime(), position, position, orientation, orientation);
}

void ImmediatePropagation::printToStream(std::ostream& stream) const
{
    stream << "immediate radio signal propagation, theoretical propagation speed = " << propagationSpeed;
}

} // namespace physicallayer
} // namespace inet

