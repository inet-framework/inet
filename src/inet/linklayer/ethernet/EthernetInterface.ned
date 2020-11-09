//
// Copyright (C) 2004 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.linklayer.ethernet;

import inet.linklayer.contract.IEtherMac;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetEncapsulation;
import inet.networklayer.common.NetworkInterface;
import inet.queueing.contract.ITrafficConditioner;


//
// This module represents an Ethernet network interface.
//
module EthernetInterface extends NetworkInterface like IEthernetInterface
{
    parameters:
        @lifecycleSupport;
        string interfaceTableModule;
        string address = default("auto");   // MAC address as hex string (12 hex digits), or
                                            // "auto". "auto" values will be replaced by
                                            // a generated MAC address in init stage 0.
        bool duplexMode = default(true);      // selects full-duplex (true) or half-duplex (false) operation
        bool csmacdSupport = default(!duplexMode);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
        double bitrate @unit(bps) = default(0 bps);
        string fcsMode @enum("declared","computed") = default("declared");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.fcsMode = fcsMode;
        *.duplexMode = duplexMode;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input cutthroughIn @loose;
        output cutthroughOut @loose;
        inout phys @labels(EtherFrame);
    submodules:
        egressTC: <default("")> like ITrafficConditioner if typename != "" {
            parameters:
                @display("p=300,100");
        }
        ingressTC: <default("")> like ITrafficConditioner if typename != "" {
            parameters:
                @display("p=700,100");
        }
        encap: <default("EthernetEncapsulation")> like IEthernetEncapsulation {
            parameters:
                @display("p=500,200");
        }
        mac: <default(csmacdSupport ? "EthernetCsmaMac" : "EthernetMac")> like IEtherMac {
            parameters:
                queue.packetCapacity = default(10000);
                @display("p=500,300");
        }
    connections:
        mac.upperLayerOut --> encap.lowerLayerIn;
        mac.upperLayerIn <-- encap.lowerLayerOut;

        mac.phys <--> { @display("m=s"); } <--> phys;

        encap.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if !exists(ingressTC);
        encap.upperLayerOut --> ingressTC.in if exists(ingressTC);
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut if exists(ingressTC);

        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if !exists(egressTC);
        upperLayerIn --> { @display("m=n"); } --> egressTC.in if exists(egressTC);
        egressTC.out --> encap.upperLayerIn if exists(egressTC);
}

