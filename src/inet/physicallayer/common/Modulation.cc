//
// Copyright (C) 2006 Andras Varga
// Based on the Mobility Framework's SnrEval by Marc Loebbers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/common/Modulation.h"

namespace inet {

namespace physicallayer {

double BPSKModulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    return 0.5 * exp(-snir * bandwidth / bitrate);
}

double QAM16Modulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    return 0.5 * (1 - 1 / sqrt(pow(2.0, 4))) * erfc(snir * bandwidth / bitrate);
}

double QAM256Modulation::calculateBER(double snir, double bandwidth, double bitrate) const
{
    return 0.25 * (1 - 1 / sqrt(pow(2.0, 8))) * erfc(snir * bandwidth / bitrate);
}

} // namespace physicallayer

} // namespace inet

