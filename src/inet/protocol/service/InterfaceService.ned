//
// Copyright (C) OpenSim Ltd.
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

package inet.protocol.service;

import inet.common.MessageDispatcher;
import inet.networklayer.common.InterfaceEntry;
import inet.protocol.acknowledgement.ReceiveWithAcknowledge;
import inet.protocol.acknowledgement.Resending;
import inet.protocol.acknowledgement.SendWithAcknowledge;
import inet.protocol.checksum.CrcHeaderChecker;
import inet.protocol.checksum.CrcHeaderInserter;
import inet.protocol.dispatching.ReceiveWithProtocol;
import inet.protocol.dispatching.SendWithProtocol;
import inet.protocol.selectivity.ReceiveAtMacAddress;
import inet.protocol.selectivity.SendToMacAddress;
import inet.protocol.transceiver.PacketTransmitter;
import inet.queueing.common.PacketMultiplexer;


module InterfaceService extends InterfaceEntry
{
    parameters:
        string interfaceTableModule;
        @class(inet::InterfaceEntry);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        inout g;
    submodules:
        sendToMacAddress: SendToMacAddress {
            @display("p=150,100");
        }
        resending: Resending {
            @display("p=150,200");
        }
        sendWithAcknowledge: SendWithAcknowledge {
            @display("p=150,300");
        }
        m1: PacketMultiplexer {
            @display("p=150,400");
        }
        sendWithProtocol: SendWithProtocol {
            @display("p=150,500");
        }
        crcInserter: CrcHeaderInserter {
            @display("p=150,600");
        }
        transmitter: PacketTransmitter {
            @display("p=150,700");
        }
        receiveAtMacAddress: ReceiveAtMacAddress {
            @display("p=450,100");
        }
        receiveWithAcknowledge: ReceiveWithAcknowledge {
            @display("p=450,200");
        }
        d1: MessageDispatcher {
            @display("p=450,300");
        }
        receiveWithProtocol: ReceiveWithProtocol {
            @display("p=450,400");
        }
        crcChecker: CrcHeaderChecker {
            @display("p=450,500");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> sendToMacAddress.in;
        sendToMacAddress.out --> resending.in;
        resending.out --> sendWithAcknowledge.in;
        sendWithAcknowledge.out --> m1.in++;
        m1.out --> sendWithProtocol.in;
        sendWithProtocol.out --> crcInserter.in;
        crcInserter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> g$o;
        g$i --> { @display("m=s"); } --> crcChecker.in;
        crcChecker.out --> receiveWithProtocol.in;
        receiveWithProtocol.out --> d1.in++;
        d1.out++ --> receiveWithAcknowledge.in;
        receiveWithAcknowledge.out --> receiveAtMacAddress.in;
        receiveWithAcknowledge.ackOut --> m1.in++;
        d1.out++ --> sendWithAcknowledge.ackIn;
        receiveAtMacAddress.out --> { @display("m=n"); } --> upperLayerOut;
}
