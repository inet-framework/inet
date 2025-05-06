//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.quic;

import inet.transportlayer.contract.IQuic;

//
// Implements the QUIC protocol. For other implementations, see ~IQuic.
//
// This module uses ~Udp, which is compatible with both ~Ipv4 and ~Ipv6.
//
// QUIC packet headers are represented by the classes in PacketHeader.msg,
// QUIC frame headers are represented by the classes in  FrameHeader.msg.
//
// <b>Usage</b>
//
// In order to use QUIC for data transfer, a module (app module) can
// communicate with the QUIC module by ~QuicCommand messages. Using the
// ~QuicSocket class is the user-friendly way of doing so.
//
// <b>Sending</b>
//
// With the send method of the ~QuicSocket class, an app module can provide
// data to send for QUIC, more specific for a QUIC stream. For all streams,
// QUIC sends data as fast as possible. That is, as fast as the congestion and
// flow control permits. As long as the destination endpoint has not
// acknowledged their receipt, QUIC stores the data to send in its send queue
// for the stream. The parameter sendQueueLimit defines a limit for the data
// over all stream send queues.
//
// If the size of the data in the stream send queues reaches the limit, QUIC
// signals to the app module that it must stop providing data. If the queue
// drains (i.e., if the size of data in the stream send queues is less than
// sendQueueLowWaterRatio * sendQueueLimit), QUIC signals to the app module
// that it can provide more data.
//
// <b>Receiving</b>
//
// When receiving data, QUIC stores the data in the receive queue of the
// corresponding stream. If data is available to read for the app module,
// QUIC notifies the app module, which can call the recv method of the
// ~QuicSocket class to receive the available data from QUIC. QUIC then takes
// the data out of the stream receive queue and sends it to the app module.
//
// The flow control limits the size of data in stream receive queues.
//
// <b>Reliability</b>
//
// QUIC ensures that the provided data will arrive at the receiver. When a
// packet arrives, QUIC acknowledges (acks) the receipt to the sender. Packets
// that contain information other than acks are ack-eliciting. The parameter
// numReceivedAckElicitingsBeforeAck defines after how many received
// ack-eliciting packets QUIC sends an ack immediately. When receiving less,
// QUIC waits 25 ms before sending the ack.
//
// Packets that contain only ack information are not ack-eliciting. By default,
// QUIC acks these packets only with the next ack it must send due to the
// receipt of other packets that are ack-eliciting. With the parameter
// bundleAckForNonAckElicitingPackets set to true, QUIC bundles acks for non-
// ack-eliciting packets into packets it has to send due to another reason
// (e.g., to transfer data).
//
// <b>Congestion Control</b>
//
// QUIC is designed to work with different congestion control algorithms.
// The parameter congestionControl defines the algorithm to use. By default,
// it uses the NewReno algorithm. Setting this parameter to "no" disables the
// congestion control.
//
// The QUIC standard specifies a NewReno congestion control that uses an
// approximate congestion window (cwnd) increase in the congestion avoidance
// phase. By default, this QUIC module uses a more accurate way of increasing
// the cwnd during congestion avoidance. The parameter
// accurateIncreaseInNewRenoCongestionAvoidance allows changing that.
//
// <b>Flow Control</b>
//
// QUIC has a flow control for each stream and for the whole connection. The
// flow control restricts the size of data the peer is allowed to send. The
// parameters initialMaxData and initialMaxStreamData enable to set the initial
// limit for the whole connection and per stream, respectively.
//
// QUIC stores the received data in its stream receive queues. It takes data
// out and frees the space if the app module reads that data. If the app module
// reads enough data, QUIC will increase the flow control limit and inform its
// peer by sending a MAX_DATA or MAX_STREAM_DATA frame. The parameter
// maxStreamDataFrameThreshold defines how many data the app module has to read
// from a stream before QUIC sends a MAX_STREAM_DATA frame. The parameter
// defines how many data the app module has to read over all streams before
// QUIC sends a MAX_DATA frame.
//
// By default, after QUIC decides to send a MAX_(STREAM_)DATA frame, it sends
// it when appropriate. With the parameter sendMaxDataFramesImmediately set to
// true, QUIC sends a MAX_(STREAM_)DATA frame immediately if after it decided
// to send one.
//
// <b>Path MTU Discovery</b>
//
// QUIC can use a Path MTU (PMTU) Discovery that determines the PMTU by sending
// probe packets. The parameter useDplpmtud set to true enables the discovery.
// When enabled, QUIC probes PMTU candidates between a Min PMTU (set by the
// parameter dplpmtudMinPmtu) and Max PMTU (the lesser of the local link MTU
// and the parameter maxPmtu). The parameter dplpmtudCandidateSequence defines
// the sequence in which PMTU candidates are probed.
//
// Receiving an acknowledgment for a PMTU probe packet, shows that the path
// supports the size of that probe packet. If QUIC declares a PMTU probe packet
// as lost, the size might was too large for the path. The parameter defines
// how often a probe packet should be resent before assuming that the PMTU
// is smaller than the probe packet. If the parameter dplpmtudUsePtb is set
// to true, a received ICMP Packet Too Big (PTB) messages is used as a signal
// that the PMTU is smaller than the size of the packet that triggered the
// message.
//
// If the parameter dplpmtudUsePmtuValidator is set to true, QUIC uses a PMTU
// Validator to check if the determined PMTU is still valid. That is, if the
// determined PMTU is not larger than the actual PMTU. In absent of a PTB
// message, the PMTU Validator checks for a specific packet loss pattern.
// After a number of qualified lost packets (defined by the parameter
// pmtuValidatorLostPacketsThreshold) with a some distance between the send
// times (defined by the parameter pmtuValidatorTimeThreshold) the PMTU
// Validator assumes that the determined PMTU is too large. To trigger acks
// from the peer in such a case, QUIC reduces the packet size of outgoing
// packets after some time without an receiving a ack (defined by the
// parameter reducePacketTimeThreshold).
//
// <b>Standards and Deviations</b>
//
// Implementation is based on the following RFCs:
//  - RFC 5681 - TCP Congestion Control
//  - RFC 8899 - Packetization Layer Path MTU Discovery for Datagram Transports
//  - RFC 9000 - QUIC: A UDP-Based Multiplexed and Secure Transport
//  - RFC 9002 - QUIC Loss Detection and Congestion Control
//
// Missing bits:
//  - Encryption: QUIC packets are encrypted (RFC 9001). This module does use
//    encryption. All packets are sent unencrypted. Overhead by encryption is
//    not considered.
//  - Pacing: QUIC should pace outgoing packets (RFC 9002). This module bursts
//    as many packets out at once as possible.
//  - ECN: QUIC should use Explicit Congestion Notification (ECN). This module
//    does not set or read ECN.
//  - Connection Migration: A QUIC server can handle the case where a QUIC
//    client changes its source IP address or port number without losing the
//    connection.
//  - Path Validation: QUIC can send PATH_CHALLENGE frame to validate if its
//    peer, which should respond with a PATH_RESPONSE frame, is still in
//    possession of its address. This module does not validate the path.
//  - Amplification Attack Mitigation: A QUIC server ensures that it cannot be
//    used for a traffic amplification attack by restricting the size of
//    outgoing packets on a connection initiation packet from a client. This
//    module does not apply that restriction.
//  - Client Address Validation: A QUIC server can request address validation
//    by sending a Retry packet containing a token. The client must repeat
//    this token to validate its address. This module does not validate the
//    client's address. It does not use a Retry packet.
//  - Stream types: QUIC distinguish streams depending on who opened the stream
//    (client or server) and whether the stream is unidirectional or
//    bidirectional. This module does not distinguish that. All streams can be
//    used bidirectional.
//  - Stream types and Flow Control: QUIC has three transport parameters for
//    flow control limits of different stream types. One for unidirectional
//    streams and two for bidirectional streams (opened locally or by the
//    peer). This module has only one parameter for the stream flow control
//    limit that applies for all streams.
//  - Stream termination: QUIC can end the stream (clean termination) or reset
//    the stream (abrupt termination). This module does not terminate streams.
//    They remain open until connection termination.
//  - 0RTT connection setup: A QUIC server can provide the client a token
//    which allows the client to initiate a follow-up connection with zero RTT.
//    This means, the client can send data immediately when adding the token
//    in a follow-up connection setup. This module does not generate token
//    and does not support 0RTT connection setup.
//  - Exchange of Connection IDs: QUIC endpoints can add new connection IDs
//    retire old connection IDS during a connection. This module always uses
//    the fixed connection ID 0. It does not allow adding or retiring
//    connection IDs.
//  - Version Negotiation: QUIC contains a mechanism to negotiate the QUIC
//    version to use. This module implements QUIC version 1. It does not
//    negotiate a version.
//
// The paper 'A QUIC Simulation Model for INET and its Application to the
// Acknowledgment Ratio Issue' describes this QUIC implementation in some
// detail.
// https://ieeexplore.ieee.org/document/9142723 or
// https://www.hb.fh-muenster.de/opus4/frontdoor/index/index/docId/14966
//
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
        bool dplpmtudUsePmtuValidator = default(true); // If true, activates the PMTU Validator that trys to detect when the current PMTU estimation is larger than the actual PMTU.
        int pmtuValidatorLostPacketsThreshold = default(2); // Specifies the number of lost data packets before assuming a decreased PMTU.
        double pmtuValidatorTimeThreshold @unit(s) = default(-1s); // Specifies the time threshold between two lost packets. If negative, pmtuValidatorSrttFactorThreshold is used instead.
        int pmtuValidatorSrttFactorThreshold = default(1); // If pmtuValidatorTimeThreshold is negative, this number multiplied by SRTT is used instead.
        int pmtuValidatorInvalidOnPersistentCongestion = default(1); // If 0, disabled. If 1, the PMTU validator uses persistent congeation as signal for a decreased PMTU. If 2, only two consecutive persisten congestions used as signal. If 3, ...
        double reducePacketTimeThreshold @unit(s) = default(0s); // Specifies after which time with no acks, packets with reduced size are sent. If zero, packet size reduction is disabled. If negative, reducePacketPtoFactorThreshold is used instead.
        int reducePacketPtoFactorThreshold = default(3); // If reducePacketTimeThreshold is negative, this number multiplied by PTO is used instead.

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
