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

import inet.networklayer.queue.BasicTokenBucketMeter;

//
// Model for an ISP traffic controller enabling proportional allocation of
// excess bandwidth allocation.
// An incoming packet is first classfied by a VLAN classifier and then metered
// by token buckets. If conformant, the packet is put into a common FIFO queue
// without any processing; if not conformant, it goes through core-stateless fair
// queueing (CSFQ), dropping if needed, and then put into a common FIFO queue.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-07-01
//
simple CSFQVLANTokenBucketQueue like OutputQueue
{
    parameters:
        @labels(node,queue-node);
        @display("i=block/queue");
        int numFlows = default(1);	// number of flows (i.e., subscribers)
//		int frameCapacity = default(100);
        int queueSize @unit(b) = default(100000000b);  // FIFO queue size
        int queueThreshold @unit(b) = default(50000000b);	// FIFO threshold to detect link congestion
        string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier
        string vids = default("");	// VIDs used for initialization of index table
    gates:
        input in;
        output out;
}



