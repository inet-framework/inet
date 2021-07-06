//
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.linklayer.ethernet.basic;

import inet.linklayer.ethernet.contract.IEthernetEncapsulation;
import inet.linklayer.ethernet.modular.EthernetFcsChecker;
import inet.linklayer.ethernet.modular.EthernetFcsInserter;
import inet.linklayer.ethernet.modular.EthernetMacHeaderChecker;
import inet.linklayer.ethernet.modular.EthernetMacHeaderInserter;


//
// Dummy encapsulation/decapsulation module for ~EthernetInterface.
//
// Expected environment:
// - lowerLayerIn, lowerLayerOut gates should be connected to an ~EthernetCsmaMac
// - upperLayerIn, upperLayerOut gates should be connected to an ~IMacRelayUnit
//
// Functionality:
// - packets coming from upper layers will be sent to MAC.
// - Ethernet frames arriving from MAC will be sent up to the higher layers.
//
module SimpleEthernetEncapsulation like IEthernetEncapsulation
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        *.interfaceTableModule = this.interfaceTableModule;
    gates:
        input upperLayerIn @labels(EtherFrame); // higher layer
        output upperLayerOut @labels(EtherFrame); // higher layer
        input lowerLayerIn @labels(EtherFrame);   // to Ethernet MAC
        output lowerLayerOut @labels(EtherFrame); // to Ethernet MAC
    submodules:
        macHeaderInserter: EthernetMacHeaderInserter {
            @display("p=200,100");
        }
        fcsInserter: EthernetFcsInserter {
            @display("p=200,200");
        }
        fcsChecker: EthernetFcsChecker {
            @display("p=500,200");
        }
        macHeaderChecker: EthernetMacHeaderChecker {
            @display("p=500,200");
        }
    connections:
        upperLayerIn --> macHeaderInserter.in;
        macHeaderInserter.out --> fcsInserter.in;
        fcsInserter.out --> lowerLayerOut;
        lowerLayerIn --> fcsChecker.in;
        fcsChecker.out --> macHeaderChecker.in;
        macHeaderChecker.out --> upperLayerOut;
}

