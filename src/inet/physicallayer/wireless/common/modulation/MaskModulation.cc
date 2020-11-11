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

#include "inet/physicallayer/wireless/common/modulation/MaskModulation.h"

namespace inet {

namespace physicallayer {

static std::vector<ApskSymbol> *createConstellation(double maxAmplitude, unsigned int codeWordSize)
{
    auto symbols = new std::vector<ApskSymbol>();
    unsigned int constellationSize = pow(2, codeWordSize);
    for (unsigned int i = 0; i < constellationSize; i++) {
        double amplitude = 2 * maxAmplitude / (constellationSize - 1) * i - maxAmplitude;
        symbols->push_back(ApskSymbol(amplitude));
    }
    return symbols;
}

MaskModulation::MaskModulation(double maxAmplitude, unsigned int codeWordSize) : ApskModulationBase(createConstellation(maxAmplitude, codeWordSize))
{
}

MaskModulation::~MaskModulation()
{
    delete constellation;
}

std::ostream& MaskModulation::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "MASKModulaiton";
    return ApskModulationBase::printToStream(stream, level);
}

double MaskModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not implemented yet");
}

double MaskModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not implemented yet");
}

} // namespace physicallayer

} // namespace inet

