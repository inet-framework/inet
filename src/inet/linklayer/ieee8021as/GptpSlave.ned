//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

//
// Provides a gPTP slave network node.
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

