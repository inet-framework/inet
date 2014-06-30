//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "DimensionalNoise.h"

namespace inet {

using namespace physicallayer;

W DimensionalNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    Argument start(DimensionSet::timeFreqDomain);
    Argument end(DimensionSet::timeFreqDomain);
    start.setTime(startTime);
    start.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
    end.setTime(endTime);
    end.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    return W(MappingUtils::findMax(*power));// TODO: W(MappingUtils::findMax(*power), start, end));
}


} // namespace inet


