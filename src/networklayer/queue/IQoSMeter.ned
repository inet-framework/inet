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
moduleinterface IQoSMeter
{
    parameters:
        @display("i=block/subqueue");
	gates:
}