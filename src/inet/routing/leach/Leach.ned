//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.leach;

import inet.routing.contract.IManetRouting;

//
// This module provides Low-Energy Adaptive Clustering Hierarchy (LEACH) 
// routing protocol for sensor nodes. 
// LEACH is a hierachical adhoc routing protocol for Wireless Sensor Networks. 
// This implementation is designed to use a a single wireless interface and run on
// the IEEE 802.15.4 standard for MAC and Physical layers.
// Therefore the standard TDMA scheduling system has been implemented 
// with delayed time slots due to the use of CSMA in the 802.15.4 standard.
//
// For more information on the algorithm, see the LEACH paper
// https://ieeexplore.ieee.org/document/926982
//
simple Leach like IManetRouting
{
    parameters:
        // properties
        @class("leach::Leach");
        @display("i=block/routing");

        // context parameters
        string interfaceTableModule;

        // LEACH parameter group
        volatile double startupDelay @unit(s) = default(uniform(0s, 1s));
        double clusterHeadRatio @unit(ratio) = default(0.05ratio); 	// Determines ratio of cluster heads in network, which is used for threshold function
        int roundDurationVariance = default(10);
    gates:
        input ipIn;
        output ipOut;
}
