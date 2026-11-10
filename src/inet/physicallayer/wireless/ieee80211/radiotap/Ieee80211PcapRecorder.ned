// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.physicallayer.wireless.ieee80211.radiotap;
import inet.common.packet.recorder.PcapRecorder;

// Opt-in WLAN observation adapter: one record per complete A-MPDU payload member.
// Ordinary frames retain PcapRecorder filtering, conversion and file behavior.
simple Ieee80211PcapRecorder extends PcapRecorder
{
    parameters:
        @class(Ieee80211PcapRecorder);
}
