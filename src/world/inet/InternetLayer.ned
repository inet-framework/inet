//
// Copyright (C) 2010 Philipp Berndt
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

// Based on ../../nodes/inet/NetworkLayer.ned


package inet.world.inet;

import inet.networklayer.arp.ARP;
import inet.networklayer.ipv4.ErrorHandling;
import inet.networklayer.ipv4.ICMP;
import inet.networklayer.ipv4.IP;


//
// Network layer of the \InternetCloud
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module InternetLayer
{
    parameters:
        bool proxyARP = default(true);
        bool usePingErRttData    = default(false);
        bool useAnalyticalRtt    = default(false);
        bool usePingErJitter     = default(true);
        bool usePingErPacketLoss = default(false);

        @display("i=block/fork");
    gates:
        input ifIn[] @labels(IPDatagram);
        output ifOut[];

    submodules:
        ip: IP {
            parameters:
                timeToLive = 32;
                multicastTimeToLive = 32;
                fragmentTimeout = 60s;
                protocolMapping = "1:0";
                @display("p=85,95;q=queue");
            gates:
                transportIn[1];
                transportOut[1];
                queueIn[sizeof(ifIn)];
        }

        delayGenerator: InternetPropagationDelayGate {
            parameters:
                usePingErRttData    = usePingErRttData;
                useAnalyticalRtt    = useAnalyticalRtt;
                usePingErJitter     = usePingErJitter;
                usePingErPacketLoss = usePingErPacketLoss;
                useAccessLatency    = false;
                @display("p=163,95");
        }
        arp: ARP {
            parameters:
                proxyARP = proxyARP;
                @display("p=163,206;q=pendingQueue");
            gates:
                nicOut[sizeof(ifOut)];
        }
        icmp: ICMP {
            parameters:
                @display("p=160,33");
        }
        errorHandling: ErrorHandling {
            parameters:
                @display("p=239,33");
        }
    connections allowunconnected:
        // transport Layer

        ip.transportOut[0] --> icmp.localIn;
        ip.transportIn[0] <-- icmp.sendOut;

        icmp.errorOut --> errorHandling.in;

        ip.queueOut --> delayGenerator.ipIn;
        delayGenerator.ipOut --> arp.ipIn;

        // L2 interfaces to IP and from ARP
        for i=0..sizeof(ifOut)-1 {
            ifIn[i] --> { @display("m=s"); } --> ip.queueIn[i];
            ifOut[i] <-- { @display("m=s"); } <-- arp.nicOut[i];
        }
}

