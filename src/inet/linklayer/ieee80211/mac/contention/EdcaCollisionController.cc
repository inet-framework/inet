//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
// 

#include "inet/linklayer/ieee80211/mac/channelaccess/Edcaf.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/contention/EdcaCollisionController.h"

namespace inet {
namespace ieee80211 {

Define_Module(EdcaCollisionController);

void EdcaCollisionController::initialize()
{
    for (int ac = 0; ac < 4; ac++) {
        txStartTimes[ac] = -1;
        WATCH(txStartTimes[ac]);
    }
}

void EdcaCollisionController::expectedChannelAccess(Edcaf *edcaf, simtime_t time)
{
    auto ac = edcaf->getAccessCategory();
    EV_INFO << "The expected channel access of the " << printAccessCategory(ac) << " queue is: " << time << std::endl;
    txStartTimes[ac] = time;
}

bool EdcaCollisionController::isInternalCollision(Edcaf *edcaf)
{
    simtime_t now = simTime();
    AccessCategory accessCategory = edcaf->getAccessCategory();
    if (txStartTimes[accessCategory] == now) {
        for (int ac = accessCategory + 1; ac < 4; ac++) {
            if (txStartTimes[ac] == now) {
                EV_WARN << "Internal collision detected between multiple access categories for the current simulation time.\n";
                return true;
            }
        }
    }
    return false;
}

} /* namespace ieee80211 */
} /* namespace inet */
