//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.node.ethernet.EthernetSwitch;

//
// This module implements a gPTP bridge network node.
//
module GptpBridge extends EthernetSwitch
{
    parameters:
        hasGptp = true;
}
