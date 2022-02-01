//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/MpskModulation.h"

namespace inet {

namespace physicallayer {

static std::vector<ApskSymbol> *createConstellation(unsigned int codeWordSize)
{
    auto symbols = new std::vector<ApskSymbol>();
    unsigned int constellationSize = pow(2, codeWordSize);
    for (unsigned int i = 0; i < constellationSize; i++) {
        unsigned int gray = (i >> 1) ^ i;
        double alpha = 2 * M_PI * gray / constellationSize;
        symbols->push_back(ApskSymbol(cos(alpha), sin(alpha)));
    }
    return symbols;
}

MpskModulation::MpskModulation(unsigned int codeWordSize) : ApskModulationBase(createConstellation(codeWordSize))
{
}

MpskModulation::~MpskModulation()
{
    delete constellation;
}

std::ostream& MpskModulation::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "MPSKModulaiton";
    return ApskModulationBase::printToStream(stream, level);
}

double MpskModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    // http://www.dsplog.com/2008/05/18/bit-error-rate-for-16psk-modulation-using-gray-mapping/
    double ber = erfc(sqrt(snir) * sin(M_PI / constellationSize)) / codeWordSize;
    ASSERT(0.0 <= ber && ber <= 1.0);
    return ber;
}

double MpskModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    // http://www.dsplog.com/2008/03/18/symbol-error-rate-for-16psk/
    double ser = erfc(sqrt(snir) * sin(M_PI / constellationSize));
    ASSERT(0.0 <= ser && ser <= 1.0);
    return ser;
}

} // namespace physicallayer

} // namespace inet

