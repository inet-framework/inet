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
//

package inet.linklayer.ieee80211.mac.channelaccess;

import inet.linklayer.ieee80211.mac.contract.IContention;

//
// IEEE 802.11 Enhanced Distributed Channel Access Function
//
module Edcaf
{
    parameters:
        string accessCategory @enum("AC_BK","AC_BE","AC_VI","AC_VO");
        int aifsn = default(-1);
        int cwMin = default(-1);
        int cwMax = default(-1);

        string rateSelectionModule;
        string rxModule;
        string collisionControllerModule;

        @class(Edcaf);
        @display("i=block/queue");
        @signal[packetSentToPeer](type=inet::Packet);
        @signal[channelOwningChanged](type=bool);
        @statistic[packetSentToPeer](title="packet sent"; record=count, vector?; interpolationmode=none);
        @statistic[packetSentToPeerUnicast](title="packet sent: unicast"; source=ieee80211Unicast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetSentToPeerMulticast](title="packet sent: multicast"; source=ieee80211Multicast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetSentToPeerBroadcast](title="packet sent: broadcast "; source=ieee80211Broadcast(packetSentToPeer); record=count, vector?; interpolationmode=none);
        @statistic[packetSentToPeerWithRetry](title="packet sent: with retry"; source=ieee80211Retry(packetSentToPeer); record=count);
        @statistic[packetSentToPeerWithoutRetry](title="packet sent: without retry"; source=ieee80211NotRetry(packetSentToPeer); record=count);
        @statistic[channelOwningChanged](title="channel owning"; record=vector?; interpolationmode=sample-hold);

    submodules:
        contention: <default("Contention")> like IContention {
            parameters:
                @display("p=100,100");
        }
}

