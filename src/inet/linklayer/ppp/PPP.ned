//
// Copyright (C) 2004 Andras Varga
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



package inet.linklayer.ppp;

//
// PPP implementation.
//
// Packets are encapsulated in ~PPPFrame.
//
// PPP is a complex protocol with strong support for link configuration
// and maintenance. This model ignores those details, and only performs
// simple encapsulation/decapsulation and queuing.
//
// In routers, PPP relies on an external queue module (see ~IOutputQueue)
// to model finite buffer, implement QoS and/or RED, and requests packets
// from this external queue one-by-one.
//
// In hosts, no such queue is used, so PPP contains an internal
// queue named txQueue to queue up packets waiting for transmission.
// Conceptually, txQueue is of infinite size, but for better diagnostics
// one can specify a hard limit in the txQueueLimit parameter -- if this is
// exceeded, the simulation stops with an error.
//
// There is no buffering done on received packets -- they are just decapsulated
// and sent up immediately.
//
// @see ~PPPInterface, ~IOutputQueue, ~PPPFrame
//
simple PPP
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        int txQueueLimit = default(1000);  // only used if queueModule==""; zero means infinite
        string queueModule = default("");  // path of external (QoS,RED,etc) queue module
        int mtu @unit("B") = default(4470B);
        @display("i=block/rxtx");

        @signal[txState](type=long);    // 1:transmit, 0:idle
        @signal[rxPkOk](type=PPPFrame);
        @signal[dropPkIfaceDown](type=cPacket);  //TODO currently both upper and lower; todo separate them
        @signal[dropPkBitError](type=cPacket);
        @signal[packetSentToLower](type=PPPFrame);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[NF_PP_TX_BEGIN](type=TxNotifDetails);
        @signal[NF_PP_TX_END](type=TxNotifDetails);
        @signal[NF_PP_RX_END](type=TxNotifDetails);
        @statistic[txState](title="tx state"; record=timeavg,vector; interpolationmode=sample-hold);
        @statistic[txPk](title="packets transmitted"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rxPkOk](title="packets received OK"; source=rxPkOk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkBitError](title="packets dropped/bit error"; source=dropPkBitError; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkIfaceDown](title="packets dropped/interface down"; source=dropPkIfaceDown; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkFromHL](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(PPPFrame);
}

