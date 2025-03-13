//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.node.ethernet.EthernetSwitch;

//
// Implements a network bridge with IEEE 802.1AS time synchronization capabilities.
// This module represents intermediate devices in a time-sensitive network that
// forward both data traffic and time synchronization messages.
//
// The gPTP bridge:
// - Forwards timing information from the grandmaster to downstream devices
// - Compensates for link delays in the synchronization path
// - Maintains its own synchronized clock
// - Combines Ethernet switching functionality with time synchronization
//
// Bridges play a critical role in the time synchronization hierarchy by:
// - Accurately measuring and compensating for link delays
// - Forwarding Sync and Follow_Up messages with appropriate corrections
// - Participating in the peer delay mechanism with neighboring devices
// - Maintaining precise synchronization while forwarding time information
//
// This module extends the standard ~EthernetSwitch with gPTP capabilities,
// enabling it to participate in the IEEE 802.1AS time synchronization domain
// while performing its normal switching functions.
//
// @see ~GptpMaster, ~GptpSlave, ~Gptp
//
module GptpBridge extends EthernetSwitch
{
    parameters:
        hasGptp = true;
}
