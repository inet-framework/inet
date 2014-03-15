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
// Model for an ISP traffic controller enabling proportional allocation of
// excess bandwidth allocation.
// An incoming packet is first classfied by a VLAN classifier and then metered
// by token buckets. If conformant, the packet is put into a common FIFO queue;
// if not conformant, it is put into per-subscriber queues served by a DRR
// scheduler, which are joined by priority queueing with high priority for the
// FIFO queue.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-07-01
//
simple DRRVLANQueue like OutputQueue
{
    parameters:
        @labels(node,queue-node);
        @display("i=block/subqueue;bgb=130,90");
        int numFlows = default(1);	// number of flows (i.e., subscribers)
//		int frameCapacity = default(100);
		int fifoSize @unit(B) = default(15220000B);	// FIFO queue size; default value corresponds to 10,000 maximum-sized VLAN frames.
        int voqSize @unit(B) = default(15220000B);	// per-flow virtual output queue (VOQ) size; default value corresponds to 10,000 maximum-sized VLAN frames.
        string quanta = default("1522");	// DRR quantum sizes in byte
        string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier        
		string vids = default("");	// VIDs used for initialization of index table
    gates:
        input in;
        output out;
}
