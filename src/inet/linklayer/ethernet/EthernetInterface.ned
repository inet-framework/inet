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

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.contract.IEtherMac;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.ITrafficConditioner;
import inet.linklayer.ieee8021q.IIeee8021qEncap;
import inet.networklayer.common.InterfaceEntry;


//
// Ethernet network interface. Complements ~EtherMac and ~EtherEncap
// with an output queue for QoS and RED support.
//
module EthernetInterface extends InterfaceEntry like IEthernetInterface
{
    parameters:
        @class(InterfaceEntry);
        @lifecycleSupport;
        string interfaceTableModule;
        string address @mutable = default("auto");   // MAC address as hex string (12 hex digits), or
                                                     // "auto". "auto" values will be replaced by
                                                     // a generated MAC address in init stage 0.
        bool csmacdSupport = default(false);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
        string fcsMode @enum("declared","computed") = default("declared");
        @display("i=block/ifcard;bgl=2");
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.fcsMode = this.fcsMode;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
    submodules:
        ingressTC: <default("")> like ITrafficConditioner if typename != "" {
            @display("p=400,100");
        }
        egressTC: <default("")> like ITrafficConditioner if typename != "" {
            @display("p=100,100");
        }
        mac: <default(csmacdSupport ? "EtherMac" : "EtherMacFullDuplex")> like IEtherMac {
            parameters:
                queue.packetCapacity = default(10000);
                @display("p=250,400");
        }
        encap: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=250,200");
        }
        qEncap: <default("")> like IIeee8021qEncap if typename != "" {
            parameters:
                @display("p=150,300");
        }
    connections:
        mac.upperLayerOut --> qEncap.lowerLayerIn if exists(qEncap);
        mac.upperLayerOut --> encap.lowerLayerIn if !exists(qEncap);

        mac.upperLayerIn <-- qEncap.lowerLayerOut if exists(qEncap);
        mac.upperLayerIn <-- encap.lowerLayerOut if !exists(qEncap);

        encap.lowerLayerOut --> qEncap.upperLayerIn if exists(qEncap);
        encap.lowerLayerIn <-- qEncap.upperLayerOut if exists(qEncap);

        mac.phys <--> { @display("m=s"); } <--> phys;

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

