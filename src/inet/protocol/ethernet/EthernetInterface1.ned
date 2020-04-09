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

package inet.protocol.ethernet;

import inet.linklayer.contract.IEthernetInterface;
import inet.networklayer.common.InterfaceEntry;
import inet.protocol.common.InterPacketGap;
import inet.protocol.common.PacketSerializer;
import inet.protocol.common.PaddingInserter;
import inet.protocol.transceiver.StreamingReceiver;
import inet.protocol.transceiver.StreamingTransmitter;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.InstantServer;


module EthernetInterface1 extends InterfaceEntry like IEthernetInterface
{
    parameters:
        string interfaceTableModule;
        string address = default("auto");
        bool broadcast = default(true);
        bool multicast = default(true);
        bool promiscuous = default(false);  // if true, all packets are received, otherwise only the
                                            // ones with matching destination MAC address
        double bitrate @unit(bps) = default(nan bps);
        int mtu @unit(B) = default(1500B);
        string fcsMode @enum("declared","computed") = default("declared");
        @class(InterfaceEntry);
        @display("i=block/ifcard");
        paddingInserter.minLength = 60B;
        ifg.duration = 96s/dropUnit(bitrate);
        transmitter.datarate = bitrate;
        receiver.datarate = bitrate;
        addressChecker.promiscuous = promiscuous;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys;
    submodules:
        queue: PacketQueue {
            parameters:
                @display("p=150,100");
        }
        server: InstantServer {
            parameters:
                @display("p=150,200");
        }
        typeOrLengthInserter: EthernetTypeOrLengthInserter {
            parameters:
                @display("p=150,300");
        }
        addressInserter: EthernetAddressInserter {
            parameters:
                @display("p=150,400");
        }
        paddingInserter: PaddingInserter {
            parameters:
                @display("p=150,500");
        }
        fcsInserter: EthernetFcsInserter {
            parameters:
                @display("p=150,600");
        }
        preambleInserter: EthernetPreambleInserter {
            parameters:
                @display("p=150,700");
        }
        serializer: PacketSerializer {
            parameters:
                @display("p=150,800");
        }
        ifg: InterPacketGap {
            parameters:
                @display("p=150,900");
        }
        transmitter: StreamingTransmitter {
            parameters:
                @display("p=150,1000");
        }
        typeOrLengthChecker: EthernetTypeOrLengthChecker {
            parameters:
                @display("p=450,100");
        }
        addressChecker: EthernetAddressChecker {
            parameters:
                @display("p=450,200");
        }
        fcsChecker: EthernetFcsChecker {
            parameters:
                @display("p=450,600");
        }
        preambleChecker: EthernetPreambleChecker {
            parameters:
                @display("p=450,800");
        }
        receiver: StreamingReceiver {
            parameters:
                @display("p=450,1000");
        }
        channelDatarateReader: ChannelDatarateReader {
            @display("p=301,1000");
        }
    connections:
        upperLayerIn --> queue.in;
        queue.out --> server.in;
        server.out --> typeOrLengthInserter.in;
        typeOrLengthInserter.out --> addressInserter.in;
        addressInserter.out --> paddingInserter.in;
        paddingInserter.out --> fcsInserter.in;
        fcsInserter.out --> preambleInserter.in;
        preambleInserter.out --> serializer.in;
        serializer.out --> ifg.in;
        ifg.out --> transmitter.in;
        transmitter.out --> phys$o;
        phys$i --> receiver.in;
        receiver.out --> preambleChecker.in;
        preambleChecker.out --> fcsChecker.in;
        fcsChecker.out --> addressChecker.in;
        addressChecker.out --> typeOrLengthChecker.in;
        typeOrLengthChecker.out --> upperLayerOut;
}
