//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.applications.contract.IApp;

//
// This module implements the IEEE 802.1as protocol also known as gPTP. It
// measures link delays to neighboring gPTP network nodes periodically. The
// slave and master ports specify where are the connected gPTP network nodes
// and their roles in the time synchronization domain. The time synchronization
// is done periodically and the clock module is set.
//
simple Gptp like IApp
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
        // It means it should be large than transmission time of message sent before
        double pDelayReqProcessingTime @unit(s) = default(8us);  // processing time between arrived PDelayReq and send of PDelayResp
        double followUpInterval @unit(s) = default(7us);

        @display("i=block/timer");
        @signal[localTime](type=simtime_t);  // as clocktime_t
        @signal[timeDifference](type=simtime_t);
        @signal[rateRatio](type=double);
        @signal[peerDelay](type=simtime_t);
        @signal[packetDropped](type=inet::Packet);
        @statistic[localTime](record=vector; interpolationmode=linear);
        @statistic[timeDifference](record=vector; interpolationmode=linear);
        @statistic[rateRatio](record=vector; interpolationmode=sample-hold);
        @statistic[peerDelay](record=vector; interpolationmode=sample-hold);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @selfMessageKinds(inet::GptpSelfMsgKind);

    gates:
        input socketIn;
        output socketOut;
}

