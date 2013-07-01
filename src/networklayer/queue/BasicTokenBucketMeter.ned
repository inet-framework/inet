//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.networklayer.queue;

//
// Interface for QoS meter implementations.
// All QoS metersshould implement this (e.g., BasicTokenBucketMeter).
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-07-01
//
simple BasicTokenBucketMeter
{
    parameters:
        @display("i=block/subqueue");
        int bucketSize @unit(byte) = default(3000B);
			// token bucket size; must be larger than mtu.
			// note that with a finite line/peak rate,
			// the maximum burst size is not equal to token bucket size.
		double meanRate @unit(bps) = default(10Mbps);	// token generation rate
		int mtu @unit(byte) = default(1522B);	// for Ethernet frame with header(18) + VLAN tag (4) + maximum payload (1500)
		double peakRate @unit(bps) = default(100Mbps);	// must be larger than meanRate
}
