//
// Copyright (C) 2013 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.examples.pim.iptv;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherLink;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;


//
// A generated network with grid topology.
//
network Network
{
    parameters:
        *.eth[*].bitrate = 100 Mbps;
    submodules:
        R00: MulticastRouter {
            @display("p=400,100");
            gates:
                ethg[4];
        }
        R01: MulticastRouter {
            @display("p=400,190");
            gates:
                ethg[4];
        }
        R02: MulticastRouter {
            @display("p=400,280");
            gates:
                ethg[4];
        }
        R03: MulticastRouter {
            @display("p=400,370");
            gates:
                ethg[4];
        }

        R10: MulticastRouter {
            @display("p=550,100");
            gates:
                ethg[4];
        }
        R11: MulticastRouter {
            @display("p=550,190");
            gates:
                ethg[4];
        }
        R12: MulticastRouter {
            @display("p=550,280");
            gates:
                ethg[4];
        }
        R13: MulticastRouter {
            @display("p=550,370");
            gates:
                ethg[4];
        }

        R20: MulticastRouter {
            @display("p=700,100");
            gates:
                ethg[4];
        }
        R21: MulticastRouter {
            @display("p=700,190");
            gates:
                ethg[4];
        }
        R22: MulticastRouter {
            @display("p=700,280");
            gates:
                ethg[4];
        }
        R23: MulticastRouter {
            @display("p=700,370");
            gates:
                ethg[4];
        }

        R30: MulticastRouter {
            @display("p=850,100");
            gates:
                ethg[4];
        }
        R31: MulticastRouter {
            @display("p=850,190");
            gates:
                ethg[4];
        }
        R32: MulticastRouter {
            @display("p=850,280");
            gates:
                ethg[4];
        }
        R33: MulticastRouter {
            @display("p=850,370");
            gates:
                ethg[4];
        }


        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        client: StandardHost {
            @display("p=1000,370");
        }
        server: StandardHost {
            @display("p=250,100");
        }
    connections allowunconnected:
        R00.ethg[1] <--> EtherLink <--> R10.ethg[0];
        R00.ethg[3] <--> EtherLink <--> R01.ethg[2];
        R01.ethg[1] <--> EtherLink <--> R11.ethg[0];
        R01.ethg[3] <--> EtherLink <--> R02.ethg[2];
        R02.ethg[1] <--> EtherLink <--> R12.ethg[0];
        R02.ethg[3] <--> EtherLink <--> R03.ethg[2];
        R03.ethg[1] <--> EtherLink <--> R13.ethg[0];

        R10.ethg[1] <--> EtherLink <--> R20.ethg[0];
        R10.ethg[3] <--> EtherLink <--> R11.ethg[2];
        R11.ethg[1] <--> EtherLink <--> R21.ethg[0];
        R11.ethg[3] <--> EtherLink <--> R12.ethg[2];
        R12.ethg[1] <--> EtherLink <--> R22.ethg[0];
        R12.ethg[3] <--> EtherLink <--> R13.ethg[2];
        R13.ethg[1] <--> EtherLink <--> R23.ethg[0];

        R20.ethg[1] <--> EtherLink <--> R30.ethg[0];
        R20.ethg[3] <--> EtherLink <--> R21.ethg[2];
        R21.ethg[1] <--> EtherLink <--> R31.ethg[0];
        R21.ethg[3] <--> EtherLink <--> R22.ethg[2];
        R22.ethg[1] <--> EtherLink <--> R32.ethg[0];
        R22.ethg[3] <--> EtherLink <--> R23.ethg[2];
        R23.ethg[1] <--> EtherLink <--> R33.ethg[0];


        R30.ethg[3] <--> EtherLink <--> R31.ethg[2];

        R31.ethg[3] <--> EtherLink <--> R32.ethg[2];

        R32.ethg[3] <--> EtherLink <--> R33.ethg[2];


        server.ethg++ <--> EtherLink <--> R00.ethg[0];
        R33.ethg[1] <--> EtherLink <--> client.ethg++;
}
