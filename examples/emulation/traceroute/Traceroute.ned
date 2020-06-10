//
// Copyright (C) 2008 Irene Ruengeler
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

package inet.examples.emulation.traceroute;

import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;
import ned.TransmissionChannel;

network Traceroute
{
    parameters:
        *.eth[*].bitrate = 1 Gbps;
    types:
        channel pppline extends DatarateChannel {
            delay = 100ms;
            datarate = 1 Gbps;
        }

        channel ethline extends TransmissionChannel {
            delay = 100ms;
        }
    submodules:
        cli1: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "cli1.mrt";
                @display("p=37,62;i=old/laptop3");
            gates:
                ethg[1];
        }
        cli2: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "cli2.mrt";
                @display("p=35,131;i=old/laptop3");
            gates:
                ethg[1];
        }
        cli3: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "cli3.mrt";
                @display("p=40,223;i=old/laptop3");
            gates:
                ethg[1];
        }
        cli4: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "cli4.mrt";
                @display("p=40,295;i=old/laptop3");
            gates:
                ethg[1];
        }
        cli5: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "cli5.mrt";
                @display("p=40,383;i=old/laptop3");
            gates:
                ethg[1];
        }
        cli6: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "cli6.mrt";
                @display("p=40,439;i=old/laptop3");
            gates:
                ethg[1];
        }
        cli7: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "cli7.mrt";
                @display("p=40,511;i=old/laptop3");
            gates:
                ethg[1];
        }
        cli8: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "cli8.mrt";
                @display("p=40,575;i=old/laptop3");
            gates:
                ethg[1];
        }
        switch1: EtherSwitch {
            parameters:
                @display("p=127,102;i=device/switch");
            gates:
                ethg[3];
        }
        switch2: EtherSwitch {
            parameters:
                @display("p=129,254;i=device/switch");
            gates:
                ethg[3];
        }
        switch3: EtherSwitch {
            parameters:
                @display("p=137,398;i=device/switch");
            gates:
                ethg[3];
        }
        switch4: EtherSwitch {
            parameters:
                @display("p=138,534;i=device/switch");
            gates:
                ethg[3];
        }
        router1: Router {
            parameters:
                ipv4.routingTable.routingFile = "router1.mrt";
                @display("p=220,101;i=device/router_s");
            gates:
                pppg[1];
                ethg[1];
        }
        router2: Router {
            parameters:
                ipv4.routingTable.routingFile = "router2.mrt";
                @display("p=236,253;i=device/router_s");
            gates:
                pppg[1];
                ethg[1];
        }
        router3: Router {
            parameters:
                ipv4.routingTable.routingFile = "router3.mrt";
                @display("p=252,397;i=device/router_s");
            gates:
                pppg[1];
                ethg[1];
        }
        router4: Router {
            parameters:
                ipv4.routingTable.routingFile = "router4.mrt";
                @display("p=268,533;i=device/router_s");
            gates:
                pppg[1];
                ethg[1];
        }
        Router5: Router {
            parameters:
                ipv4.routingTable.routingFile = "Router5.mrt";
                @display("p=372,165;i=device/router");
            gates:
                pppg[3];
        }
        Router6: Router {
            parameters:
                ipv4.routingTable.routingFile = "Router6.mrt";
                @display("p=396,445;i=device/router");
            gates:
                pppg[3];
        }
        extRouter: Router {
            parameters:
                ipv4.routingTable.routingFile = "extRouter.mrt";
                @display("p=500,293;i=device/router_l");
            gates:
                pppg[2];
        }
    connections allowunconnected:
        cli1.ethg[0] <--> ethline <--> switch1.ethg[0];
        cli2.ethg[0] <--> ethline <--> switch1.ethg[1];
        switch1.ethg[2] <--> ethline <--> router1.ethg[0];
        router1.pppg[0] <--> pppline <--> Router5.pppg[0];
        Router5.pppg[2] <--> pppline <--> extRouter.pppg[0];
        cli3.ethg[0] <--> ethline <--> switch2.ethg[0];
        cli4.ethg[0] <--> ethline <--> switch2.ethg[1];
        switch2.ethg[2] <--> ethline <--> router2.ethg[0];
        router2.pppg[0] <--> pppline <--> Router5.pppg[1];
        cli5.ethg[0] <--> ethline <--> switch3.ethg[0];
        cli6.ethg[0] <--> ethline <--> switch3.ethg[1];
        switch3.ethg[2] <--> ethline <--> router3.ethg[0];
        router3.pppg[0] <--> pppline <--> Router6.pppg[0];
        Router6.pppg[2] <--> pppline <--> extRouter.pppg[1];
        cli7.ethg[0] <--> ethline <--> switch4.ethg[0];
        cli8.ethg[0] <--> ethline <--> switch4.ethg[1];
        switch4.ethg[2] <--> ethline <--> router4.ethg[0];
        router4.pppg[0] <--> pppline <--> Router6.pppg[1];
}

