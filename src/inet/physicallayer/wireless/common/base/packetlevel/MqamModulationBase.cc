//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/MqamModulationBase.h"

namespace inet {

namespace physicallayer {

MqamModulationBase::MqamModulationBase(double normalizationFactor, const std::vector<ApskSymbol> *constellation) :
    ApskModulationBase(constellation),
    normalizationFactor(normalizationFactor)
{
}

double MqamModulationBase::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    // http://en.wikipedia.org/wiki/Eb/N0
    double EbN0 = snir * bandwidth.get() / bitrate.get();
    double EsN0 = EbN0 * log2(constellationSize);
    // http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation#Rectangular_QAM
    double Psc = 2 * (1 - 1 / sqrt(constellationSize)) * 0.5 * erfc(1 / sqrt(2) * sqrt(3.0 / (constellationSize - 1) * EsN0));
    double ser = 1 - (1 - Psc) * (1 - Psc);
    ASSERT(0.0 <= ser && ser <= 1.0);
    return ser;
}

double MqamModulationBase::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    double EbN0 = snir * bandwidth.get() / bitrate.get();
    // http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation#Rectangular_QAM
    double ber = 4.0 / codeWordSize * (1 - 1 / sqrt(constellationSize)) * 0.5 * erfc(1 / sqrt(2) * sqrt(3.0 * codeWordSize / (constellationSize - 1) * EbN0));
    ASSERT(0.0 <= ber && ber <= 1.0);
    return ber;
}

} // namespace physicallayer

} // namespace inet

