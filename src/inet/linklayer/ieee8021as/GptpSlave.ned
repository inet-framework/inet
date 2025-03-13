//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

//
// Implements an end device that synchronizes its clock to the grandmaster in an
// IEEE 802.1AS network. This module represents devices that need precise time
// synchronization but do not distribute timing information to other devices.
//
// The gPTP slave:
// - Receives timing information from upstream devices (master or bridges)
// - Adjusts its local clock to match the grandmaster clock
// - Has its gPTP node type configured as SLAVE_NODE
// - Typically has routing and proxy ARP disabled as it often serves a dedicated function
//
// Slave devices are the endpoints in the time synchronization hierarchy, representing
// equipment that requires precise timing for their operation, such as:
// - Audio/video equipment in professional media networks
// - Industrial controllers in automation systems
// - Sensors and actuators in control networks
// - End stations in automotive networks
//
// This module extends ~GptpEndstation with specific configuration for the slave role,
// making it suitable for use in time-sensitive networking simulations requiring
// synchronized end devices.
//
// @see ~GptpMaster, ~GptpBridge, ~Gptp
//
module GptpSlave extends GptpEndstation
{
    parameters:
        @display("i=device/pc2");
        forwarding = default(false);  // Disable routing by default
        networkLayer.proxyARP = default(false);
        gptp.gptpNodeType = "SLAVE_NODE"; // @enum("gptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
    connections allowunconnected:
}
