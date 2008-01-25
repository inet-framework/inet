//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package org.omnetpp.inet.examples.Wireless.HostToHost;

import org.omnetpp.inet.Applications.Ethernet.EtherAppCli;
import org.omnetpp.inet.Base.NotificationBoard;
import org.omnetpp.inet.Base.Sink;
import org.omnetpp.inet.Mobility.CircleMobility;
import org.omnetpp.inet.Mobility.NullMobility;
import org.omnetpp.inet.NetworkInterfaces.Ieee80211.Ieee80211NicSTASimplified;
import org.omnetpp.inet.Nodes.Wireless.WirelessAPSimplified;
import org.omnetpp.inet.World.ChannelControl;


module ThroughputClient
{
    parameters:
        @display("i=device/wifilaptop");
    gates:
        input radioIn;

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=52,70");
        }
        cli: EtherAppCli {
            parameters:
                registerSAP = false;
                @display("b=40,24;p=180,60,col");
        }
        wlan: Ieee80211NicSTASimplified {
            parameters:
                @display("p=112,134;q=queue");
        }
        mobility: CircleMobility {
            parameters:
                @display("p=50,141");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        cli.out --> wlan.uppergateIn;
}

module ThroughputServer
{
    parameters:
        @display("i=device/wifilaptop");
    gates:
        input radioIn;

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70");
        }
        sink: Sink {
            parameters:
                @display("p=210,68,col");
        }
        wlan: Ieee80211NicSTASimplified {
            parameters:
                @display("p=120,158;q=queue");
        }
        mobility: NullMobility {
            parameters:
                @display("p=50,141");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        sink.in++ <-- wlan.uppergateOut;
}

network Throughput
{
    parameters:
        int numCli;
        double playgroundSizeX;
        double playgroundSizeY;
        @display("b=297,203");
    submodules:
        cliHost[numCli]: ThroughputClient {
            parameters:
                @display("r=,,#707070");
        }
        srvHost: ThroughputServer {
            parameters:
                @display("p=350,350;r=,,#707070");
        }
        ap: WirelessAPSimplified {
            parameters:
                @display("p=200,200;r=,,#707070");
        }
        channelcontrol: ChannelControl {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                @display("p=61,46");
        }
    connections allowunconnected:
}

