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


package inet.examples.old.hosttohost;

import inet.applications.ethernet.EtherAppCli;
import inet.common.queue.Sink;
import inet_old.base.NotificationBoard;
import inet_old.linklayer.ieee80211.Ieee80211NewNic;
import inet_old.linklayer.ieee80211.Ieee80211Nic;
import inet_old.mobility.CircleMobility;
import inet_old.mobility.NullMobility;
import inet_old.world.ChannelControlExtended;


module ThroughputClient
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
    gates:
        input radioIn @directIn;

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
        wlan: Ieee80211NewNic {
            parameters:
                @display("p=112,134;q=queue");
                mgmtType = "Ieee80211MgmtAdhoc";
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
        @networkNode();
    gates:
        input radioIn @directIn;

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70");
        }
        sink: Sink {
            parameters:
                @display("p=210,68,col");
        }
        wlan: Ieee80211NewNic {
            parameters:
                @display("p=120,158;q=queue");
                mgmtType = "Ieee80211MgmtAdhoc";
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
                @display("p=200,200;r=,,#707070");
        }
        srvHost: ThroughputServer {
            parameters:
                @display("p=350,350;r=,,#707070");
        }
        channelcontrol: ChannelControlExtended {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                @display("p=61,46");
        }
}

