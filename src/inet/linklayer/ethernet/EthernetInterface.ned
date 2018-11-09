//
// Copyright (C) 2004 Andras Varga
//               2010 Zoltan Bojthe
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

package inet.linklayer.ethernet;

import inet.common.queue.IOutputQueue;
import inet.linklayer.contract.IEtherMac;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.ITrafficConditioner;

//
// Ethernet network interface. Complements ~EtherMac and ~EtherEncap
// with an output queue for QoS and RED support.
//
module EthernetInterface like IEthernetInterface
{
    parameters:
        @class(InterfaceEntry);
        string interfaceTableModule;
        bool csmacdSupport = default(false);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
        string fcsMode @enum("declared", "computed") = default("declared");
        @display("i=block/ifcard;bgl=2");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.fcsMode = fcsMode;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
    submodules:
        ingressTC: <default("")> like ITrafficConditioner if typename != "" {
            @display("p=400,200");
        }
        egressTC: <default("")> like ITrafficConditioner if typename != "" {
            @display("p=100,200");
        }
        queue: <default("")> like IOutputQueue if typename != "" {
            parameters:
                @display("p=100,400;q=l2queue");
        }
        mac: <default(csmacdSupport ? "EtherMac" : "EtherMacFullDuplex")> like IEtherMac {
            parameters:
                queueModule = (exists(queue) ? "^.queue" : "");
                txQueueLimit = (exists(queue) ? 1 : 10000); // queue sends one packet at a time
                @display("p=200,500");
        }
        encap: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=250,300");
        }
    connections:
        mac.upperLayerOut --> encap.lowerLayerIn;
        mac.phys <--> { @display("m=s"); } <--> phys;

        if exists(queue) {
            encap.lowerLayerOut --> queue.in;
            queue.out --> mac.upperLayerIn;
        }

        encap.lowerLayerOut --> mac.upperLayerIn if !exists(queue);

        // no ingressTC
        encap.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if !exists(ingressTC);

        // there is ingressTC
        if exists(ingressTC) {
            encap.upperLayerOut --> ingressTC.in;
            ingressTC.out --> { @display("m=n"); } --> upperLayerOut;
        }

        // no egressTC
        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if !exists(egressTC);

        // there is egressTC
        if exists(egressTC) {
            upperLayerIn --> { @display("m=n"); } --> egressTC.in;
            egressTC.out --> encap.upperLayerIn;
        }
}

