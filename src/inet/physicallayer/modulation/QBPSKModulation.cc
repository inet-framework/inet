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

#include "inet/physicallayer/modulation/QBPSKModulation.h"

namespace inet {

namespace physicallayer {

const std::vector<APSKSymbol> QBPSKModulation::constellation = {
    APSKSymbol(0, -1), APSKSymbol(0, 1)
};

const QBPSKModulation QBPSKModulation::singleton;

QBPSKModulation::QBPSKModulation() : MQAMModulationBase(&constellation)
{
}

double QBPSKModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const // TODO: Review
{
    return 0.5 * erfc(sqrt(snir * bandwidth.get() / bitrate.get()));
}

double QBPSKModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    // http://en.wikipedia.org/wiki/Phase-shift_keying#Bit_error_rate
    return calculateSER(snir, bandwidth, bitrate);
}

} // namespace physicallayer

} // namespace inet

