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

package inet.transportlayer.quic;

import inet.transportlayer.contract.IQuic;

simple Quic like IQuic
{
    parameters:
        string routingTableModule; // Set by Standard(Quic)Host.

        // Congestion Control
        string congestionControl = default("NewReno"); // Sets the congestion control algorithm (NewReno or No).
        bool accurateIncreaseInNewRenoCongestionAvoidance = default(true); // For NewReno in Congestion Avoidance, if true, increases cwnd after cwnd-acked bytes; if false, increases cwnd on every acked packet by acked_bytes/cwnd.

        // Send Queue (to buffer data to send from the app)
        int sendQueueLimit @unit(B) = default(64MB); // Sets the limit of the send queue in bytes.
        double sendQueueLowWaterRatio = default(.4); // Defines the send queue low water mark relative to the limit.

		// Flow Control
        int initialMaxData @unit(B) = default(15MB); // Connection level flow control credit. Used as own and peer's value.
        int initialMaxStreamDataUni @unit(B) = default(6MB); // Stream level flow control credit for unidirectional streams. Used as own and peer's value.
        int initialMaxStreamDataBidiLocal @unit(B) = default(6MB); // Stream level flow control credit for bidirectional streams opened by the local host. Used as own and peer's value.
        int initialMaxStreamDataBidiRemote @unit(B) = default(6MB); // Stream level flow control credit for bidirectional streams opened by the remote host. Used as own and peer's value.
        double maxStreamDataFrameThreshold @unit(B) = default(0.5*initialMaxData); // Defines when MaxStreamDataFrame in StreamFlowControl will be send. Now absolte value is used. TODO: Define it as ratio = consumedData/maxReceiveOffset
        double maxDataFrameThreshold @unit(B) = default(0.5*initialMaxStreamDataUni); // Defines when MaxDataFrame in ConnectionFlowControl will be send.Now absolte value is used. TODO: Define it as ratio = consumedData/maxReceiveOffset
        double maxDataFrameDelay @unit(ms) = default(-1ms); // TODO implement this. -1 =  do not use this parameter. Indicating the maximum amount of time in milliseconds by which the endpoint will delay sending maxDataFrame.
        bool roundConsumedDataValue = default(false); // If true, FlowControlResponder round consumedData in streamRcvBuffer to the next packetSize.
        bool sendMaxDataFramesImmediately = default(false); // If true, a MAX_(STREAM)_DATA frame is sent immediately, after generation. Otherwise, sending the frame is delayed until the sending process starts due to another event.

		// Acknowledgments
        bool bundleAckForNonAckElicitingPackets = default(false); // If true, bundles ack for non-ack-eliciting packets in an outgoing packet.
        int numReceivedAckElicitingsBeforeAck = default(2); // Number of ack eliciting packets a receiver waits for, before it sends an ACK, immediately.
        bool useIBit = default(false); // If true, an ACK frame is sent immediately upon an ack-eliciting packet with the I-Bit set.

		// Path MTU Discovery
        int maxPmtu @unit(B) = default(65535B); // With useDplpmtud, it is used as upper limit for the PMTU search. Without useDplpmtud, the PMTU is set to the minimum of the MTU of the outgoing interface and this value.
        bool useDplpmtud = default(false); // If true, uses DPLPMTUD (RFC8899) to determine the PMTU.
        bool dplpmtudUsePtb = default(true); // If true, uses ICMP Packet Too Big (PTB) messages for DPLPMTUD.
        int dplpmtudMinPmtu = default(0); // Specifies the minimum PMTU to use (if 0, it uses 1200 B for IPv4 or 1280 B for IPv6 as minimum PMTU).
        string dplpmtudCandidateSequence = default("Up"); // Specifies the sequence of candidates used for the search algorithm of DPLPMTUD (Up, Down, OptUp, Binary, OptBinary, Jump).
        int dplpmtudMaxProbes = default(3); // Specifies the number of probe packet sent for one candidate before given up.
        int useCwndForParallelProbes = default(0); // If 0, no parallel probe packets; if 1, uses the availbale cwnd to send additional probe packets and if 2 prefers to send probe packets over other packets.
        bool skipPacketNumberForDplpmtudProbePackets = default(false); // Specifies whether to skip a packet number for a DPLPMTUD probe packet in order to avoid ack delay.
        bool sendDataDuringInitalDplpmtudBase = default(true); // If false, waits for DPLPMTUD switching initially to SEARCH before start sending application data.
        bool dplpmtudUsePmtuValidator = default (true); // If true, activates the PMTU Validator that trys to detect when the current PMTU estimation is larger than the actual PMTU.
        int pmtuValidatorLostPacketsThreshold = default(2); // Specifies the number of lost data packets before assuming a decreased PMTU.
        double pmtuValidatorTimeThreshold @unit(s) = default(-1s); // Specifies the time threshold between two lost packets (if negative, the number of seconds are used multiplied by SRTT).
        int pmtuValidatorInvalidOnPersistentCongestion = default(1); // If 0, disabled. If 1, the PMTU validator uses persistent congeation as signal for a decreased PMTU. If 2, only two consecutive persisten congestions used as signal. If 3, ...
        double reducePacketTimeThreshold @unit(s) = default(0s); // Specifies after which time with no acks, packets with reduced size are sent (if negative, the number of seconds are used multiplied by PTO).

        @display("i=block/transport");
        @signal[cwnd*];
        @statisticTemplate[cwnd](record=vector);
        @signal[bytesInFlight*];
        @statisticTemplate[bytesInFlight](record=vector);
        @signal[totalReceivedDataBytes*];
        @statisticTemplate[totalReceivedDataBytes](record=vector);
        @signal[availableRwnd*];
        @statisticTemplate[availableRwnd](record=vector);
        @signal[sendBufferUnsentData*];
        @statisticTemplate[sendBufferUnsentData](record=vector);
        @signal[latestRtt*];
        @statisticTemplate[latestRtt](record=vector);
        @signal[partialBytesAcked*];
        @statisticTemplate[partialBytesAcked](record=vector);
        @signal[persistentCongestionPeriod*];
        @statisticTemplate[persistentCongestionPeriod](record=vector);

        // Stream Statistic
        @signal[streamRcvDataBytes*]; 
        @statisticTemplate[streamRcvDataBytes](record=vector);  
        @signal[streamTotalRcvDataBytes*]; 
        @statisticTemplate[streamTotalRcvDataBytes](record=vector); 
        
        // Flow Control Statistic
        @signal[availableRwnd*];
        @statisticTemplate[availableRwnd](record=vector);  
        
        // Statistic of Flow Controller     
        @signal[generatedBlockFrameCount*];
        @statisticTemplate[generatedBlockFrameCount](record=last);   
        @signal[blockFrameOffset*];
        @statisticTemplate[blockFrameOffset](record=vector);     
        @signal[rcvMaxFrameCount*];
        @statisticTemplate[rcvMaxFrameCount](record=last);
        
        // Statistic of Flow Control Responder
        @signal[rcvBlockFrameCount*];
        @statisticTemplate[rcvBlockFrameCount](record=last);
		@signal[generatedMaxDataFrameCount*];
        @statisticTemplate[generatedMaxDataFrameCount](record=last); 
        @signal[maxDataFrameOffset*];
        @statisticTemplate[maxDataFrameOffset](record=vector);
        @signal[consumedData*];
        @statisticTemplate[consumedData](record=vector);
        @signal[retransmitFCUpdateCount*];
        @statisticTemplate[retransmitFCUpdateCount](record=last);

        //Statistic of StreamReceivedQueue
        @signal[rcvStreamFrameStartOffset*];
        @statisticTemplate[rcvStreamFrameStartOffset](record=vector);
        @signal[rcvStreamFrameEndOffset*];
        @statisticTemplate[rcvStreamFrameEndOffset](record=vector);
        
        @signal[packetReceived](type=cPacket);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @signal[packetSent](type=cPacket);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

        @signal[dplpmtudSearchPmtu*];
        @statisticTemplate[dplpmtudSearchPmtu](record=vector);
        @signal[dplpmtudSearchNumberOfTests*];
        @statisticTemplate[dplpmtudSearchNumberOfTests](record=last);
        @signal[dplpmtudSearchTime*];
        @statisticTemplate[dplpmtudSearchTime](record=last);
        @signal[dplpmtudSearchNetworkLoad*];
        @statisticTemplate[dplpmtudSearchNetworkLoad](record=last);
        @signal[dplpmtudInvalidSignals*];
        @statisticTemplate[dplpmtudInvalidSignals](record=vector);
        @signal[dplpmtudNumOfProbesBeforeAck*];
        @statisticTemplate[dplpmtudNumOfProbesBeforeAck](record=vector);
        @signal[pmtuValidatorNumberOfLostPackets*];
        @statisticTemplate[pmtuValidatorNumberOfLostPackets](record=vector);
        @signal[pmtuValidatorTimeBetweenLostPackets*];
        @statisticTemplate[pmtuValidatorTimeBetweenLostPackets](record=vector);
        @signal[pmtuValidatorPersistentCongestions*];
        @statisticTemplate[pmtuValidatorPersistentCongestions](record=vector);
        
        @signal[usedMaxQuicPacketSize*];
        @statisticTemplate[usedMaxQuicPacketSize](record=vector);

        @signal[packetNumberReceived*];
        @statisticTemplate[packetNumberReceived](record=vector);
        @signal[packetNumberSent*];
        @statisticTemplate[packetNumberSent](record=vector);

    gates:
        input appIn @labels(QuicCommand/down) @messageKinds(inet::QuicCommandCode);
        input udpIn @labels(QuicHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(QuicCommand/up) @messageKinds(inet::QuicStatusInd);
        output udpOut @labels(QuicHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}
