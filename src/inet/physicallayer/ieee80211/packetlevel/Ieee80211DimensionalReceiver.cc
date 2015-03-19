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
    FlatReceiverBase(),
    modeSet(nullptr)
{
}

void Ieee80211DimensionalReceiver::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        carrierFrequency = Hz(CENTER_FREQUENCIES[par("channelNumber")]);
        modeSet = Ieee80211ModeSet::getModeSet(*par("opMode").stringValue());
    }
}

bool Ieee80211DimensionalReceiver::computeIsReceptionPossible(const ITransmission *transmission) const
{
    const Ieee80211DimensionalTransmission *ieee80211Transmission = check_and_cast<const Ieee80211DimensionalTransmission *>(transmission);
    return NarrowbandReceiverBase::computeIsReceptionPossible(transmission) && modeSet->containsMode(ieee80211Transmission->getMode());
}

} // namespace physicallayer

} // namespace inet

