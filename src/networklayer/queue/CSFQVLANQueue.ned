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
simple CSFQVLANQueue like OutputQueue
{
    parameters:
        @labels(node,queue-node);
        @display("i=block/queue");
        int numFlows = default(1);	// number of flows (i.e., subscribers)
//		int frameCapacity = default(100);
		double linkRate @unit(bps) = default(100Mbps);	// transmission rate of output link (i.e., feeder line rate (R_F))
        int queueSize @unit(B) = default(15220000B);	// FIFO queue size; default value corresponds to 10000 frames of 1522B each.
        int queueThreshold @unit(B) = default(3805000B);	// FIFO threshold to detect link congestion; default value is a quarter of default queueSize.
        double K @unit(s) = default(100ms);	// averaging constant for flow rate estimation
        double K_alpha @unit(s) = default(200ms);	// averaging constant for fair rate (alpha) estimation
        string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier
        string vids = default("");	// VIDs used for initialization of index table
    gates:
        input in;
        output out;
}



