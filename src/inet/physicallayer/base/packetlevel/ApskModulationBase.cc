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

#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/modulation/BpskModulation.h"
#include "inet/physicallayer/modulation/DsssOqpsk16Modulation.h"
#include "inet/physicallayer/modulation/MpskModulation.h"
#include "inet/physicallayer/modulation/MqamModulation.h"
#include "inet/physicallayer/modulation/Qam16Modulation.h"
#include "inet/physicallayer/modulation/Qam256Modulation.h"
#include "inet/physicallayer/modulation/Qam64Modulation.h"
#include "inet/physicallayer/modulation/QpskModulation.h"

namespace inet {

namespace physicallayer {

ApskModulationBase::ApskModulationBase(const std::vector<ApskSymbol> *constellation) :
    constellation(constellation),
    codeWordSize(log2(constellation->size())),
    constellationSize(constellation->size())
{
}

std::ostream& ApskModulationBase::printToStream(std::ostream& stream, int level) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", constellationSize = " << constellationSize
               << ", codeWordSize = " << codeWordSize;
    return stream;
}

const ApskModulationBase *ApskModulationBase::findModulation(const char *modulation)
{
    if (!strcmp("BPSK", modulation))
        return &BpskModulation::singleton;
    else if (!strcmp("QPSK", modulation))
        return &QpskModulation::singleton;
    else if (!strcmp("QAM-16", modulation))
        return &Qam16Modulation::singleton;
    else if (!strcmp("QAM-64", modulation))
        return &Qam64Modulation::singleton;
    else if (!strcmp("QAM-256", modulation))
        return &Qam256Modulation::singleton;
    else if (!strncmp("MQAM-", modulation, 5))
        // TODO: avoid allocation
        return new MqamModulation(atoi(modulation + 5));
    else if (!strncmp("MPSK-", modulation, 5))
        // TODO: avoid allocation
        return new MpskModulation(atoi(modulation + 5));
    else if (!strcmp(modulation, "DSSS-OQPSK-16"))
        return &DsssOqpsk16Modulation::singleton;
    else
        throw cRuntimeError("Unknown modulation = %s", modulation);
}

const ApskSymbol *ApskModulationBase::mapToConstellationDiagram(const ShortBitVector& symbol) const
{
    unsigned int decimalSymbol = symbol.toDecimal();
    if (decimalSymbol >= constellationSize)
        throw cRuntimeError("Unknown input: %d", decimalSymbol);
    return &constellation->at(decimalSymbol);
}

ShortBitVector ApskModulationBase::demapToBitRepresentation(const ApskSymbol *symbol) const
{
    // TODO: Complete implementation: http://eprints.soton.ac.uk/354719/1/tvt-hanzo-2272640-proof.pdf
    double symbolQ = symbol->real();
    double symbolI = symbol->imag();
    double minDist = DBL_MAX;
    int nearestNeighborIndex = -1;
    for (unsigned int i = 0; i < constellationSize; i++) {
        const ApskSymbol *constellationSymbol = &constellation->at(i);
        double cQ = constellationSymbol->real();
        double cI = constellationSymbol->imag();
        double dist = (symbolQ - cQ) * (symbolQ - cQ) + (symbolI - cI) * (symbolI - cI);
        if (dist < minDist) {
            minDist = dist;
            nearestNeighborIndex = i;
        }
    }
    ASSERT(nearestNeighborIndex != -1);
    return ShortBitVector(nearestNeighborIndex, codeWordSize);
}

} // namespace physicallayer

} // namespace inet

