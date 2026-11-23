// SPDX-License-Identifier: LGPL-3.0-or-later
import inet.common.TagBase;
namespace inet::ieee80211;

// Recorder-created observation copies only. This is never evidence authorizing
// production A-MPDU decoding, peer admission or Block Ack context.
class Ieee80211AmpduObservationTag extends inet::TagBase
{
    uint32_t reference;
    int memberIndex;
    int memberCount;
    uint8_t delimiterCrc;
}
