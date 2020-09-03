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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.linklayer.ethernet.layered;

import inet.protocol.fragmentation.FragmentTagBasedDefragmenter;

module EthernetFragmentingMacLayer extends EthernetMacLayer
{
    parameters:
        server.typename = default("OmittedPacketServer");
        streamer.typename = default("PreemptableStreamer");
        streamer.minPacketLength = 60B;
        fcsInserter.typename = default("EthernetFragmentFcsInserter");
        preambleInserter.typename = default("EthernetFragmentPreambleInserter");
        transmitter.typename = default("StreamThroughTransmitter");
        fcsChecker.typename = default("EthernetFragmentFcsChecker");
        preambleChecker.typename = default("EthernetFragmentPreambleChecker");
    submodules:
        defragmenter: FragmentTagBasedDefragmenter {
            @display("p=500,300");
        }
    connections:
        fcsChecker.out --> { @reconnect; } --> defragmenter.in;
        defragmenter.out --> { @reconnect;@display("m=n"); } --> upperLayerOut;
}
