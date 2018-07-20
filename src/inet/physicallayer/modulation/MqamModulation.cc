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

#include "inet/physicallayer/modulation/MqamModulation.h"

namespace inet {

namespace physicallayer {

// http://www.gaussianwaves.com/2012/10/constructing-a-rectangular-constellation-for-16-qam/
static std::vector<ApskSymbol> *createConstellation(unsigned int codeWordSize)
{
    if (codeWordSize % 2 != 0)
        throw cRuntimeError("Odd code word size is not supported");
    auto symbols = new std::vector<ApskSymbol>();
    double normalizationFactor = 1 / sqrt(2 * (pow(2, codeWordSize) - 1) / 3);
    unsigned int constellationSize = pow(2, codeWordSize);
    unsigned int mask = pow(2, (codeWordSize / 2)) - 1;
    for (unsigned int i = 0; i < constellationSize; i++) {
        unsigned int column = i & mask;
        unsigned int row = i >> (codeWordSize / 2);
        unsigned int grayColumn = (column >> 1) ^ column;
        unsigned int grayRow = (row >> 1) ^ row;
        double x = -mask + 2 * grayColumn;
        double y = -mask + 2 * grayRow;
        symbols->push_back(normalizationFactor * ApskSymbol(x, y));
    }
    return symbols;
}

MqamModulation::MqamModulation(unsigned int codeWordSize) : MqamModulationBase(createConstellation(codeWordSize))
{
}

MqamModulation::~MqamModulation()
{
    delete constellation;
}

std::ostream& MqamModulation::printToStream(std::ostream& stream, int level) const
{
    stream << "MqamModulation";
    return ApskModulationBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

