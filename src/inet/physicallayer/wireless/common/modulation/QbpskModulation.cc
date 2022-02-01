//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/QbpskModulation.h"

namespace inet {

namespace physicallayer {

const std::vector<ApskSymbol> QbpskModulation::constellation = {
    ApskSymbol(0, -1), ApskSymbol(0, 1)
};

const QbpskModulation QbpskModulation::singleton;

QbpskModulation::QbpskModulation() : MqamModulationBase(1, &constellation)
{
}

double QbpskModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const // TODO Review
{
    double ser = 0.5 * erfc(sqrt(snir * bandwidth.get() / bitrate.get()));
    ASSERT(0.0 <= ser && ser <= 1.0);
    return ser;
}

double QbpskModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    // http://en.wikipedia.org/wiki/Phase-shift_keying#Bit_error_rate
    double ber = calculateSER(snir, bandwidth, bitrate);
    ASSERT(0.0 <= ber && ber <= 1.0);
    return ber;
}

} // namespace physicallayer

} // namespace inet

