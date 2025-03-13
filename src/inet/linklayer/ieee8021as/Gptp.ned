//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.linklayer.contract.IGptp;

//
// Implements the IEEE 802.1AS Generalized Precision Time Protocol (gPTP) for precise
// time synchronization in Ethernet networks. This protocol enables network devices
// to achieve sub-microsecond clock synchronization, which is essential for
// time-sensitive networking applications.
//
// The module performs several key functions:
// - Measures link delays to neighboring gPTP nodes through peer delay mechanism
// - Exchanges timing information through Sync, Follow_Up, and other messages
// - Calculates clock offsets and rate ratios between nodes
// - Periodically updates the local clock to maintain synchronization
// - Supports multiple time synchronization domains
//
// The module can operate in three different roles:
// - Master: Provides the reference time for the network
// - Bridge: Forwards and processes time synchronization messages
// - Slave: Synchronizes its local clock to the master clock
//
// The slavePort parameter specifies the port for receiving time from upstream nodes,
// while masterPorts specifies the ports for distributing time to downstream nodes.
// Various timing parameters control the frequency of synchronization messages and
// delay measurements.
//
// @see ~GptpMaster, ~GptpSlave, ~GptpBridge, ~MultiDomainGptp
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
        // These numbers must be large enough to prevent creating a queue in the MAC layer.
        // It means they should be larger than the transmission time of the message sent before
        double pDelayReqProcessingTime @unit(s) = default(8us);  // Processing time between the arrival of PDelayReq and the sending of PDelayResp
        double followUpInterval @unit(s) = default(7us);

        @display("i=block/timer");
        @signal[localTime](type=simtime_t);  // As clocktime_t
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
