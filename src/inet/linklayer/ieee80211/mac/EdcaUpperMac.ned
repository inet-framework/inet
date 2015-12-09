//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

package inet.linklayer.ieee80211.mac;

//
// UpperMac for EDCA (802.11e QoS mode).
//
// Note: Access Category comes from the TID fields of QoS Data frames.
// Non-QoS Data frames are always transmitted in the AC_BE, and management
// frames in the AC_VO category.
//
// Currently TID is filled in by the Mgmt modules encapsulate higher-layer packets
// from the 802.1d User Priority (UP) field of the Ieee802Ctrl control info.
// The best way to assign an UP is to configure a classifier (IIeee8021dQoSClassifier)
// in Ieee80211Nic.
//
simple EdcaUpperMac like IUpperMac
{
    parameters:
        string macModule;
        string rxModule;
        string txModule;
        string firstContentionModule;
        string rateSelectionModule;
        string rateControlModule;
        string statisticsModule;

        int rtsThreshold @unit("B") = default(2346B); // longer frames will be sent using RTS/CTS
        int shortRetryLimit = default(-1); // maximum number of retries per frame, -1 means default
        int longRetryLimit = default(-1); // maximum number of retries for frames sent via RTS/CTS, -1 means default
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)
        int maxQueueSize = default(14); // max queue length per access category
        bool useFullAckTimeout = default(false); // when true wait for the full ack frame to arrive instead of waiting for RX start

        double sifsTime @unit("s") = default(-1s); // SIFS time, -1 means default (mode dependent)
        double slotTime @unit("s") = default(-1s); // slot time, -1 means default (mode dependent)

        int aifsn0 = default(-1); // AIFSN for AC_BK, -1 means default
        int aifsn1 = default(-1); // AIFSN for AC_BE, -1 means default
        int aifsn2 = default(-1); // AIFSN for AC_VI, -1 means default
        int aifsn3 = default(-1); // AIFSN for AC_VO, -1 means default

        double txopLimit0 @unit(s) = default(-1s);  // TXOP limit for AC_BK, -1 means default
        double txopLimit1 @unit(s) = default(-1s);  // TXOP limit for AC_BE, -1 means default
        double txopLimit2 @unit(s) = default(-1s);  // TXOP limit for AC_VI, -1 means default
        double txopLimit3 @unit(s) = default(-1s);  // TXOP limit for AC_VO, -1 means default

        int cwMin0 = default(-1); // minimum contention window for frames in AC_BK, -1 means default
        int cwMin1 = default(-1); // minimum contention window for frames in AC_BE, -1 means default
        int cwMin2 = default(-1); // minimum contention window for frames in AC_VI, -1 means default
        int cwMin3 = default(-1); // minimum contention window for frames in AC_VO, -1 means default

        int cwMax0 = default(-1); // maximum contention window for frames in AC_BK, -1 means default
        int cwMax1 = default(-1); // maximum contention window for frames in AC_BE, -1 means default
        int cwMax2 = default(-1); // maximum contention window for frames in AC_VI, -1 means default
        int cwMax3 = default(-1); // maximum contention window for frames in AC_VO, -1 means default

        int cwMulticast0 = default(-1); // contention window for multicast frames in AC_BK, -1 means default
        int cwMulticast1 = default(-1); // contention window for multicast frames in AC_BE, -1 means default
        int cwMulticast2 = default(-1); // contention window for multicast frames in AC_VI, -1 means default
        int cwMulticast3 = default(-1); // contention window for multicast frames in AC_VO, -1 means default

        string fragmenterClass = default("BasicFragmentation");
        string reassemblyClass = default("BasicReassembly");

        @display("i=block/layer");
}
