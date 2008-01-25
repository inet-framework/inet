//
// Copyright (C) 2004 Andras Varga
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




package org.omnetpp.inet.examples.INET.NClients;

import ned.cBasicChannel;
import org.omnetpp.inet.Network.AutoRouting.FlatNetworkConfigurator;
import org.omnetpp.inet.Nodes.INET.Router;
import org.omnetpp.inet.Nodes.INET.StandardHost;
import org.omnetpp.inet.World.ChannelInstaller;
import org.omnetpp.inet.World.NAMTrace;
import org.omnetpp.inet.examples.Ethernet.LANs.ethernetline;


network NClients
{
    parameters:
        int n;
    types:
        channel fiberline extends cBasicChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
        channel ethernetline extends cBasicChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
        }
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=u";
                @display("p=98,50");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                moduleTypes = "Router StandardHost";
                nonIPModuleTypes = "";
                networkAddress = "145.236.0.0";
                netmask = "255.255.0.0";
                @display("p=185,50");
        }
        nam: NAMTrace {
            parameters:
                @display("p=260,50");
        }
        r1: Router {
            parameters:
                @display("p=186,250");
        }
        r2: Router {
            parameters:
                @display("p=304,220");
        }
        r3: Router {
            parameters:
                @display("p=433,250");
        }
        cli[n]: StandardHost {
            parameters:
                @display("i=device/laptop");
        }
        srv: StandardHost {
            parameters:
                @display("p=544,194;i=device/server_l");
        }
    connections allowunconnected:
        for i=0..n-1 {
            cli[i].pppg++ <--> ethernetline <--> r1.pppg++;
        }
        r1.pppg++ <--> ethernetline <--> r2.pppg++;
        r2.pppg++ <--> ethernetline <--> r3.pppg++;
        r3.pppg++ <--> ethernetline <--> srv.pppg++;
}

