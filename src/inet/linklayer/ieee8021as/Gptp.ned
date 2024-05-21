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
// slave and master ports specify where are the connected gPTP network nodes
// and their roles in the time synchronization domain. The time synchronization
// is done periodically and the clock module is set.
//
simple Gptp like IGptp
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        string interfaceTableModule; // relative path of the interface table module
        string gptpNodeType; // @enum("GptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
        int domainNumber = default(0); // specifies the time domain number used in gPTP messages
        string slavePort = default(""); // port for receiving time (empty for MASTER_NODE)
        object masterPorts = default([]); // ports for sending out time (empty for SLAVE_NODE)
        double correctionField @unit(s) = default(0s); // time correction for link delay measurements
        double syncInterval @unit(s) = default(0.125s); // time interval between SYNC messages
        double pdelayInterval @unit(s) = default(1s); // frequency of link delay measurements
        double syncInitialOffset @unit(s) = default(syncInterval); // time of first SYNC message
        double pdelayInitialOffset @unit(s) = default(0s); // time of first link delay measurement

        // following parameters are used to schedule follow_up and pdelay_resp messages.
        // These numbers must be enough large to prevent creating queue in MAC layer.
        // It means it should be larger than transmission time of message sent before
        double pDelayReqProcessingTime @unit(s) = default(8us);  // processing time between arrived PDelayReq and send of PDelayResp

        @display("i=block/timer");
        @signal[localTime](type=simtime_t);  // as clocktime_t
        @signal[timeDifference](type=simtime_t);
        @signal[gmRateRatio](type=double);
        @signal[receivedRateRatio](type=double);
        @signal[neighborRateRatio](type=double);
        @signal[peerDelay](type=simtime_t);
        @signal[residenceTime](type=simtime_t);
        @signal[correctionFieldIngress](type=simtime_t);
        @signal[correctionFieldEgress](type=simtime_t);
        @statistic[localTime](record=vector; interpolationmode=linear);
        @statistic[timeDifference](record=vector; interpolationmode=linear);
        @statistic[gmRateRatio](record=vector; interpolationmode=sample-hold);
        @statistic[receivedRateRatio](record=vector; interpolationmode=sample-hold);
        @statistic[neighborRateRatio](record=vector; interpolationmode=sample-hold);
        @statistic[peerDelay](record=vector; interpolationmode=sample-hold);
        @statistic[residenceTime](record=vector;interpolationmode=none);
        @statistic[correctionFieldIngress](record=vector; interpolationmode=none);
        @statistic[correctionFieldEgress](record=vector; interpolationmode=none);
        @selfMessageKinds(inet::GptpSelfMsgKind);

    gates:
        input socketIn;
        output socketOut;
}

