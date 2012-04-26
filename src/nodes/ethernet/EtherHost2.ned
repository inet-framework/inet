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

package inet.nodes.ethernet;

import inet.linklayer.ethernet.EtherEncap;
import inet.applications.ethernet.EtherTrafGen;
import inet.linklayer.ethernet.EtherLLC;
import inet.linklayer.ethernet.EtherQoSQueue;
import inet.linklayer.IEtherMAC;
import inet.applications.ethernet.EtherAppSrv;
import inet.applications.ethernet.EtherAppCli;


//
// An example host with one Ethernet interface and an Ethernet traffic
// generator directly connected to it. This host model does not contain
// higher layer protocols (IP, TCP). By default is configured to use
// half-duplex MAC (CSMA/CD).
//
module EtherHost2
{
    parameters:
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string queueType = default(""); // DropTailQueue, DiffServQueue, or "" to use the internal queue
        @node();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        app: EtherTrafGen {
            @display("p=155,59");
        }
        encap: EtherEncap {
            @display("p=155,146");
        }
        queue: EtherQoSQueue if queueType != "" {
            parameters:
                dataQueueType = queueType;
                @display("p=87,207;q=l2queue");
        }
        mac: <macType> like IEtherMAC {
            queueModule = (queueType == "" ? "" : "queue");
            @display("p=155,272;q=txQueue");
        }
    connections:
        app.out --> encap.upperLayerIn;
        app.in <-- encap.upperLayerOut;

        encap.lowerLayerOut --> queue.in if queueType != "";
        queue.out --> mac.upperLayerIn if queueType != "";
        encap.lowerLayerOut --> mac.upperLayerIn if queueType == "";
        mac.upperLayerOut --> encap.lowerLayerIn;

        mac.phys <--> ethg;
}
