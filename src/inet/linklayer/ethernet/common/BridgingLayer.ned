//
// Copyright (C) 2021 OpenSim Ltd.
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

package inet.linklayer.ethernet.common;

import inet.protocolelement.contract.IProtocolLayer;

module BridgingLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        reversingLayer: <default("ReversingLayer")> like IProtocolLayer {
            @display("p=300,150");
        }
        streamPolicy: <default("OmittedProtocolLayer")> like IProtocolLayer {
            @display("p=300,300");
        }
        relayLayer: <default("RelayLayer")> like IProtocolLayer {
            @display("p=300,450");
        }
        vlanPolicy: <default("OmittedProtocolLayer")> like IProtocolLayer {
            @display("p=300,600");
        }
    connections:
        upperLayerIn --> reversingLayer.upperLayerIn;
        reversingLayer.lowerLayerOut --> streamPolicy.upperLayerIn;
        streamPolicy.lowerLayerOut --> relayLayer.upperLayerIn;
        relayLayer.lowerLayerOut --> vlanPolicy.upperLayerIn;
        vlanPolicy.lowerLayerOut --> lowerLayerOut;

        lowerLayerIn --> vlanPolicy.lowerLayerIn;
        vlanPolicy.upperLayerOut --> relayLayer.lowerLayerIn;
        relayLayer.upperLayerOut --> streamPolicy.lowerLayerIn;
        streamPolicy.upperLayerOut --> reversingLayer.lowerLayerIn;
        reversingLayer.upperLayerOut --> upperLayerOut;
}
