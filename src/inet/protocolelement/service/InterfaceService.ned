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

package inet.protocolelement.service;

import inet.common.MessageDispatcher;
import inet.networklayer.common.NetworkInterface;
import inet.protocolelement.acknowledgement.ReceiveWithAcknowledge;
import inet.protocolelement.acknowledgement.Resending;
import inet.protocolelement.acknowledgement.SendWithAcknowledge;
import inet.protocolelement.checksum.CrcHeaderChecker;
import inet.protocolelement.checksum.CrcHeaderInserter;
import inet.protocolelement.dispatching.ReceiveWithProtocol;
import inet.protocolelement.dispatching.SendWithProtocol;
import inet.protocolelement.selectivity.ReceiveAtMacAddress;
import inet.protocolelement.selectivity.SendToMacAddress;
import inet.protocolelement.transceiver.PacketTransmitter;
import inet.queueing.common.PacketMultiplexer;

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
        inout g;
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
        crcInserter: CrcHeaderInserter {
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
        crcChecker: CrcHeaderChecker {
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
