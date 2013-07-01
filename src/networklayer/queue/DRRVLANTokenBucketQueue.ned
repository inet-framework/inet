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
// allocation. It consists of a FIFO queue and DDR queues joined with priority queueing
// with high priority for the FIFO queue. Incoming packets are classfied by a VLAN
// classifier and metered by token buckets; conformant packets are put into the FIFO queue,
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-07-01
//
module DRRVLANTokenBucketQueue
{
    parameters:
        @labels(node,hybridpon-node);
        @display("i=block/subqueue;bgb=130,90");
        int frameCapacity = default(100);  // per-subqueue capacity
        int numQueues = default(1);	// number of per-VLAN queues
		string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier
    gates:
        output out;
    submodules:
        tbm[numQueues]: BasicTokenBucketMeter {
            parameters:
                @display("i=block/classifier;p=60,40");
        }
}
