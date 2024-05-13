//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.linklayer.contract.IGptp;

//
// This module implements the IEEE 802.1as protocol also known as gPTP. It
// measures link delays to neighboring gPTP network nodes periodically. The
// slave and master ports specify where the connected gPTP network nodes
// and their roles in the time synchronization domain. The time synchronization
// is done periodically and the clock module is set.
//
simple Gptp like IGptp
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        string interfaceTableModule; // Relative path of the interface table module
        string gptpNodeType; // @enum("GptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
        int domainNumber = default(0); // Specifies the time domain number used in gPTP messages
        string slavePort = default(""); // Port for receiving time (empty for MASTER_NODE)
        object masterPorts = default([]); // Ports for sending out time (empty for SLAVE_NODE)
        double correctionField @unit(s) = default(0s); // Time correction for link delay measurements
        double syncInterval @unit(s) = default(0.125s); // Time interval between SYNC messages
        double pdelayInterval @unit(s) = default(1s); // Frequency of link delay measurements
        double syncInitialOffset @unit(s) = default(syncInterval); // Time of first SYNC message
        double pdelayInitialOffset @unit(s) = default(0s); // Time of first link delay measurement

        // following parameters are used to schedule follow_up and pdelay_resp messages.
        // These numbers must be enough large to prevent creating queue in MAC layer.
        // It means it should be large than transmission time of message sent before
        double pDelayReqProcessingTime @unit(s) = default(8us);  // Processing time between arrived PDelayReq and send of PDelayResp
        double followUpInterval @unit(s) = default(7us);

        @display("i=block/timer");
        @signal[localTime](type=simtime_t);  // As clocktime_t
        @signal[timeDifference](type=simtime_t);
        @signal[rateRatio](type=double);
        @signal[peerDelay](type=simtime_t);
        @statistic[localTime](record=vector; interpolationmode=linear);
        @statistic[timeDifference](record=vector; interpolationmode=linear);
        @statistic[rateRatio](record=vector; interpolationmode=sample-hold);
        @statistic[peerDelay](record=vector; interpolationmode=sample-hold);
        @selfMessageKinds(inet::GptpSelfMsgKind);

    gates:
        input socketIn;
        output socketOut;
}

