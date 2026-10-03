//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.linklayer.common.MacAddress;

namespace inet;

// The management procedure's BSSID for Address3 of an outgoing management frame.
// It is independent of the receiver address and survives queuing/state changes.
class Ieee80211BssidReq extends TagBase
{
    MacAddress bssid;
}
