//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

// Allows to set only one flow control credit limit for all stream types. It has no parameter specfic for unidirectional, bidirectional local or bidrectional remote streams.

package inet.transportlayer.quic;

import inet.transportlayer.contract.IQuic;

simple Quic like IQuic
{
    parameters:
        string routingTableModule; // Set by StandardHost.

        // Congestion Control
        string congestionControl = default("NewReno"); // Sets the congestion control algorithm (NewReno or No).
        bool accurateIncreaseInNewRenoCongestionAvoidance = default(true); // For NewReno in Congestion Avoidance, if true, increases cwnd after cwnd-acked bytes; if false, increases cwnd on every acked packet by acked_bytes/cwnd.

        // Send Queue (to buffer data to send from the app)
        int sendQueueLimit @unit(B) = default(64MB); // Sets the limit of the send queue in bytes.
        double sendQueueLowWaterRatio = default(.4); // Defines the send queue low water mark relative to the limit.

		// Flow Control
        int initialMaxData @unit(B) = default(15MB); // Connection level flow control credit. Used as own and peer's value.
        int initialMaxStreamData @unit(B) = default(6MB); // Stream level flow control credit for any stream (unidirectional, bidirectional local and remote). Used as own and peer's value.
        double maxDataFrameThreshold @unit(B) = default(0.5*initialMaxData); // Defines when MaxDataFrame in ConnectionFlowControl will be send.
        double maxStreamDataFrameThreshold @unit(B) = default(0.5*initialMaxStreamData); // Defines when MaxStreamDataFrame in StreamFlowControl will be send.
        bool roundConsumedDataValue = default(false); // If true, FlowControlResponder rounds consumedData in streamRcvBuffer to the next packetSize.
        bool sendMaxDataFramesImmediately = default(false); // If true, a MAX_(STREAM)_DATA frame is sent immediately, after generation. Otherwise, sending the frame is delayed until the sending process starts due to another event.

		// Acknowledgments
        bool bundleAckForNonAckElicitingPackets = default(false); // If true, bundles ack for non-ack-eliciting packets in an outgoing packet.
        int numReceivedAckElicitingsBeforeAck = default(2); // Number of ack eliciting packets a receiver waits for, before it sends an ACK, immediately.
        bool useIBit = default(false); // If true, an ACK frame is sent immediately upon an ack-eliciting packet with the I-Bit set.

		// Path MTU Discovery
        bool useDplpmtud = default(false); // If true, uses DPLPMTUD (RFC8899) to determine the PMTU.
        int maxPmtu @unit(B) = default(65535B); // With useDplpmtud, it is used as upper limit for the PMTU search. Without useDplpmtud, the PMTU is set to the minimum of the MTU of the outgoing interface and this value.
        int dplpmtudMinPmtu = default(0); // Specifies the minimum PMTU to use (if 0, it uses 1200 B for IPv4 or 1280 B for IPv6 as minimum PMTU).
        int dplpmtudMaxProbes = default(3); // Specifies the number of probe packet sent for one candidate before given up.
        bool dplpmtudUsePtb = default(true); // If true, uses ICMP Packet Too Big (PTB) messages for DPLPMTUD.
        string dplpmtudCandidateSequence = default("OptBinary"); // Specifies the sequence of candidates used for the search algorithm of DPLPMTUD (Up, Down, OptUp, Binary, OptBinary, Jump).
        bool skipPacketNumberForDplpmtudProbePackets = default(false); // Specifies whether to skip a packet number for a DPLPMTUD probe packet in order to avoid ack delay.
        int useCwndForParallelProbes = default(0); // If 0, no parallel probe packets; if 1, uses the availbale cwnd to send additional probe packets and if 2 prefers to send probe packets over other packets.
        bool sendDataDuringInitalDplpmtudBase = default(true); // If false, waits for DPLPMTUD switching initially to SEARCH before start sending application data.
        bool dplpmtudUsePmtuValidator = default (true); // If true, activates the PMTU Validator that trys to detect when the current PMTU estimation is larger than the actual PMTU.
        int pmtuValidatorLostPacketsThreshold = default(2); // Specifies the number of lost data packets before assuming a decreased PMTU.
        double pmtuValidatorTimeThreshold @unit(s) = default(-1s); // Specifies the time threshold between two lost packets (if negative, the number of seconds are used multiplied by SRTT).
        int pmtuValidatorInvalidOnPersistentCongestion = default(1); // If 0, disabled. If 1, the PMTU validator uses persistent congeation as signal for a decreased PMTU. If 2, only two consecutive persisten congestions used as signal. If 3, ...
        double reducePacketTimeThreshold @unit(s) = default(0s); // Specifies after which time with no acks, packets with reduced size are sent (if negative, the number of seconds are used multiplied by PTO).

        @display("i=block/transport");

        // Connection
        @signal[packetReceived](type=cPacket); // should be per connection
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @signal[packetSent](type=cPacket); // should be per connection
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @signal[packetNumberReceived*];
        @statisticTemplate[packetNumberReceived](title="packet number received"; record=vector; interpolationmode=none);
        @signal[packetNumberSent*];
        @statisticTemplate[packetNumberSent](title="packet number sent"; record=vector; interpolationmode=none);
        @signal[totalRcvAppData*];
        @statisticTemplate[totalRcvAppData](title="total received app data"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[sendBufferUnsentAppData*];
        @statisticTemplate[sendBufferUnsentAppData](title="send buffer unsent app data"; record=vector; interpolationmode=sample-hold; unit=B);

        // Stream
        @signal[streamRcvAppData*];
        @statisticTemplate[streamRcvAppData](title="stream received app data"; record=vector,vector(sum); interpolationmode=none,sample-hold; unit=B);
        @signal[streamRcvFrameStartOffset*];
        @statisticTemplate[streamRcvFrameStartOffset](title="stream received frame start offset"; record=vector; interpolationmode=none; unit=B);
        @signal[streamRcvFrameEndOffset*];
        @statisticTemplate[streamRcvFrameEndOffset](title="stream received frame end offset"; record=vector; interpolationmode=none; unit=B);

        // Reliability and Congestion Control
        @signal[cwnd*];
        @statisticTemplate[cwnd](title="congestion window"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[latestRtt*](type=simtime_t);
        @statisticTemplate[latestRtt](title="latest RTT"; record=vector; interpolationmode=none);
        @signal[bytesInFlight*];
        @statisticTemplate[bytesInFlight](title="bytes in flight"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[partialBytesAcked*];
        @statisticTemplate[partialBytesAcked](title="partial bytes acked"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[persistentCongestionPeriod*](type=simtime_t);
        @statisticTemplate[persistentCongestionPeriod](title="persistent congestion period"; record=vector; interpolationmode=none;);

        // Flow Control
        @signal[availableRwnd*];
        @statisticTemplate[availableRwnd](title="available receive window"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[generatedBlockFrameCount*];
        @statisticTemplate[generatedBlockFrameCount](title="number of generated (STREAM_)DATA_BLOCKED frames"; record=last);
        @signal[blockFrameOffset*];
        @statisticTemplate[blockFrameOffset](title="offsets in (STREAM_)DATA_BLOCKED frames"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[rcvMaxFrameCount*];
        @statisticTemplate[rcvMaxFrameCount](title="number of received MAX_(STREAM_)DATA frames"; record=last);

        // Flow Control Responder
        @signal[rcvBlockFrameCount*];
        @statisticTemplate[rcvBlockFrameCount](title="number of received (STREAM_)DATA_BLOCKED frames"; record=last);
		@signal[generatedMaxDataFrameCount*];
        @statisticTemplate[generatedMaxDataFrameCount](title="number of generated MAX_(STREAM_)DATA frames"; record=last);
        @signal[maxDataFrameOffset*];
        @statisticTemplate[maxDataFrameOffset](title="offsets in MAX_(STREAM_)DATA frames"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[consumedData*];
        @statisticTemplate[consumedData](title="data consumed by app"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[maxDataFrameLostCount*];
        @statisticTemplate[maxDataFrameLostCount](title="number of lost MAX_(STREAM_)DATA frames"; record=last);

        // Path MTU Discovery
        @signal[usedMaxQuicPacketSize*];
        @statisticTemplate[usedMaxQuicPacketSize](title="QUIC packet size limit"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[dplpmtudSearchPmtu*];
        @statisticTemplate[dplpmtudSearchPmtu](title="PMTU of DPLPMTUD"; record=vector; interpolationmode=sample-hold; unit=B);
        @signal[dplpmtudSearchNumberOfTests*];
        @statisticTemplate[dplpmtudSearchNumberOfTests](title="number of tested PMTU candidates"; record=last);
        @signal[dplpmtudSearchTime*](type=simtime_t);
        @statisticTemplate[dplpmtudSearchTime](title="time required by the last PMTU search"; record=last);
        @signal[dplpmtudSearchNetworkLoad*];
        @statisticTemplate[dplpmtudSearchNetworkLoad](title="network load caused by the last PMTU search"; record=last; unit=B);
        @signal[dplpmtudInvalidSignals*];
        @statisticTemplate[dplpmtudInvalidSignals](title="largest acked packet size at PMTU invalid signal"; record=vector; ; interpolationmode=none; unit=B);
        @signal[dplpmtudNumOfProbesBeforeAck*];
        @statisticTemplate[dplpmtudNumOfProbesBeforeAck](title="number of probe packets sent before receiving ack"; record=vector; interpolationmode=none);
        @signal[pmtuValidatorNumberOfLostPackets*];
        @statisticTemplate[pmtuValidatorNumberOfLostPackets](title="number of lost packets for PMTU validation"; record=vector; interpolationmode=sample-hold);
        @signal[pmtuValidatorTimeBetweenLostPackets*](type=simtime_t);
        @statisticTemplate[pmtuValidatorTimeBetweenLostPackets](title="sent time difference of lost packets for PMTU validation"; record=vector; interpolationmode=sample-hold);
        @signal[pmtuValidatorPersistentCongestions*];
        @statisticTemplate[pmtuValidatorPersistentCongestions](title="number of consecutive persistent congestions for PMTU validation"; record=vector; interpolationmode=sample-hold);

    gates:
        input appIn @labels(QuicCommand/down) @messageKinds(inet::QuicCommandCode);
        input udpIn @labels(QuicHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(QuicCommand/up) @messageKinds(inet::QuicStatusInd);
        output udpOut @labels(QuicHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}
