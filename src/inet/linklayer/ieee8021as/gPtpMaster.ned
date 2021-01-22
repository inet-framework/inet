//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

package inet.linklayer.ieee8021as;

module GPtpMaster extends GPtpEndstation
{
    parameters:
        @display("i=device/pc2");
        string nodeType = default("MasterNode");
        forwarding = default(false);  // disable routing by default
        networkLayer.proxyARP = default(false);

        // 11 - Grandmaster, 12 - Bridge, 13 - Slave
        int gPtpNodeType = 11;
    connections allowunconnected:
}
