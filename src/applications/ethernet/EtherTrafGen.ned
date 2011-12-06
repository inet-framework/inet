//
// Copyright (C) 2011 Andras Varga
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

package inet.applications.ethernet;

//
// A simple traffic generator for the Ethernet model and the 802.11 model,
// and generally for any L2 model that accepts Ieee80211Ctrl control info
// on the packets. It should be connected directly to ~EtherEncap or 
// an Ieee802NicXXX module.
//
simple EtherTrafGen
{
    parameters:
        string destAddress = default("");  // destination MAC address, or module path name of destination station; empty means off
        int etherType = default(0);  // etherType to set on outgoing packets
        double startTime @unit(s) = default(this.sendInterval);  // time of sending the first packet
        double stopTime @unit(s) = default(0);  // time of finish sending, 0 means forever
        volatile double sendInterval @unit(s);  // interval between sending bursts
        volatile int numPacketsPerBurst = default(1);  // number of packets to send per burst (packets within a burst are sent at the same simulation time)
        volatile int packetLength @unit(B);  // length of packets to send
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPkBytes](title="Received packet bytes"; source="packetBytes(rcvdPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[sentPkBytes](title="Sent packet bytes"; source="packetBytes(sentPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,stats,vector; interpolationmode=none);
    gates:
        input in @labels(Ieee802Ctrl/up);
        output out @labels(Ieee802Ctrl/down);
}

