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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.linklayer.ieee80211;

import inet.linklayer.common.IIeee8021dQosClassifier;
import inet.linklayer.contract.IWirelessInterface;
import inet.linklayer.ieee80211.llc.IIeee80211Llc;
import inet.linklayer.ieee80211.mgmt.IIeee80211Agent;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mib.Ieee80211Mib;
import inet.linklayer.ieee8021q.Ieee8021qTimeAwareShaper;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;
import inet.protocolelement.common.PacketEmitter;
import inet.protocolelement.redundancy.StreamCoderLayer;
import inet.protocolelement.redundancy.StreamIdentifier;
import inet.protocolelement.redundancy.StreamIdentifierLayer;
import inet.queueing.common.PacketDelayer;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.PacketServer;
import ned.IdealChannel;


module tsnLayer
{
    @display("bgb=437,530");
    gates:
        input upperLayerIn;
        output upperLayerOut @loose;
        output lowerLayerOut;
        input lowerLayerIn @loose;
    submodules:
        packetDelayer: PacketDelayer {
            @display("p=87,75");
        }
        streamIdentifierLayer: StreamIdentifierLayer {
            @display("p=293,75");
        }
        streamCoderLayer: StreamCoderLayer {
            @display("p=291,172");
        }
        packetServer: PacketServer {
            @display("p=291,353");
        }
        packetEmitter: PacketEmitter {
            @display("p=291,448");
        }
        ieee8021qTimeAwareShaper: Ieee8021qTimeAwareShaper {
            @display("p=290.496,262.728");
        }
    connections allowunconnected:
        upperLayerIn --> IdealChannel --> packetDelayer.in;
        packetDelayer.out --> IdealChannel --> streamIdentifierLayer.upperLayerIn;
        streamIdentifierLayer.lowerLayerOut --> IdealChannel --> streamCoderLayer.upperLayerIn;
        streamCoderLayer.upperLayerOut --> IdealChannel --> streamIdentifierLayer.lowerLayerIn;



        packetServer.out --> IdealChannel --> packetEmitter.in;
        packetEmitter.out --> IdealChannel --> lowerLayerOut;
        streamIdentifierLayer.upperLayerOut --> IdealChannel --> upperLayerOut;
        streamCoderLayer.lowerLayerOut --> IdealChannel --> ieee8021qTimeAwareShaper.in;
        ieee8021qTimeAwareShaper.out --> IdealChannel --> packetServer.in;
}
