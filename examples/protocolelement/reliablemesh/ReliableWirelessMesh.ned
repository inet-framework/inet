//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// A multi-hop reliable wireless network built ONLY from inet.protocolelement + inet.queueing
// modules -- no real protocols (no Ipv4/Udp/Ieee80211/Radio). Four @networkNode nodes in a line
// over one shared SimplifiedRadioMedium (range-limited so each node hears only its neighbours):
//
//     source(10.0.0.1) --- relay1(10.0.0.2) --- relay2(10.0.0.3) --- dest(10.0.0.4)
//
// The source app addresses a packet to the distant destination; Forwarding routes it hop-by-hop
// (static routes), and every hop is an independent reliable link: stop-and-wait acknowledgement
// (SendWithAcknowledge/ReceiveWithAcknowledge) + retransmission (Resending) + exponential backoff
// (ExponentialBackoff). The medium drops frames with a configurable probability, so hops retransmit
// (with backoff) and still deliver. See omnetpp.ini and the plan doc in plan/done/.
//

package inet.examples.protocolelement.reliablemesh;

import ned.DatarateChannel;
import inet.common.MessageDispatcher;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.common.NetworkInterface;
import inet.queueing.source.ActivePacketSource;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.PacketServer;
import inet.protocolelement.acknowledgement.Resending;
import inet.protocolelement.acknowledgement.SendWithAcknowledge;
import inet.protocolelement.acknowledgement.ReceiveWithAcknowledge;
import inet.protocolelement.backoff.ExponentialBackoff;
import inet.protocolelement.dispatching.SendWithProtocol;
import inet.protocolelement.dispatching.ReceiveWithProtocol;
import inet.protocolelement.forwarding.Forwarding;
import inet.protocolelement.forwarding.SendWithHopLimit;
import inet.protocolelement.forwarding.ReceiveWithHopLimit;
import inet.protocolelement.medium.SimplifiedRadioMedium;
import inet.protocolelement.selectivity.SendToL3Address;
import inet.protocolelement.selectivity.ReceiveAtL3Address;
import inet.protocolelement.selectivity.SendToMacAddress;
import inet.protocolelement.selectivity.ReceiveAtMacAddress;
import inet.protocolelement.selectivity.SendFromMacAddress;
import inet.protocolelement.selectivity.ReceiveFromMacAddress;
import inet.protocolelement.selectivity.NextHopMacResolver;
import inet.protocolelement.transceiver.PacketTransmitter;
import inet.protocolelement.transceiver.PacketReceiver;

moduleinterface IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
}

module SourceApp like IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
    submodules:
        source: ActivePacketSource { @display("p=100,100"); }
        sendToL3: SendToL3Address { @display("p=100,200"); }
    connections allowunconnected:
        source.out --> sendToL3.in;
        sendToL3.out --> out;
}

module SinkApp like IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
    submodules:
        receiveAtL3: ReceiveAtL3Address { @display("p=100,100"); }
        sink: PassivePacketSink { @display("p=100,200"); }
    connections allowunconnected:
        in --> receiveAtL3.in;
        receiveAtL3.out --> sink.in;
}

// A wireless network interface that makes its link reliable: stop-and-wait acknowledgement +
// retransmission + exponential backoff, with MAC filtering for the shared medium.
// Submodules are declared downstream-first so a module has cached its gates before an upstream
// module's protocol/service registration is forwarded into it.
module WirelessInterface extends NetworkInterface
{
    parameters:
        string interfaceTableModule;
        @display("i=block/ifcard");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        inout radio;
    submodules:
        transmitter: PacketTransmitter { @display("p=150,700"); }
        // a queue + server serialise data and acks onto the shared transmitter (a relay may want to
        // forward a frame and send an ack at nearly the same time; the transmitter handles one at a time)
        txServer: PacketServer { @display("p=150,625"); }
        txQueue: PacketQueue { @display("p=150,550"); }
        sendToMac: SendToMacAddress { @display("p=150,490"); }
        sendFromMac: SendFromMacAddress { @display("p=150,450"); }
        sendProtocol: SendWithProtocol { @display("p=150,410"); }
        mux: PacketMultiplexer { @display("p=150,360"); }
        sendWithAck: SendWithAcknowledge { @display("p=150,310"); }
        macResolver: NextHopMacResolver { @display("p=150,250"); }
        backoff: ExponentialBackoff { @display("p=150,175"); }
        resending: Resending { @display("p=150,100"); }
        receiver: PacketReceiver { @display("p=450,625"); }
        receiveAtMac: ReceiveAtMacAddress { @display("p=450,500"); }
        receiveFromMac: ReceiveFromMacAddress { @display("p=450,450"); }
        receiveProtocol: ReceiveWithProtocol { @display("p=450,400"); }
        d1: MessageDispatcher { @display("p=450,300"); }
        receiveWithAck: ReceiveWithAcknowledge { @display("p=450,175"); }
    connections:
        upperLayerIn --> resending.in;
        resending.out --> backoff.in;
        backoff.out --> macResolver.in;
        macResolver.out --> sendWithAck.in;
        sendWithAck.out --> mux.in++;
        receiveWithAck.ackOut --> mux.in++;
        // encode the data-vs-ack protocol, then stamp source + destination MAC (destination is
        // outermost on the wire), so the receiver can MAC-filter, learn the sender (to unicast the
        // ack back), and still recover which kind of frame this is
        mux.out --> sendProtocol.in;
        sendProtocol.out --> sendFromMac.in;
        sendFromMac.out --> sendToMac.in;
        sendToMac.out --> txQueue.in;
        txQueue.out --> txServer.in;
        txServer.out --> transmitter.in;
        transmitter.out --> radio$o;
        radio$i --> receiver.in;
        receiver.out --> receiveAtMac.in;
        receiveAtMac.out --> receiveFromMac.in;
        receiveFromMac.out --> receiveProtocol.in;
        receiveProtocol.out --> d1.in++;
        d1.out++ --> receiveWithAck.in;
        receiveWithAck.out --> upperLayerOut;
        d1.out++ --> sendWithAck.ackIn;
}

// Hop-by-hop forwarding: inputs merged by a multiplexer, the forwarding decision routed by a
// dispatcher to either local delivery (up) or the hop-limit sender (down). Avoids Network91's
// single forwarding<->dispatcher loop (which makes canPushSomePacket recurse forever).
module ForwardingLayer
{
    parameters:
        @display("i=block/routing");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        // forwarding declared last so all its neighbours have cached their gates before it
        // registers (else the registration walk hits a not-yet-initialized multiplexer/dispatcher)
        outputDispatcher: MessageDispatcher { @display("p=300,120"); }
        inputMux: PacketMultiplexer { @display("p=300,420"); }
        sendWithHopLimit: SendWithHopLimit { @display("p=150,220"); }
        receiveWithHopLimit: ReceiveWithHopLimit { @display("p=450,220"); }
        forwarding: Forwarding { interfaceTableModule = "^.^.interfaceTable"; @display("p=300,320"); }
    connections:
        upperLayerIn --> inputMux.in++;
        receiveWithHopLimit.out --> inputMux.in++;
        inputMux.out --> forwarding.in;
        forwarding.out --> outputDispatcher.in++;
        outputDispatcher.out++ --> sendWithHopLimit.in;
        outputDispatcher.out++ --> upperLayerOut;
        sendWithHopLimit.out --> lowerLayerOut;
        lowerLayerIn --> receiveWithHopLimit.in;
}

module WirelessNode
{
    parameters:
        @networkNode;
        @display("i=device/pc");
    gates:
        inout radio;
    submodules:
        interfaceTable: InterfaceTable { @display("p=100,100"); }
        app: <default("")> like IApp if typename != "" { @display("p=300,100"); }
        forwardingLayer: ForwardingLayer { @display("p=300,200"); }
        interface: WirelessInterface {
            interfaceTableModule = "^.interfaceTable";
            @display("p=300,350");
        }
    connections:
        app.out --> forwardingLayer.upperLayerIn if exists(app);
        forwardingLayer.upperLayerOut --> app.in if exists(app);
        forwardingLayer.lowerLayerOut --> interface.upperLayerIn;
        interface.upperLayerOut --> forwardingLayer.lowerLayerIn;
        radio <--> interface.radio;
}

network ReliableWirelessMesh
{
    submodules:
        radioMedium: SimplifiedRadioMedium { @display("p=250,60;i=misc/sun"); }
        source: WirelessNode { @display("p=100,200"); }
        relay1: WirelessNode { @display("p=250,200"); }
        relay2: WirelessNode { @display("p=400,200"); }
        dest: WirelessNode { @display("p=550,200"); }
    connections:
        source.radio <--> DatarateChannel { delay = 1us; } <--> radioMedium.radio++;
        relay1.radio <--> DatarateChannel { delay = 1us; } <--> radioMedium.radio++;
        relay2.radio <--> DatarateChannel { delay = 1us; } <--> radioMedium.radio++;
        dest.radio   <--> DatarateChannel { delay = 1us; } <--> radioMedium.radio++;
}

