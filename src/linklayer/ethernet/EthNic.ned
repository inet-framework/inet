//
// Copyright (C) 2013 OpenSim Ltd.
// author: Zoltan Bojthe
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

import inet.base.IHook;
import inet.linklayer.IEtherMAC;
import inet.linklayer.ITrafficConditioner;
import inet.linklayer.IWiredNic;


//
// Simple ethernet network interface. Complements ~EtherMAC and ~EtherEncapDummy
// modules.
//
module EthNic like IWiredNic
{
    parameters:
        bool csmacdSupport = default(false);  // by default CSMA/CD is turned off, so only point-to-point duplex links are supported.
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        @display("i=block/ifcard;bgl=2");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
    submodules:
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = "";
                txQueueLimit = 10000; // queue sends one packet at a time
                @display("p=182,321");
        }
        encap: EtherEncapDummy {
            parameters:
                @display("p=182,205");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn;
        encap.lowerLayerOut --> mac.upperLayerIn;
        mac.phys <--> { @display("m=s"); } <--> phys;
        mac.upperLayerOut --> encap.lowerLayerIn;
        encap.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

