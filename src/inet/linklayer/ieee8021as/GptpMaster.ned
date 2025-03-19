//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

//
// Provides a gPTP master network node.
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
