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

//
// Dummy encapsulation/decapsulation module for ~EthernetInterface.
//
// Expected environment:
// - lowerLayerIn, lowerLayerOut gates should be connected to an ~EtherMAC
// - upperLayerIn, upperLayerOut gates should be connected to an ~IMACRelayUnit
//
// Functionality:
// - packets coming from upper layers will be sent to MAC.
// - Ethernet frames arriving from MAC will be sent up to the higher layers.
//
module EtherEncapDummy like IEtherEncap
{
    parameters:
        @display("i=block/square");
    gates:
        input upperLayerIn @labels(EtherFrame); // higher layer
        output upperLayerOut @labels(EtherFrame); // higher layer
        input lowerLayerIn @labels(EtherFrame);   // to Ethernet MAC
        output lowerLayerOut @labels(EtherFrame); // to Ethernet MAC
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

