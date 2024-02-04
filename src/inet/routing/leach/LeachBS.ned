//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.leach;

import inet.routing.contract.IManetRouting;

//
// This module provides Low-Energy Adaptive Clustering Hierarchy (LEACH) 
// routing protocol for the data sink/base station. 
// This implementation is designed to use a a single wireless interface and run on
// the IEEE 802.15.4 standard for MAC and Physical layers.
// This module simply receives data from the sensor nodes.
//
// For more information on the algorithm, see the LEACH paper
// https://ieeexplore.ieee.org/document/926982
//
simple LeachBS like IManetRouting
{
    parameters:
        // properties
        @class("leach::LeachBS");
        @display("i=block/routing");
        
        // context parameters
        string interfaceTableModule;   
    gates:
        input ipIn;
        output ipOut;
}
