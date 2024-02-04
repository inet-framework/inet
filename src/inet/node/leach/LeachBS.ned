//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.leach;

import inet.node.inet.AdhocHost;
import inet.routing.leach.LeachBS;

//
// This module provides the data sink/base station implementation of 
// Low-Energy Adaptive Clustering Hierarchy (LEACH) routing, a hierachical 
// adhoc routing protocol for Wireless Sensor Networks. 
// This implementation is designed to use a a single wireless interface and run on
// the IEEE 802.15.4 standard for MAC and Physical layers.
// This module simply receives data from the sensor nodes.
//
// For more information on the algorithm, see the LEACH paper
// https://ieeexplore.ieee.org/document/926982
//
module LeachBS extends AdhocHost
{
    submodules:
        LeachBS: LeachBS {
            @display("p=825,226");
        }
    connections:
        LeachBS.ipOut --> tn.in++;
        LeachBS.ipIn <-- tn.out++;
}
