//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
import inet.protocolelement.processing.IProcessingDelayLayer;

//
// This module provides various layer 2 services such as packet forwarding,
// interface selection, virtual LAN handling, stream handling.
//
module BridgingLayer like IProtocolLayer
{
    parameters:
        string macTableModule = default("");
        string interfaceTableModule; // relative module path of the interface table
        *.macTableModule = default(absPath(this.macTableModule));
        *.interfaceTableModule = default(this.interfaceTableModule);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        processingDelayLayer: <default("")> like IProcessingDelayLayer {
            @display("p=300,150");
        }
        directionReverser: <default("DirectionReverserLayer")> like IProtocolLayer {
            @display("p=300,300");
        }
        streamIdentifier: <default("")> like IProtocolLayer {
            @display("p=300,450");
        }
        streamRelay: <default("")> like IProtocolLayer {
            @display("p=300,600");
        }
        streamFilter: <default("")> like IProtocolLayer {
            @display("p=300,750");
        }
        streamCoder: <default("")> like IProtocolLayer {
            @display("p=300,900");
        }
        interfaceRelay: <default("InterfaceRelayLayer")> like IProtocolLayer {
            @display("p=300,1050");
        }
        vlanPolicy: <default("")> like IProtocolLayer {
            @display("p=300,1200");
        }
    connections:
        upperLayerIn --> processingDelayLayer.upperLayerIn;
        processingDelayLayer.lowerLayerOut --> directionReverser.upperLayerIn;
        directionReverser.lowerLayerOut --> streamIdentifier.upperLayerIn;
        streamIdentifier.lowerLayerOut --> streamRelay.upperLayerIn;
        streamRelay.lowerLayerOut --> streamFilter.upperLayerIn;
        streamFilter.lowerLayerOut --> streamCoder.upperLayerIn;
        streamCoder.lowerLayerOut --> interfaceRelay.upperLayerIn;
        interfaceRelay.lowerLayerOut --> vlanPolicy.upperLayerIn;
        vlanPolicy.lowerLayerOut --> lowerLayerOut;

        lowerLayerIn --> vlanPolicy.lowerLayerIn;
        vlanPolicy.upperLayerOut --> interfaceRelay.lowerLayerIn;
        interfaceRelay.upperLayerOut --> streamCoder.lowerLayerIn;
        streamCoder.upperLayerOut --> streamFilter.lowerLayerIn;
        streamFilter.upperLayerOut --> streamRelay.lowerLayerIn;
        streamRelay.upperLayerOut --> streamIdentifier.lowerLayerIn;
        streamIdentifier.upperLayerOut --> directionReverser.lowerLayerIn;
        directionReverser.upperLayerOut --> processingDelayLayer.lowerLayerIn;
        processingDelayLayer.upperLayerOut --> upperLayerOut;
}
