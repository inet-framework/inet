//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

package ieee8021as;

//
// this will be used to store a value of correctionField and rateRatio
//
simple TableGPTP
{
    parameters:
        string nodeType = default("SlaveNode"); // MasterNode, BridgeNode and SlaveNode
        double correctionField = default(0);
        double rateRatio = default(1);
        double syncReceivedTime = default(0);
    gates:
        input gptpLayerIn[];
        output gptpLayerOut[];
}
