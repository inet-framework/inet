// SPDX-License-Identifier: LGPL-3.0-or-later
import inet.common.TagBase;
namespace inet::ieee80211;

// Local metadata, never inferred from a received sender tag. MeshForwarding
// commits source identity; validated receive reconstructs the constituent count.
class Ieee80211MeshAggregationTag extends inet::TagBase
{
    bool pendingSourceIdentity = false;
    int msduCount = 1;
    int initialMpduLength = -1; // Bare MAC header/body/FCS octets; HCF writes once after actual completion.
}
