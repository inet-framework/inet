//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.linklayer.common.MacAddress;

namespace inet::ieee80211;

// MAC reports a rejected Class-3 MPDU through the management service boundary.
// The original transmitter is captured before any A-MSDU address rewriting.
message Ieee80211Class3FrameInd
{
    MacAddress transmitterAddress;
    bool individuallyAddressed;
}
