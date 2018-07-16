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

import inet.applications.ethernet.EtherTrafGen;
import inet.common.queue.IOutputQueue;
import inet.linklayer.contract.IEtherMac;
import inet.linklayer.ethernet.IEtherEncap;

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
        string fcsMode @enum("declared", "computed") = default("declared");
        *.interfaceTableModule = default("");
        *.fcsMode = fcsMode;
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
    gates:
        inout ethg @labels(EtherFrame-conn);
    submodules:
        app: EtherTrafGen {
            @display("p=200,100");
        }
        encap: <default("EtherEncap")> like IEtherEncap {
            @display("p=200,200");
        }
        queue: <default("")> like IOutputQueue if typename != "" {
            parameters:
                @display("p=100,300;q=l2queue");
        }
        mac: <default(csmacdSupport ? "EtherMac" : "EtherMacFullDuplex")> like IEtherMac {
            queueModule = (!exists(queue) ? "" : "^.queue");
            @display("p=200,400;q=txQueue");
        }
    connections:
        app.out --> encap.upperLayerIn;
        app.in <-- encap.upperLayerOut;

        encap.lowerLayerOut --> queue.in if exists(queue);
        queue.out --> mac.upperLayerIn if exists(queue);
        encap.lowerLayerOut --> mac.upperLayerIn if !exists(queue);
        mac.upperLayerOut --> encap.lowerLayerIn;

        mac.phys <--> ethg;
}
