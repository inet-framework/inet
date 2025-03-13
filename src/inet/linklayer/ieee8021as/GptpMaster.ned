//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

//
// Implements a Grandmaster Clock node in an IEEE 802.1AS network. This module
// serves as the primary time reference for the entire time-sensitive network,
// providing the source of time to which all other devices synchronize.
//
// The gPTP master:
// - Acts as the root timing reference for the entire network
// - Distributes its time to downstream devices (bridges and slaves)
// - Has its gPTP node type configured as MASTER_NODE
// - Typically has routing and proxy ARP disabled as it often serves a dedicated function
//
// In a gPTP network, the master clock is selected based on the Best Master Clock
// Algorithm (BMCA), but in simulations, it is often explicitly designated. The master
// clock should have high stability and accuracy characteristics to ensure the entire
// network maintains precise synchronization.
//
// This module extends ~GptpEndstation with specific configuration for the master role,
// making it suitable for use in time-sensitive networking simulations requiring
// a definitive time source.
//
// @see ~GptpSlave, ~GptpBridge, ~Gptp
//
module GptpMaster extends GptpEndstation
{
    parameters:
        @display("i=device/pc2");
        forwarding = default(false);  // Disable routing by default
        networkLayer.proxyARP = default(false);
        gptp.gptpNodeType = "MASTER_NODE"; // @enum("gptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
    connections allowunconnected:
}
