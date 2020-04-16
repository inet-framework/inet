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

#include "inet/physicallayer/modulation/BpskModulation.h"

namespace inet {

namespace physicallayer {

const std::vector<ApskSymbol> BpskModulation::constellation = {
    ApskSymbol(-1, 0), ApskSymbol(1, 0)
};

const BpskModulation BpskModulation::singleton;

BpskModulation::BpskModulation() : MqamModulationBase(&constellation)
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

