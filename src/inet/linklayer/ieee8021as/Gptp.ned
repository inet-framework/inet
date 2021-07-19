//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.applications.contract.IApp;


//
// ieee 802.1as gPTP
//
simple Gptp like IApp
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        string interfaceTableModule;
        string gptpNodeType; // @enum("GptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
        int domainNumber = default(0);
        string slavePort = default(""); // port for receiving time (empty for MASTER_NODE)
        object masterPorts = default([]); // ports for sending out time (empty for SLAVE_NODE)
        double correctionField @unit(s) = default(0s);
        double syncInterval @unit(s) = default(0.125s);
        double pdelayInterval @unit(s) = default(1s);
        double syncInitialOffset @unit(s) = default(syncInterval);
        double pdelayInitialOffset @unit(s) = default(0s);

        // following parameters are used to schedule follow_up and pdelay_resp messages.
        // These numbers must be enough large to prevent creating queue in MAC layer.
        // It means it should be large than transmission time of message sent before
        double pDelayReqProcessingTime @unit(s) = default(8us);  // processing time between arrived PDelayReq and send of PDelayResp
        double followUpInterval @unit(s) = default(7us);

        @display("i=block/timer");
        @signal[localTime](type=simtime_t);  // as clocktime_t
        @signal[timeDifference](type=simtime_t);
        @signal[rateRatio](type=double);
        @signal[peerDelay](type=simtime_t);
        @statistic[localTime](record=vector; interpolationmode=linear);
        @statistic[timeDifference](record=vector; interpolationmode=linear);
        @statistic[rateRatio](record=vector; interpolationmode=samplehold);
        @statistic[peerDelay](record=vector; interpolationmode=samplehold);
        @selfMessageKinds(inet::GptpSelfMsgKind);

    gates:
        input socketIn;
        output socketOut;
}

