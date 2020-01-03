//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "inet/physicallayer/modulation/QbpskModulation.h"

namespace inet {

namespace physicallayer {

const std::vector<ApskSymbol> QbpskModulation::constellation = {
    ApskSymbol(0, -1), ApskSymbol(0, 1)
};

const QbpskModulation QbpskModulation::singleton;

QbpskModulation::QbpskModulation() : MqamModulationBase(&constellation)
{
}

double QbpskModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const // TODO: Review
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

