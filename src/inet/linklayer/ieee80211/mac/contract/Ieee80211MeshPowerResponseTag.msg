// SPDX-License-Identifier: LGPL-3.0-or-later
import inet.common.TagBase;
import inet.linklayer.common.MacAddress;

namespace inet::ieee80211;

// Local acknowledgment proof carried only by the existing recipient Tx copy.
// Never serialized or interpreted on received packets as a wire identity.
class Ieee80211MeshPowerResponseTag extends TagBase
{
    MacAddress peer;
    uint16_t localLinkId = 0;
    uint16_t peerLinkId = 0;
    int64_t openingId = -1;
    simtime_t openingTime = -1;
    int64_t receiptId = -1;
    bool qualifyingBlockAck = false;
}
