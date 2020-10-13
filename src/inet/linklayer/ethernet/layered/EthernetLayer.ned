//
// Copyright (C) 2020 OpenSim Ltd.
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

package inet.linklayer.ethernet.layered;

import inet.protocol.contract.IProtocolHeaderChecker;
import inet.protocol.contract.IProtocolHeaderInserter;
import inet.queueing.contract.IPacketFlow;

module EthernetLayer like IEthernetLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(interfaceTableModule);
        paddingInserter.minLength = default(60B);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        macHeaderInserter: <default("EthernetMacHeaderInserter")> like IProtocolHeaderInserter {
            @display("p=200,100");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,200");
        }
        macHeaderChecker: <default("EthernetMacHeaderChecker")> like IProtocolHeaderChecker {
            @display("p=500,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> macHeaderInserter.in;
        macHeaderInserter.out --> paddingInserter.in;
        paddingInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> macHeaderChecker.in;
        macHeaderChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
