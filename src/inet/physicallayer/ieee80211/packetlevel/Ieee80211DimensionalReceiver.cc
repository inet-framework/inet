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

#include "inet/physicallayer/base/packetlevel/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211DimensionalReceiver.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211DimensionalTransmission.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"

namespace inet {

using namespace ieee80211;

namespace physicallayer {

Define_Module(Ieee80211DimensionalReceiver);

Ieee80211DimensionalReceiver::Ieee80211DimensionalReceiver() :
    Ieee80211ReceiverBase()
{
}

std::ostream& Ieee80211DimensionalReceiver::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211DimensionalReceiver";
    return Ieee80211ReceiverBase::printToStream(stream, level);
}

bool Ieee80211DimensionalReceiver::computeIsReceptionPossible(const ITransmission *transmission) const
{
    const Ieee80211DimensionalTransmission *ieee80211Transmission = check_and_cast<const Ieee80211DimensionalTransmission *>(transmission);
    return NarrowbandReceiverBase::computeIsReceptionPossible(transmission) && modeSet->containsMode(ieee80211Transmission->getMode());
}

} // namespace physicallayer

} // namespace inet

