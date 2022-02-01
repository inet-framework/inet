//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"

namespace inet {

namespace physicallayer {

const std::vector<ApskSymbol> BpskModulation::constellation = {
    ApskSymbol(-1, 0), ApskSymbol(1, 0)
};

const BpskModulation BpskModulation::singleton;

BpskModulation::BpskModulation() : MqamModulationBase(1, &constellation)
{
}

double BpskModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    // https://en.wikipedia.org/wiki/Eb/N0#Relation_to_carrier-to-noise_ratio
    double ser = 0.5 * erfc(sqrt(snir * bandwidth.get() / bitrate.get()));
    ASSERT(0.0 <= ser && ser <= 1.0);
    return ser;
}

double BpskModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    // http://en.wikipedia.org/wiki/Phase-shift_keying#Bit_error_rate
    double ber = calculateSER(snir, bandwidth, bitrate);
    ASSERT(0.0 <= ber && ber <= 1.0);
    return ber;
}

} // namespace physicallayer

} // namespace inet

