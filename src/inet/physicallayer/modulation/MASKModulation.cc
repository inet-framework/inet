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

#include "inet/physicallayer/modulation/MASKModulation.h"

namespace inet {

namespace physicallayer {

static std::vector<APSKSymbol> *createConstellation(double maxAmplitude, unsigned int codeWordSize)
{
    auto symbols = new std::vector<APSKSymbol>();
    unsigned int constellationSize = pow(2, codeWordSize);
    for (unsigned int i = 0; i < constellationSize; i++) {
        double amplitude = 2 * maxAmplitude / (constellationSize - 1) * i - maxAmplitude;
        symbols->push_back(APSKSymbol(amplitude));
    }
    return symbols;
}

MASKModulation::MASKModulation(double maxAmplitude, unsigned int codeWordSize) : APSKModulationBase(createConstellation(maxAmplitude, codeWordSize))
{
}

MASKModulation::~MASKModulation()
{
    delete constellation;
}

std::ostream& MASKModulation::printToStream(std::ostream& stream, int level) const
{
    stream << "MASKModulaiton";
    return APSKModulationBase::printToStream(stream, level);
}

double MASKModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not implemented yet");
}

double MASKModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not implemented yet");
}

} // namespace physicallayer

} // namespace inet

