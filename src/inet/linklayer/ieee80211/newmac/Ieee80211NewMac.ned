//
// Copyright (C) 2006 Andras Varga and Levente M�sz�ros
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

package inet.linklayer.ieee80211.newmac;

import inet.linklayer.ieee80211.IIeee80211Mac;


// near copy-paste of inet.linklayer.base.MACProtocolBase to work around "compound-module cannot extend a simple-module" error in OMNeT++ 4.x
module LayeredProtocolBase
{
    parameters:
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetFromUpperDropped](type=cPacket);
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetFromLowerDropped](type=cPacket);
}

// near copy-paste of inet.linklayer.base.MACProtocolBase to work around "compound-module cannot extend a simple-module" error in OMNeT++ 4.x
module MACProtocolBase extends LayeredProtocolBase
{
    parameters:
        string interfaceTableModule;
        int mtu @unit("B") = default(2304B);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(INetworkDatagram/down);
        output upperLayerOut @labels(INetworkDatagram/up);
        input lowerLayerIn @labels(ILinkLayerFrame/up);
        output lowerLayerOut @labels(ILinkLayerFrame/down);
}

//
// Implementation of the 802.11b MAC protocol. This module is intended
// to be used in combination with the ~Ieee80211Radio module as the physical
// layer. (The ~SnrEval80211 and ~Decider80211 modules should also work if
// per-packet bitrate setting gets implemented.)
//
// Encapsulation/decapsulation must be done in the upper layers. (It is
// typically in the 802.11 management module, see in ~Ieee80211Nic).
// The base class for 802.11 frame messages is ~Ieee80211Frame, but this
// module expects ~Ieee80211DataOrMgmtFrame (a subclass) from upper layers
// (the management module). This module will assign the transmitter address
// (address 2) and the frame sequence number/fragment number fields in the
// frames; all other fields must already be filled when this module gets
// the frame for transmission.
//
// The module has an internal queue, but usually it is to be used with an
// external passive queue module (its name should be given in the queueModule
// parameter; specifying "" causes the internal queue to be used). The passive
// queue module is a simple module whose C++ class implements the IPassiveQueue
// interface.
//
// <b>Limitations</b>
//
// The following features not supported: 1) fragmentation, 2) power management,
// 3) polling (PCF). Physical layer algorithms such as frequency hopping and
// direct sequence spread spectrum are not modelled directly.
//
// Fields related to the above unsupported features are omitted from
// management frame formats as well (for example, FH/DS/CF parameter sets,
// beacon/probe timestamp which is related to physical layer synchronization,
// listen interval which is related to power management, capability information
// which is related to PCF and other non-modelled features).
//
module Ieee80211NewMac extends MACProtocolBase like IIeee80211Mac
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        string queueModule = default("");    // name of optional external queue module
        int maxQueueSize = default(14); // max queue length in frames; only used if queueModule==""

        string upperMacModule = ".upperMac";
        string rxModule = ".rx";
        string immediateTxModule = ".iTx";
        string firstContentionTxModule = ".cTx[0]";
        int numContentionTx = default(4);
        string collisionControllerModule = ".ctrl";
        *.macModule = "^";
        *.upperMacModule = "^" + upperMacModule;
        *.rxModule = "^" + rxModule;
        *.immediateTxModule = "^" + immediateTxModule;
        *.firstContentionTxModule = "^" + firstContentionTxModule;
        *.collisionControllerModule = "^" + collisionControllerModule;

        @display("i=block/layer");
        @class(Ieee80211NewMac);

    submodules:
        upperMac: <default("BasicUpperMac")> like IUpperMac {
            @display("p=208,66");
        }
        iTx: <default("BasicImmediateTx")> like IImmediateTx {
            @display("p=70,200");
        }
        cTx[numContentionTx]: <default("BasicContentionTx")> like IContentionTx {
            @display("p=140,200,r,50");
        }
        rx: <default("BasicRx")> like IRx {
            @display("p=380,200");
        }
        ctrl: <default("BasicCollisionController")> like ICollisionController {
            @display("p=208,300");
        }
}

// Delegate some upperMac parameters to Ieee80211NewMac, to be more compatible with the old MAC  
module Ieee80211CompatibleNewMac extends Ieee80211NewMac
{
    parameters:
        string opMode @enum("a","b","g","n","p") = default("g");
        double bitrate @unit("bps");
        int rtsThresholdBytes @unit("B") = default(2346B);
        int retryLimit = default(-1);

        upperMac.opMode = opMode;
        upperMac.bitrate = bitrate;
        upperMac.rtsThresholdBytes = rtsThresholdBytes;
        upperMac.retryLimit = retryLimit;
}
