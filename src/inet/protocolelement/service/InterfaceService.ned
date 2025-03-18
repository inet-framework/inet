//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.service;

import inet.common.MessageDispatcher;
import inet.networklayer.common.NetworkInterface;
import inet.protocolelement.acknowledgement.ReceiveWithAcknowledge;
import inet.protocolelement.acknowledgement.Resending;
import inet.protocolelement.acknowledgement.SendWithAcknowledge;
import inet.protocolelement.checksum.EthernetFcsHeaderChecker;
import inet.protocolelement.checksum.EthernetFcsHeaderInserter;
import inet.protocolelement.dispatching.ReceiveWithProtocol;
import inet.protocolelement.dispatching.SendWithProtocol;
import inet.protocolelement.selectivity.ReceiveAtMacAddress;
import inet.protocolelement.selectivity.SendToMacAddress;
import inet.protocolelement.transceiver.PacketTransmitter;
import inet.queueing.common.PacketMultiplexer;

//
// Implements a network interface with comprehensive protocol processing
// capabilities. Provides MAC address filtering, packet acknowledgment and
// resending, protocol-based dispatching, and CRC verification. Extends the
// NetworkInterface module with protocol elements for reliable communication.
//
module InterfaceService extends NetworkInterface
{
    parameters:
        string interfaceTableModule;
        string protocol;
        @class(inet::NetworkInterface);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        inout phys;
    submodules:
        sendToMacAddress: SendToMacAddress {
            @display("p=300,100");
        }
        resending: Resending {
            @display("p=300,200");
        }
        sendWithAcknowledge: SendWithAcknowledge {
            @display("p=300,300");
        }
        m1: PacketMultiplexer {
            @display("p=300,400");
        }
        sendWithProtocol: SendWithProtocol {
            @display("p=300,500");
        }
        crcInserter: EthernetFcsHeaderInserter {
            @display("p=300,600");
        }
        transmitter: PacketTransmitter {
            @display("p=300,700");
        }
        receiveAtMacAddress: ReceiveAtMacAddress {
            @display("p=600,100");
        }
        receiveWithAcknowledge: ReceiveWithAcknowledge {
            @display("p=600,200");
        }
        d1: MessageDispatcher {
            @display("p=600,300");
        }
        receiveWithProtocol: ReceiveWithProtocol {
            @display("p=600,400");
        }
        crcChecker: EthernetFcsHeaderChecker {
            @display("p=600,500");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> sendToMacAddress.in;
        sendToMacAddress.out --> resending.in;
        resending.out --> sendWithAcknowledge.in;
        sendWithAcknowledge.out --> m1.in++;
        m1.out --> sendWithProtocol.in;
        sendWithProtocol.out --> crcInserter.in;
        crcInserter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> phys$o;

        phys$i --> { @display("m=s"); } --> crcChecker.in;
        crcChecker.out --> receiveWithProtocol.in;
        receiveWithProtocol.out --> d1.in++;
        d1.out++ --> receiveWithAcknowledge.in;
        receiveWithAcknowledge.out --> receiveAtMacAddress.in;
        receiveWithAcknowledge.ackOut --> m1.in++;
        d1.out++ --> sendWithAcknowledge.ackIn;
        receiveAtMacAddress.out --> { @display("m=n"); } --> upperLayerOut;
}
