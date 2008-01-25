//
// Copyright (C) 2005 Vojtech Janota
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




package org.omnetpp.inet.examples.Quagga.FourRouters;

import ned.cBasicChannel;
import org.omnetpp.inet.Network.Extras.FailureManager;
import org.omnetpp.inet.Nodes.INET.QuaggaRouter;
import org.omnetpp.inet.Nodes.INET.StandardHost;
import org.omnetpp.inet.World.ChannelInstaller;
import org.omnetpp.inet.World.NAMTrace;
import org.omnetpp.inet.World.ScenarioManager;
import org.omnetpp.inet.examples.Ethernet.LANs.ethernetline;


network ZebraTest
{
    parameters:
        int n;
    types:
        channel fiberline extends cBasicChannel
        {
            delay = 1ms;
            datarate = 1Mbps;
        }
        channel ethernetline extends cBasicChannel
        {
            delay = 10ms;
            datarate = 100Kbps;
        }
    submodules:
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=68,50");
        }
        failureManager: FailureManager {
            parameters:
                @display("p=152,50");
        }
        nam: NAMTrace {
            parameters:
                @display("p=219,50");
        }
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=u #N";
                @display("p=284,50");
        }
        cli[n]: StandardHost {
            parameters:
                namid = index;
                @display("p=62,146,c;i=device/laptop");
        }
        srv: StandardHost {
            parameters:
                namid = n;
                @display("p=512,242;i=device/server_l");
        }
        r1: QuaggaRouter {
            parameters:
                namid = n + 1;
                @display("p=173,250");
        }
        r2: QuaggaRouter {
            parameters:
                namid = n + 2;
                @display("p=284,130");
        }
        r3: QuaggaRouter {
            parameters:
                namid = n + 3;
                @display("p=405,242");
        }
        r4: QuaggaRouter {
            parameters:
                namid = n + 4;
                @display("p=294,370");
        }
    connections allowunconnected:
        for i=0..n-1 {
            cli[i].pppg++ <--> ethernetline <--> r1.pppg++;
        }
        r1.pppg++ <--> ethernetline <--> r2.pppg++;
        r2.pppg++ <--> ethernetline <--> r3.pppg++;
        r3.pppg++ <--> ethernetline <--> srv.pppg++;
        r1.pppg++ <--> ethernetline <--> r4.pppg++;
        r3.pppg++ <--> ethernetline <--> r4.pppg++;
}

