//
// Copyright (C) 2006 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.examples.wireless.throughput;

import inet.linklayer.ieee80211.Ieee80211Interface;
import inet.mobility.static.StationaryMobility;
import inet.networklayer.common.InterfaceTable;
import inet.queueing.sink.PassivePacketSink;

//
// Well, this models a 802.11 Access Point with a Sink.
//
module WirelessAPWithSink
{
    parameters:
        @networkNode();
        @labels(node);
        @display("i=device/accesspoint");
        *.interfaceTableModule = "^.interfaceTable";
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;
    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,100;is=s");
        }
        sink: PassivePacketSink {
            parameters:
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                llc.typename = "Ieee80211Portal";
                mgmt.typename = "Ieee80211MgmtApSimplified";
                agent.typename = "";
        }
        mobility: StationaryMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> wlan.radioIn;
        wlan.upperLayerOut --> sink.in;
}

