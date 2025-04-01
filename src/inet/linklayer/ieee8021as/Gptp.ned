//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.linklayer.contract.IGptp;

//
// Implements the IEEE 802.1as protocol also known as gPTP. It
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
        string gptpNodeType; // @enum("GptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE, BMCA_NODE
        bool useNrr = default(false); // Use neighbor rate ratio
        string gmRateRatioCalculationMethod = default("NONE"); // @enum("GptpRateRatioCalculationMethod"): NONE, NRR, DIRECT
        int domainNumber = default(0); // Specifies the time domain number used in gPTP messages
        string slavePort = default(""); // Port for receiving time (empty for MASTER_NODE and BMCA_NODE)
        object masterPorts = default([]); // Ports for sending out time (empty for SLAVE_NODE and BMCA_NODE)
        object bmcaPorts = default([]); // Ports for sending out time (only fill when BMCA_NODE is used)
        double correctionField @unit(s) = default(0s); // Time correction for link delay measurements
        double syncInterval @unit(s) = default(0.125s); // Time interval between SYNC messages
        double pdelayInterval @unit(s) = default(1s); // Frequency of link delay measurements
        double announceInterval @unit(s) = default(1s); // Frequency of Announce messages (BMCA mode only)
        double syncInitialOffset @unit(s) = default(syncInterval); // Time of first SYNC message
        double pdelayInitialOffset @unit(s) = default(0s); // Time of first link delay measurement
        double announceInitialOffset @unit(s) = default(0s); // Time of first Announce message (BMCA mode only)
        double announceTimeout @unit(s) = default(announceInterval * 3); // Timeout for Announce messages (BMCA mode only)
        double syncTimeout @unit(s) = default(syncInterval * 3); // When to assume we are out of sync
        bool resetClockStateOnSyncLoss = default(true); // Resets the clock state to INIT after sync timeout
        bool sendAnnounceImmediately = default(false); // Send Announce message immediately after entering executing BMCA (otherwise wait until next announceInterval)

        int grandmasterPriority1 @mutable = default(255); // Priority1 value for BMCA
        int clockClass @mutable = default(248); // ClockClass value for BMCA
        int clockAccuracy @mutable = default(254); // ClockAccuracy value for BMCA
        int offsetScaledLogVariance @mutable = default(17258); // OffsetScaledLogVariance value for BMCA
        int grandmasterPriority2 @mutable = default(248); // Priority2 value for BMCA

        // following parameters are used to schedule follow_up and pdelay_resp messages.
        // These numbers must be large enough to prevent creating a queue in the MAC layer.
        // It means they should be larger than the transmission time of the message sent before
        double pDelayReqProcessingTime @unit(s) = default(8us);  // Processing time between the arrival of PDelayReq and the sending of PDelayResp
        double followUpInterval @unit(s) = default(7us);

        @display("i=block/timer");
        @signal[localTime](type=simtime_t);  // As clocktime_t
        @signal[timeDifference](type=simtime_t);
        @signal[gmRateRatio](type=double);
        @signal[rateRatio](type=double);
        @signal[receivedRateRatio](type=double);
        @signal[neighborRateRatio](type=double);
        @signal[peerDelay](type=simtime_t);
        @signal[residenceTime](type=simtime_t);
        @signal[correctionFieldIngress](type=simtime_t);
        @signal[correctionFieldEgress](type=simtime_t);
        @signal[gptpSyncStateChanged](type=int);
        @signal[packetDropped](type=inet::Packet);
        @signal[gmId](type=int);
        @statistic[localTime](record=vector; interpolationmode=linear);
        @statistic[timeDifference](record=vector; interpolationmode=linear);
        @statistic[rateRatio](record=vector; interpolationmode=sample-hold);
        @statistic[peerDelay](record=vector; interpolationmode=sample-hold);
        @statistic[residenceTime](record=vector;interpolationmode=none);
        @statistic[correctionFieldIngress](record=vector; interpolationmode=none);
        @statistic[correctionFieldEgress](record=vector; interpolationmode=none);
        @statistic[gmId](record=vector; interpolationmode=sample-hold);
        @statistic[gptpSyncStateChanged](record=vector; interpolationmode=sample-hold);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @selfMessageKinds(inet::GptpSelfMsgKind);

    gates:
        input socketIn;
        output socketOut;
}

