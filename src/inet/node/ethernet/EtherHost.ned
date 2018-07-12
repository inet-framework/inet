//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


package inet.node.ethernet;

import inet.applications.ethernet.EtherAppClient;
import inet.applications.ethernet.EtherAppServer;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.EtherLlc;


//
// An example host with one Ethernet port and a traffic generator that
// generates request-reply traffic directly over Ethernet. This host model
// does not contain higher layer protocols (IP, TCP). By default it is
// configured to use half-duplex MAC (CSMA/CD).
//
module EtherHost
{
    parameters:
        bool hasStatus = default(false);
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string fcsMode @enum("declared", "computed") = default("declared");
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
        eth.csmacdSupport = csmacdSupport;
        *.interfaceTableModule = default("");
        *.fcsMode = fcsMode;
        eth.encap.typename = "EtherEncapDummy";
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=100,100;is=s");
        }
        cli: EtherAppClient {
            parameters:
                registerSAP = true;
                @display("p=250,100");
        }
        srv: EtherAppServer {
            parameters:
                registerSAP = true;
                @display("p=450,100");
        }
        llc: EtherLlc {
            parameters:
                @display("p=350,200");
            gates:
                upperLayerIn[2];
                upperLayerOut[2];
        }
        eth: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=350,297,row,150;q=txQueue");
        }
    connections:

        cli.out --> llc.upperLayerIn[0];
        cli.in <-- llc.upperLayerOut[0];
        srv.out --> llc.upperLayerIn[1];
        srv.in <-- llc.upperLayerOut[1];

        llc.lowerLayerOut --> eth.upperLayerIn;
        eth.upperLayerOut --> llc.lowerLayerIn;

        eth.phys <--> { @display("m=s"); } <--> ethg;
}

