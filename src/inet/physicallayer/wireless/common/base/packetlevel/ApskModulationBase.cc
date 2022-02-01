//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"

#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/DsssOqpsk16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/MpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/MqamModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam256Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"

namespace inet {

namespace physicallayer {

ApskModulationBase::ApskModulationBase(const std::vector<ApskSymbol> *constellation) :
    constellation(constellation),
    codeWordSize(log2(constellation->size())),
    constellationSize(constellation->size())
{
}

std::ostream& ApskModulationBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(constellationSize)
               << EV_FIELD(codeWordSize);
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
        // TODO avoid allocation
        return new MqamModulation(atoi(modulation + 5));
    else if (!strncmp("MPSK-", modulation, 5))
        // TODO avoid allocation
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
    // TODO Complete implementation: http://eprints.soton.ac.uk/354719/1/tvt-hanzo-2272640-proof.pdf
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

