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

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmissionBase.h"

namespace inet {

namespace physicallayer {

Ieee80211TransmissionBase::Ieee80211TransmissionBase(const IIeee80211Mode *mode, const Ieee80211Channel *channel) :
    mode(mode),
    channel(channel)
{
}

std::ostream& Ieee80211TransmissionBase::printToStream(std::ostream& stream, int level) const
{
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", mode = " << printObjectToString(mode, level + 1)
               << ", channel = " << printObjectToString(channel, level + 1);
    return stream;
}

} // namespace physicallayer

} // namespace inet

