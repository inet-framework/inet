//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

