//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.networklayer.queue;

import inet.networklayer.queue.BasicTokenBucketMeter;

//
// Model for an ISP traffic controller enabling proportional excess bandwidth
// allocation.
// Incoming packets are first classfied by a VLAN classifier and then metered
// by token buckets. Conformant packets are put into a common FIFO queue, while
// non-conformant packets are put into per-subscriber queues served by a DRR
// scheduler, which are joined by priority queueing with high priority for the
// FIFO queue.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-07-01
//
module DRRVLANTokenBucketQueue
{
    parameters:
        @labels(node,hybridpon-node);
        @display("i=block/subqueue;bgb=130,90");
        int numFlows = default(1);	// number of flows (i.e., subscribers)
        int queueSize @unit(b) = default(100000000b);  // FIFO queue size
        int queueThreshold @unit(b) = default(50000000b);	// FIFO threshold to detect link congestion
        string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier        
		string vids = default("");	// VIDs used for initialization of index table
    gates:
        output out;
    submodules:
        tbm[numFlows]: BasicTokenBucketMeter {
            parameters:
                @display("i=block/classifier;p=60,40");
        }
}
