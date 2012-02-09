//
// Copyright (C) 2011-2012 Kyeong Soo (Joseph) Kim
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


package inet.linklayer.etherswitch;

//
// Implementation of MACRelayUnitNP with VLAN capability.
// For details other than traffic control, refer to MACRelayUnitNP.
//
// @see MACRelayUnitNP
//
simple MACRelayUnitNPWithVLAN like MACRelayUnit
{
    parameters:
        string addressTableFile = default("");  // see MACRelayUnit
        int addressTableSize = default(100); // see MACRelayUnit
        double agingTime @unit("s") = default(300.0s); // recommended by IEEE 802.1Q; see MACRelayUnit
        int numCPUs = default(1);  // number of CPUs
        double processingTime @unit("s") = default(0s);  // processing time of one frame
        int bufferSize @unit("B") = default(1MiB);  // memory
        int highWatermark @unit("B") = default(512KiB);  // buffer usage threshold to send PAUSE frame
        int pauseUnits = default(300);  // time to put in PAUSE frames (in units of 512 bit times)
        @display("i=block/switch");
    gates:
        input lowerLayerIn[] @labels(EtherFrame);
        output lowerLayerOut[] @labels(EtherFrame);
}

