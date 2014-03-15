//
// Copyright (C) 2014 Kyeong Soo (Joseph) Kim
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
// Drop-tail queue with VLAN classifier,  token bucket filter (TBF) traffic
// shaper based on external token bucket meter, and round-robin (RR) scheduler
// to be used in network interfaces.
// Compared to DropTailVLANTBFQueue, major difference is the capability to
// assign different TBF parameters per VOQ.
// Conforms to the OutputQueue interface.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2014-02-23
//
simple DropTailVLANTBFQueue2 like OutputQueue
{
    parameters:
        // int frameCapacity = default(100);  // per-subqueue capacity
        int numFlows = default(1);	// number of per-VLAN queues
        int voqSize @unit(B) = default(15220000B);	// per-flow virtual output queue (VOQ) size; default value corresponds to 10,000 maximum-sized VLAN frames.
		string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier
		string vids = default("");	// VIDs used for initialization of index table
//        int bucketSize @unit(byte) = default(3000B);
//			// token bucket size; must be larger than mtu.
//			// note that with a finite line/peak rate,
//			// the maximum burst size is not equal to token bucket size.
//		double meanRate @unit(bps) = default(10Mbps);	// token generation rate
//		int mtu @unit(byte) = default(1522B);	// for Ethernet frame with header(18) + VLAN tag (4) + maximum payload (1500)
//		double peakRate @unit(bps) = default(100Mbps);	// must be larger than meanRate
        @display("i=block/queue");
    gates:
        input in;
        output out;
}



