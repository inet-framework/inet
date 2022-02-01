//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/MqamModulation.h"

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

MqamModulation::MqamModulation(unsigned int codeWordSize) : MqamModulationBase(1 / sqrt(2 * (pow(2, codeWordSize) - 1) / 3), createConstellation(codeWordSize))
{
}

MqamModulation::~MqamModulation()
{
    delete constellation;
}

std::ostream& MqamModulation::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "MqamModulation";
    return ApskModulationBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

