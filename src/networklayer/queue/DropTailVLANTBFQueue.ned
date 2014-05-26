//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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


package inet.networklayer.queue;

//
// Drop-tail queue with VLAN classifier, token bucket filter (TBF) traffic shaper,
// and round-robin (RR) scheduler to be used in network interfaces.
// Conforms to the OutputQueue interface.
//
simple DropTailVLANTBFQueue like OutputQueue
{
    parameters:
        int frameCapacity = default(100);  // per-subqueue capacity
        int numQueues = default(1);	// number of per-VLAN queues
		string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier
        int bucketSize @unit(byte) = default(3000B);
			// token bucket size; must be larger than mtu.
			// note that with a finite line/peak rate,
			// the maximum burst size is not equal to token bucket size.
		double meanRate @unit(bps) = default(10Mbps);	// token generation rate
		int mtu @unit(byte) = default(1522B);	// for Ethernet frame with header(18) + VLAN tag (4) + maximum payload (1500)
		double peakRate @unit(bps) = default(100Mbps);	// must be larger than meanRate
        @display("i=block/queue");
    gates:
        input in;
        output out;
}



