//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.leach;

import inet.node.inet.AdhocHost;
import inet.routing.leach.Leach;

//
// This module provides the sensor node implementation of 
// Low-Energy Adaptive Clustering Hierarchy (LEACH) routing, a hierachical 
// adhoc routing protocol for Wireless Sensor Networks. 
// This implementation is designed to use a a single wireless interface and run on
// the IEEE 802.15.4 standard for MAC and Physical layers.
// Therefore the standard TDMA scheduling system has been implemented 
// with delayed time slots due to the use of CSMA in the 802.15.4 standard.
//
// For more information on the algorithm, see the LEACH paper
// https://ieeexplore.ieee.org/document/926982
//
module LeachNode extends AdhocHost
{
    submodules:
        LeachNode: Leach {
            @display("p=825,226");
        }
    connections:
        LeachNode.ipOut --> tn.in++;
        LeachNode.ipIn <-- tn.out++;
}
