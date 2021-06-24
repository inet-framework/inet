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

import inet.linklayer.ieee8021r.IIeee8021rLayer;
import inet.protocolelement.contract.IProtocolLayer;

module StreamPolicyLayer like IIeee8021rLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(this.interfaceTableModule);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        streamIdentifier: <default("StreamIdentifierLayer")> like IProtocolLayer {
            @display("p=300,150");
        }
        streamRelay: <default("StreamRelayLayer")> like IProtocolLayer {
            @display("p=300,300");
        }
        streamCoder: <default("StreamCoderLayer")> like IProtocolLayer {
            @display("p=300,450");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> streamIdentifier.upperLayerIn;
        streamIdentifier.lowerLayerOut --> streamRelay.upperLayerIn;
        streamRelay.lowerLayerOut --> streamCoder.upperLayerIn;
        streamCoder.lowerLayerOut --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> streamCoder.lowerLayerIn;
        streamCoder.upperLayerOut --> streamRelay.lowerLayerIn;
        streamRelay.upperLayerOut --> streamIdentifier.lowerLayerIn;
        streamIdentifier.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}
