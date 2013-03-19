//
// Copyright (C) 2013 OpenSim Ltd.
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
// author: Zoltan Bojthe
//

package inet.linklayer.idealwireless;


//
// Implements a dummy MAC layer for use in ~IdealWirelessNic.
//
// This module only provides encapsulation/decapsulation (into IdealAirFrame), 
// but no real medium access protocol.
//
// Higher layer packets should have Ieee802Ctrl attached to them as
// control info.
//
// See ~IdealWirelessNic for rationale and details.
//
simple IdealWirelessMac
{
    parameters:
        string address = default("auto");   // MAC address as hex string (12 hex digits), or
                                            // "auto". "auto" values will be replaced by
                                            // a generated MAC address in init stage 0.
        double bitrate @unit("bps");
        int mtu @unit("B") = default(4470B);
        int headerLength @unit("B");        // IdealWirelessFrame.byteLength = headerLength + packetFromUpperLayer.byteLength
        bool promiscuous = default(false);
        @display("i=block/rxtx");

        @signal[packetSentToLower](type=IdealWirelessFrame);
        @signal[packetReceivedFromLower](type=IdealWirelessFrame);
        @signal[packetSentToUpper](type=cPacket);   // cPacket with Ieee802Ctrl ctrlinfo
        @signal[packetReceivedFromUpper](type=cPacket);   // cPacket with Ieee802Ctrl ctrlinfo
        @signal[dropPkNotForUs](type=IdealWirelessFrame);

        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentDownPk](title="packets sent to lower layer"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromHL](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromLL](title="packets received from lower layer"; source=packetReceivedFromLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkNotForUs](title="packets dropped/not for us"; source=dropPkNotForUs; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input upperLayerIn @labels(Ieee802Ctrl);        // should connect to output gate of an IPassiveQueue module
        output upperLayerOut @labels(Ieee802Ctrl);
        input lowerLayerIn @labels(IdealWirelessFrame);
        output lowerLayerOut @labels(IdealWirelessFrame);
}

