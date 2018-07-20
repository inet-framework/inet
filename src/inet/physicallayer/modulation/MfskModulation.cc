//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/physicallayer/modulation/MfskModulation.h"

namespace inet {

namespace physicallayer {

MfskModulation::MfskModulation(unsigned int codeWordSize) :
    codeWordSize(codeWordSize)
{
}

std::ostream& MfskModulation::printToStream(std::ostream& stream, int level) const
{
    return stream << "MFSKModulaiton";
}

double MfskModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not implemented yet");
}

double MfskModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not implemented yet");
}

} // namespace physicallayer

} // namespace inet

