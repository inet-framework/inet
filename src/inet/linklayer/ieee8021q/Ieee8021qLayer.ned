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

package inet.linklayer.ieee8021q;

import inet.queueing.contract.IPacketFlow;


module Ieee8021qLayer like IIeee8021qLayer
{
    parameters:
        string vlanTagType = default("c");
        bool hasSocketSupport = default(false);
        *.vlanTagType = default(vlanTagType);
        *.socketTableModule = default(absPath(".socketTable"));
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        socketTable: Ieee8021qSocketTable if hasSocketSupport {
            @display("p=100,100");
        }
        socketCommandProcessor: <default(hasSocketSupport ? "Ieee8021qSocketCommandProcessor" : "OmittedPacketFlow")> like IPacketFlow {
            @display("p=300,100");
        }
        qTagHeaderInserter: <default("Ieee8021qTagEpdHeaderInserter")> like IIeee8021qTagEpdHeaderInserter {
            @display("p=300,200");
        }
        socketPacketProcessor: <default(hasSocketSupport ? "Ieee8021qSocketPacketProcessor" : "OmittedPacketFlow")> like IPacketFlow {
            @display("p=600,100");
        }
        qTagHeaderChecker: <default("Ieee8021qTagEpdHeaderChecker")> like IIeee8021qTagEpdHeaderChecker {
            @display("p=600,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> socketCommandProcessor.in;
        socketCommandProcessor.out --> qTagHeaderInserter.in;
        qTagHeaderInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> qTagHeaderChecker.in;
        qTagHeaderChecker.out --> socketPacketProcessor.in;
        socketPacketProcessor.out --> { @display("m=n"); } --> upperLayerOut;
}
