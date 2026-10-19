//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/contention/EdcaCollisionController.h"

#include "inet/linklayer/ieee80211/mac/channelaccess/Edcaf.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"

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
    edcafs[ac] = edcaf;
}

bool EdcaCollisionController::isInternalCollision(Edcaf *edcaf)
{
    simtime_t now = simTime();
    AccessCategory accessCategory = edcaf->getAccessCategory();
    auto hasFrames = [](const Edcaf *candidate) {
        return candidate->hasEligibleFrames();
    };
    // Deferred or expired DATA cannot initiate a TXOP or cause an internal collision.
    // Keep the winner's in-progress frame visible until its transmission completes.
    if (txStartTimes[accessCategory] == now && hasFrames(edcaf)) {
        for (int ac = accessCategory + 1; ac < 4; ac++) {
            if (txStartTimes[ac] == now && hasFrames(edcafs[ac])) {
                EV_WARN << "Internal collision detected between multiple access categories for the current simulation time.\n";
                return true;
            }
        }
    }
    return false;
}

} /* namespace ieee80211 */
} /* namespace inet */

