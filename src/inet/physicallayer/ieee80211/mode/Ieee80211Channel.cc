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

#include "inet/physicallayer/ieee80211/mode/Ieee80211Channel.h"

namespace inet {

namespace physicallayer {

Ieee80211Channel::Ieee80211Channel(const IIeee80211Band *band, int channelNumber) :
    band(band),
    channelNumber(channelNumber)
{
}

std::ostream& Ieee80211Channel::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211Channel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", band = " << printObjectToString(band, level + 1);
    if (level <= PRINT_LEVEL_INFO)
        stream << ", channelNumber = " << channelNumber;
    return stream;
}

} // namespace physicallayer

} // namespace inet

