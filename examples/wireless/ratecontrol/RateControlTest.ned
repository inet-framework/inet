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

package inet.examples.wireless.ratecontrol;

import inet.applications.ethernet.EtherAppCli;
import inet.common.queue.Sink;
import inet.environment.common.PhysicalEnvironment;
import inet.linklayer.ieee80211.Ieee80211Nic;
import inet.mobility.single.RectangleMobility;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


module SinkClient
{
    parameters:
        @networkNode();
        @labels(node);
        @display("i=device/cellphone");
        *.interfaceTableModule = default("");
    gates:
        input radioIn @directIn;
    submodules:
        sink: Sink {
            parameters:
                @display("p=169,124");
        }
        wlan: Ieee80211Nic {
            parameters:
                @display("p=89,225;q=queue");
                mgmtType = default("Ieee80211MgmtAdhoc");
        }
        mobility: RectangleMobility {
            parameters:
                @display("p=71,121");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> wlan.radioIn;
        wlan.upperLayerOut --> sink.in++;
}

module Client
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
        *.interfaceTableModule = "";
    gates:
        input radioIn @directIn;

    submodules:
        cli: EtherAppCli {
            parameters:
                registerSAP = false;
                @display("b=40,24;p=180,60,col");
        }
        wlan: Ieee80211Nic {
            parameters:
                @display("p=112,134;q=queue");
                mgmtType = default("Ieee80211MgmtAdhoc");
        }
        mobility: RectangleMobility {
            parameters:
                @display("p=50,141");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        cli.out --> wlan.upperLayerIn;
}

network RateControlTest
{
    parameters:
        int numCli;
        @display("b=297,203;bgb=1300,460");
    submodules:
        cliHost[numCli]: Client {
            parameters:
                @display("r=,,#707070");
        }
        sinkClient: SinkClient {
            parameters:
                @display("p=200,200;r=,,#707070");
        }
        environment: PhysicalEnvironment {
            parameters:
                @display("p=50,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=61,46");
        }
}

