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

#include "inet/physicallayer/dimensional/DimensionalReceiver.h"
#include "inet/physicallayer/dimensional/DimensionalReception.h"
#include "inet/physicallayer/dimensional/DimensionalNoise.h"
#include "inet/physicallayer/dimensional/DimensionalSNIR.h"
#include "inet/physicallayer/common/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(DimensionalReceiver);

DimensionalReceiver::DimensionalReceiver() :
    FlatReceiverBase()
{
}

void DimensionalReceiver::printToStream(std::ostream& stream) const
{
    stream << "DimensionalReceiver, ";
    FlatReceiverBase::printToStream(stream);
}

} // namespace physicallayer

} // namespace inet

