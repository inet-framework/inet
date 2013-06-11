//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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
// Core-Stateless Fqir Queueing (CSFQ) queue with VLAN classifier and
// token bucket filter (TBF) packet marker to be used in network interfaces.
// Conforms to the OutputQueue interface.
//
simple CSFQVLANTBFQueue like OutputQueue
{
    parameters:
        int numQueues = default(1);	// number of per-VLAN queues
        string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier
        string vids = default("");	// VIDs used for initialization of index table
        int frameCapacity = default(100);  // FIFO queue capacity
		//---------------------------------------------------------------------
		// parameter values for TBFs in string
		// - either provide one value for all TBFs or multiple values whose
		// - number is equal to 'numQueues'
		//---------------------------------------------------------------------
		string bucketSize = default("3000");	// token bucket sizes in byte
//        int bucketSize @unit(byte) = default(3000B);
			// token bucket size; must be larger than mtu.
			// note that with a finite line/peak rate,
			// the maximum burst size is not equal to token bucket size.
		string meanRate = default("10000000");	// token generation rates in bps
//		double meanRate @unit(bps) = default(10Mbps);	// token generation rate
		string mtu = default("1522");			// MTUs in byte; for Ethernet frame with header(18) + VLAN tag (4) + maximum payload (1500)
//		int mtu @unit(byte) = default(1522B);	// for Ethernet frame with header(18) + VLAN tag (4) + maximum payload (1500)
		string peakRate = default("100000000");	// peak rates in bps
//		double peakRate @unit(bps) = default(100Mbps);	// must be larger than meanRate
        @display("i=block/queue");
    gates:
        input in;
        output out;
}



