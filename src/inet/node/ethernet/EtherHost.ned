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

import inet.applications.ethernet.EtherAppCli;
import inet.applications.ethernet.EtherAppSrv;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IEtherMAC;
import inet.linklayer.ethernet.EtherLLC;
import inet.linklayer.ethernet.EtherQoSQueue;


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
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string queueType = default(""); // DropTailQueue, DiffServQueue, or empty to use the internal queue
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
        *.interfaceTableModule = default("");
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=50,50;is=s");
        }
        cli: EtherAppCli {
            parameters:
                registerSAP = true;
                @display("p=120,58,col");
        }
        srv: EtherAppSrv {
            parameters:
                registerSAP = true;
                @display("p=310,58,col");
        }
        llc: EtherLLC {
            parameters:
                @display("p=215,118");
            gates:
                upperLayerIn[2];
                upperLayerOut[2];
        }
        queue: EtherQoSQueue if queueType != "" {
            parameters:
                dataQueueType = queueType;
                @display("p=159,199;q=l2queue");
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = (queueType == "" ? "" : "^.queue");
                @display("p=215,270;q=txQueue");
        }
    connections:

        cli.out --> llc.upperLayerIn[0];
        cli.in <-- llc.upperLayerOut[0];
        srv.out --> llc.upperLayerIn[1];
        srv.in <-- llc.upperLayerOut[1];

        llc.lowerLayerOut --> queue.in if queueType != "";
        queue.out --> mac.upperLayerIn if queueType != "";
        llc.lowerLayerOut --> mac.upperLayerIn if queueType == "";
        mac.upperLayerOut --> llc.lowerLayerIn;

        mac.phys <--> ethg;
}
