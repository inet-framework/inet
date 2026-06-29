//
// Protocol Test Framework for INET -- sample test programs.
//
// These exercise the matching engine and injector against the two-host demo network.
// They are registered by name; select one via the ProtocolTester's `testName` parameter.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ProtocolTest.h"

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/DirectionTag_m.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPlca.h"

namespace inet {
namespace protocoltest {

using inet::tcp::TcpHeader;
using inet::physicallayer::EthernetPlca;

// host1 sends a UDP datagram to port 5000; host2 receives it. Should PASS.
Define_ProtocolTest(udp_basic_pass)
{
    return ProtocolTest("udp_basic_pass")
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("udp.destPort == 5000").within(0.2))
        .once(on("host2").signal("packetReceivedFromLower").layer(Layer::Transport)
                  .packet("udp.destPort == 5000").within(0.1));
}

// Expects a datagram to a port nobody uses -- the deadline is missed. Should FAIL.
Define_ProtocolTest(udp_basic_fail)
{
    return ProtocolTest("udp_basic_fail")
        .once(on("host1").signal("packetSentToLower")
                  .packet("udp.destPort == 9999").within(0.3));
}

// TCP three-way handshake, observed from the initiator (host1) at the transport layer:
// host1 sends the SYN, receives the SYN+ACK, and sends the final ACK, with the ack
// numbers following seq+1 at each step. {name} in a match expression is the value
// captured by an earlier step.
Define_ProtocolTest(tcp_handshake_seq)
{
    return ProtocolTest("tcp_handshake_seq")
        // host1 sends the SYN; remember its initial sequence number.
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.synBit == true && tcp.ackBit == false")
                  .describe("host1's SYN")
                  .capture("isn", "tcp.sequenceNo")
                  .within(0.2))
        // host1 receives the SYN+ACK acknowledging isn+1; remember the peer's ISN.
        .once(on("host1").signal("packetReceivedFromLower").layer(Layer::Transport)
                  .packet("tcp.synBit == true && tcp.ackBit == true && tcp.ackNo == {isn} + 1")
                  .describe("the SYN+ACK acknowledging host1's ISN+1")
                  .capture("peerIsn", "tcp.sequenceNo")
                  .within(0.5))
        // host1 sends the final ACK acknowledging the peer's ISN+1.
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.ackBit == true && tcp.synBit == false && tcp.ackNo == {peerIsn} + 1")
                  .describe("host1's final ACK acknowledging the peer's ISN+1")
                  .within(0.5));
}

// Should FAIL: asserts a wrong ack relationship (isn+2) for the SYN+ACK host1 receives.
Define_ProtocolTest(tcp_handshake_seq_bad)
{
    return ProtocolTest("tcp_handshake_seq_bad")
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.synBit == true && tcp.ackBit == false")
                  .capture("isn", "tcp.sequenceNo")
                  .within(0.2))
        .once(on("host1").signal("packetReceivedFromLower").layer(Layer::Transport)
                  .packet("tcp.synBit == true && tcp.ackBit == true && tcp.ackNo == {isn} + 2") // wrong
                  .within(0.5));
}

// Phase 6 fault injection: the test program drives a PacketTap (spliced on the wire) to
// drop host1's first TCP data segment, so host1's TCP retransmits the same segment (same
// sequence number) after the retransmission timeout.
Define_ProtocolTest(tcp_retransmit)
{
    return ProtocolTest("tcp_retransmit")
        // drop the first data-bearing segment to host2's port (not the bare SYN/ACK frames).
        .intercept(intercept("tap")
                     .match("tcp.destPort == 1000 && tcp.synBit == false").minBytes(100).nth(1)
                     .drop().describe("the first data segment"))
        // host1 sends the data segment after the handshake; remember its sequence number.
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.destPort == 1000 && tcp.synBit == false")
                  .after(0.15)
                  .capture("dataSeq", "tcp.sequenceNo")
                  .describe("the data segment")
                  .within(0.3))
        // the tap drops it on the wire, so host1 retransmits the same segment (same
        // sequence number) once the retransmission timer fires.
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.sequenceNo == {dataSeq} && tcp.synBit == false")
                  .notBefore(0.3)
                  .describe("the retransmitted data segment (same sequence number)")
                  .within(5.0));
}

// Same outcome via the mutate action: instead of dropping, the tap corrupts the first data
// segment (sets a bit error); host2's MAC discards the corrupted frame, so host1 still
// retransmits. Demonstrates intercept(...).mutate(lambda).
Define_ProtocolTest(tcp_retransmit_mutate)
{
    return ProtocolTest("tcp_retransmit_mutate")
        .intercept(intercept("tap")
                     .match("tcp.destPort == 1000 && tcp.synBit == false").minBytes(100).nth(1)
                     .mutate([](Packet *frame) { frame->setBitError(true); })
                     .describe("the first data segment (corrupt it)"))
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.destPort == 1000 && tcp.synBit == false")
                  .after(0.15)
                  .capture("dataSeq", "tcp.sequenceNo")
                  .describe("the data segment")
                  .within(0.3))
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.sequenceNo == {dataSeq} && tcp.synBit == false")
                  .notBefore(0.3)
                  .describe("the retransmitted data segment (same sequence number)")
                  .within(5.0));
}

// Time-scheduled injection: craft an IP/UDP datagram and push it up host2's Ethernet
// interface as if it arrived from the wire (the interface's inbound seam, as INET
// emulation uses). host2 runs a UdpEchoApp, so it echoes the datagram back; the program
// asserts that host2 then emits a UDP segment to the lower layer (the echo).

static Packet *buildInjectedUdpDatagram(const CaptureStore&)
{
    auto packet = new Packet("injectedUdp");
    packet->insertAtBack(makeShared<ByteCountChunk>(B(16)));

    auto udpHeader = makeShared<UdpHeader>();
    udpHeader->setSrcPort(1234);
    udpHeader->setDestPort(5000);
    udpHeader->setTotalLengthField(B(24)); // 8B header + 16B payload
    udpHeader->setChecksum(0);
    udpHeader->setChecksumMode(CHECKSUM_DECLARED_CORRECT);
    packet->insertAtFront(udpHeader);

    auto ipv4Header = makeShared<Ipv4Header>();
    ipv4Header->setSrcAddress(Ipv4Address("10.0.0.1"));  // host1
    ipv4Header->setDestAddress(Ipv4Address("10.0.0.2")); // host2
    ipv4Header->setProtocolId(IP_PROT_UDP);
    ipv4Header->setTimeToLive(64);
    ipv4Header->setTotalLengthField(B(20) + B(24));      // IP header + UDP datagram
    ipv4Header->setChecksum(0);
    ipv4Header->setChecksumMode(CHECKSUM_DECLARED_CORRECT);
    packet->insertAtFront(ipv4Header);

    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    packet->addTag<DirectionTag>()->setDirection(DIRECTION_INBOUND);
    // Route it up to the network protocol, as a decapsulated inbound frame would be.
    auto dispatchReq = packet->addTag<DispatchProtocolReq>();
    dispatchReq->setProtocol(&Protocol::ipv4);
    dispatchReq->setServicePrimitive(SP_INDICATION);
    return packet;
}

// Should PASS: the injected datagram reaches host2's echo app, which replies.
Define_ProtocolTest(udp_inject_echo)
{
    return ProtocolTest("udp_inject_echo")
        .inject(inject("host2").into("eth[0]", "upperLayerOut").at(0.5)
                  .describe("a UDP datagram to port 5000").packet(buildInjectedUdpDatagram))
        // The inject is an ordered step, so this expect is anchored at the injection time.
        .once(on("host2").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("udp.srcPort == 5000").within(0.2));
}

// Reactive injection (stimulus/response): the test is the sole TCP peer. host1 actively
// opens a connection to a phantom IP (10.0.0.99, owned by nobody), so no real peer
// competes. The test observes host1's SYN, reactively injects a crafted SYN+ACK that
// acknowledges host1's ISN+1, then observes host1's final ACK -- a full three-way
// handshake driven by injection.

static Packet *buildSynAck(const CaptureStore& captures)
{
    intval_t isn = captures.at("isn").intValue();
    intval_t clientPort = captures.at("clientPort").intValue();

    auto packet = new Packet("injSYNACK");
    auto tcpHeader = makeShared<TcpHeader>();
    tcpHeader->setSrcPort(1000);            // the peer's listening port (= connectPort)
    tcpHeader->setDestPort(clientPort);     // host1's ephemeral port (captured)
    tcpHeader->setSynBit(true);
    tcpHeader->setAckBit(true);
    tcpHeader->setSequenceNo(5000);         // the peer's ISN (chosen by the test)
    tcpHeader->setAckNo(isn + 1);           // acknowledge host1's SYN
    tcpHeader->setWindow(16384);
    tcpHeader->setHeaderLength(B(20));
    tcpHeader->setChecksum(0);
    tcpHeader->setChecksumMode(CHECKSUM_DECLARED_CORRECT);
    packet->insertAtFront(tcpHeader);

    auto ipv4Header = makeShared<Ipv4Header>();
    ipv4Header->setSrcAddress(Ipv4Address("10.0.0.99")); // phantom peer
    ipv4Header->setDestAddress(Ipv4Address("10.0.0.1")); // host1
    ipv4Header->setProtocolId(IP_PROT_TCP);
    ipv4Header->setTimeToLive(64);
    ipv4Header->setTotalLengthField(B(20) + B(20));
    ipv4Header->setChecksum(0);
    ipv4Header->setChecksumMode(CHECKSUM_DECLARED_CORRECT);
    packet->insertAtFront(ipv4Header);

    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    packet->addTag<DirectionTag>()->setDirection(DIRECTION_INBOUND);
    auto dispatchReq = packet->addTag<DispatchProtocolReq>();
    dispatchReq->setProtocol(&Protocol::ipv4);
    dispatchReq->setServicePrimitive(SP_INDICATION);
    return packet;
}

Define_ProtocolTest(tcp_handshake_peer)
{
    return ProtocolTest("tcp_handshake_peer")
        // 1. host1's outgoing SYN; capture its ISN and ephemeral source port.
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.synBit == true && tcp.ackBit == false")
                  .describe("host1's SYN (SYN set, ACK clear)")
                  .capture("isn", "tcp.sequenceNo")
                  .capture("clientPort", "tcp.srcPort")
                  .within(1.0))
        // 2. Reactively inject a SYN+ACK acknowledging host1's ISN+1.
        .inject(inject("host1").into("eth[0]", "upperLayerOut").after(0.001)
                  .describe("a SYN+ACK acknowledging host1's ISN+1").packet(buildSynAck))
        // 3. host1's final ACK, acknowledging the peer's ISN+1 (5000+1).
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("tcp.ackBit == true && tcp.synBit == false && tcp.ackNo == 5001")
                  .describe("host1's final ACK (acknowledging the peer's ISN+1)")
                  .within(1.0));
}

// Combinators: host1 runs two UDP flows (to ports 5000 and 5001). Assert that both
// datagrams leave in any order (unordered), then that nothing goes to port 9999 for a
// while (negative). Should PASS.
Define_ProtocolTest(multi_flow)
{
    return ProtocolTest("multi_flow")
        .unordered({
            on("host1").signal("packetSentToLower").layer(Layer::Transport)
              .packet("udp.destPort == 5000").describe("a datagram to port 5000").within(1.0),
            on("host1").signal("packetSentToLower").layer(Layer::Transport)
              .packet("udp.destPort == 5001").describe("a datagram to port 5001").within(1.0)
        })
        .never(on("host1").signal("packetSentToLower")
                    .packet("udp.destPort == 9999").describe("any datagram to port 9999").within(0.3));
}

// Should FAIL: the forbidden datagram (to port 5000) does occur within the window.
Define_ProtocolTest(multi_flow_bad)
{
    return ProtocolTest("multi_flow_bad")
        .never(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                    .packet("udp.destPort == 5000").describe("any datagram to port 5000").within(0.3));
}

// Should PASS: the optional datagram (to port 9999) never occurs and is skipped, then
// the required datagram (to port 5000) is matched.
Define_ProtocolTest(optional_skip)
{
    return ProtocolTest("optional_skip")
        .atMostOnce(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                    .packet("udp.destPort == 9999").describe("a datagram to port 9999").within(0.3))
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("udp.destPort == 5000").describe("a datagram to port 5000").within(2.0));
}

// delivery: the datagram host1 sends to port 5000 is received by host2 -- the same
// packet (correlated by treeId) -- within 0.1s. Should PASS.
Define_ProtocolTest(udp_delivery)
{
    return ProtocolTest("udp_delivery")
        .delivery(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                    .packet("udp.destPort == 5000").describe("a datagram to port 5000"),
                  on("host2").signal("packetReceivedFromLower").layer(Layer::Transport)
                    .packet("udp.destPort == 5000"),
                  0.1);
}

// anyOf: host1 sends to port 5000 or 9999 -- the 5000 alternative matches. Should PASS.
Define_ProtocolTest(udp_anyof)
{
    return ProtocolTest("udp_anyof")
        .anyOf({
            on("host1").signal("packetSentToLower").layer(Layer::Transport)
              .packet("udp.destPort == 9999").describe("a datagram to port 9999").within(1.0),
            on("host1").signal("packetSentToLower").layer(Layer::Transport)
              .packet("udp.destPort == 5000").describe("a datagram to port 5000").within(1.0)
        });
}

// exactlyTimes: host1 sends three datagrams to port 5000 within 2s. Should PASS.
Define_ProtocolTest(udp_repeat)
{
    return ProtocolTest("udp_repeat")
        .exactlyTimes(3, on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("udp.destPort == 5000").describe("a datagram to port 5000").within(2.0));
}

// greedy count (lower bound): host1 sends a datagram to port 5000 every 0.2s (5 within
// the window). The greedy window consumes them all; "at least 3" is satisfied. PASS.
Define_ProtocolTest(udp_count_pass)
{
    return ProtocolTest("udp_count_pass")
        .atLeastTimes(3, on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("udp.destPort == 5000").describe("a datagram to port 5000").within(0.95));
}

// greedy count (upper bound): at most 2 such datagrams are allowed, but host1 sends 5 --
// the third match overflows the window's upper bound. Should FAIL.
Define_ProtocolTest(udp_count_overflow)
{
    return ProtocolTest("udp_count_overflow")
        .atMostTimes(2, on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("udp.destPort == 5000").describe("a datagram to port 5000").within(0.95));
}

// strict: closed-world. host1 sends to port 5000 at the transport layer, but this strict
// expect wants port 9999 there -- the in-scope-but-wrong packet fails. Should FAIL.
Define_ProtocolTest(udp_strict_bad)
{
    return ProtocolTest("udp_strict_bad")
        .strict()
        .once(on("host1").signal("packetSentToLower").layer(Layer::Transport)
                  .packet("udp.destPort == 9999").describe("a datagram to port 9999").within(1.0));
}

// Cookbook: ARP resolution. Before host1 can send to host2 it resolves host2's MAC --
// host1 broadcasts an ARP request and receives host2's reply (opcode 1 = request, 2 = reply).
Define_ProtocolTest(arp_resolution)
{
    return ProtocolTest("arp_resolution")
        .once(on("host1").signal("packetSentToLower").layer(Layer::Link)
                  .packet("arp.opcode == 1").describe("an ARP request").within(0.2))
        .once(on("host1").signal("packetReceivedFromLower").layer(Layer::Link)
                  .packet("arp.opcode == 2").describe("host2's ARP reply").within(0.2));
}

// Cookbook: IPv4 fragmentation. A 4000-byte datagram exceeds the 1500-byte Ethernet MTU,
// so host1 emits several IPv4 fragments -- non-final ones carry the more-fragments flag,
// and later fragments sit at a non-zero offset.
Define_ProtocolTest(ipv4_fragmentation)
{
    return ProtocolTest("ipv4_fragmentation")
        .once(on("host1").signal("packetSentToLower").layer(Layer::Link)
                  .packet("ipv4.moreFragments == true")
                  .describe("an IPv4 fragment with the more-fragments flag set").within(0.2))
        .once(on("host1").signal("packetSentToLower").layer(Layer::Link)
                  .packet("ipv4.fragmentOffset > 0")
                  .describe("a later fragment at a non-zero offset").within(0.2));
}

// WiFi 802.11 frame-sequence tests. Frame types use the Ieee80211 MAC header `type`
// field: ST_ACTION=13 (ADDBA action frames), ST_BLOCKACK_REQ=24, ST_BLOCKACK=25,
// ST_DATA_WITH_QOS=40. (A-MSDU aggregation and Block Ack are exercised by separate
// scenarios: with A-MSDU each aggregate is a single MPDU acked normally, whereas the
// Block Ack agreement is used on the non-aggregated multi-MPDU path.)

// Block Ack establishment + use: host1 sets up a Block Ack agreement with host2 (ADDBA
// request/response) and then runs a Block-Ack-Request / Block-Ack exchange.
Define_ProtocolTest(wifi_block_ack)
{
    return ProtocolTest("wifi_block_ack")
        // ADDBA setup: host1 sends the ADDBA request, then receives the ADDBA response.
        .once(on("host1").signal("packetSentToLower").layer(Layer::Link)
                  .packet("ieee80211mac.type == 13").describe("an ADDBA request (action frame)").within(0.1))
        .once(on("host1").signal("packetReceivedFromLower").layer(Layer::Link)
                  .packet("ieee80211mac.type == 13").describe("the ADDBA response").within(0.5))
        // Block Ack in use: host1 sends a Block Ack Request and receives the Block Ack.
        .once(on("host1").signal("packetSentToLower").layer(Layer::Link)
                  .packet("ieee80211mac.type == 24").describe("a Block Ack Request").within(0.5))
        .once(on("host1").signal("packetReceivedFromLower").layer(Layer::Link)
                  .packet("ieee80211mac.type == 25").describe("the Block Ack").within(0.5));
}

// A-MSDU aggregation: host1 sends a QoS data frame carrying an aggregated A-MSDU.
Define_ProtocolTest(wifi_aggregation)
{
    return ProtocolTest("wifi_aggregation")
        .once(on("host1").signal("packetSentToLower").layer(Layer::Link)
                  .packet("ieee80211mac.type == 40 && Ieee80211DataHeader.aMsduPresent == true")
                  .describe("an A-MSDU aggregated QoS data frame").within(0.2));
}

// The full IEEE 802.11 Block Ack sequence from the originator (host1), including the
// immediate ACKs, exactly as the standard describes setting up and using the agreement:
//   ADDBA Request -> ACK -> ADDBA Response -> ACK -> (block-acked QoS data) ->
//   Block Ack Request -> Block Ack.
// Frame types: ST_ACTION=13, ST_ACK=29, ST_DATA_WITH_QOS=40, ST_BLOCKACK_REQ=24,
// ST_BLOCKACK=25; QoS-data ackPolicy BLOCK_ACK=3.
Define_ProtocolTest(wifi_block_ack_full)
{
    // host1's MAC-layer transmit / receive of an 802.11 frame matching `expr`.
    auto send = [](const char *expr, const char *desc, double w) {
        return on("host1").signal("packetSentToLower").layer(Layer::Link).match(expr).describe(desc).within(w);
    };
    auto receive = [](const char *expr, const char *desc, double w) {
        return on("host1").signal("packetReceivedFromLower").layer(Layer::Link).match(expr).describe(desc).within(w);
    };

    return ProtocolTest("wifi_block_ack_full")
        // --- Create the Block Ack agreement (ADDBA handshake, each frame ACKed) ---
        .once(send   ("ieee80211mac.type == 13", "ADDBA Request", 0.1))
        .once(receive("ieee80211mac.type == 29", "ACK (recipient acknowledges the ADDBA Request)", 0.002))
        .once(receive("ieee80211mac.type == 13", "ADDBA Response", 0.01))
        .once(send   ("ieee80211mac.type == 29", "ACK (originator acknowledges the ADDBA Response)", 0.002))
        // --- Use the agreement: a block of 5 QoS data frames (Block Ack policy, no
        //     immediate ACK), then one Block Ack Request, then a single Block Ack
        //     covering the whole block. ---
        .exactlyTimes(5, send("ieee80211mac.type == 40 && Ieee80211DataHeader.ackPolicy == 3",
                         "a QoS data frame with Block Ack policy", 0.5))
        .once(send   ("ieee80211mac.type == 24", "a Block Ack Request", 0.5))
        .once(receive("ieee80211mac.type == 25", "a Block Ack covering the block", 0.01));
}

// Phase 9 -- state-machine observation (Ethernet PLCA, 10BASE-T1S). PLCA's behaviour
// (beacons, transmit opportunities, the cycle) lives in two state machines, not in the
// packet trace, so the test observes the FSM state-change signals directly. EthernetPlca
// wires controlFsm/dataFsm to emit their state index on every transition (controlStateChanged
// / dataStateChanged), and also emits curID (the transmit-opportunity owner) and txCmd/rxCmd.
// The state values are the public EthernetPlca::ControlState / DataState enums.
//
// This asserts one full beacon -> sync -> transmit-opportunity -> node-transmit handover on a
// controller + 2-node multidrop bus. node[0] has a frame to send (EthernetSourceApp), so it
// uses its transmit opportunity; the sink-only controller does not.
Define_ProtocolTest(plca_beacon_cycle)
{
    return ProtocolTest("plca_beacon_cycle")
        // 1. The controller (local_nodeID 0) starts a PLCA cycle by sending the BEACON.
        .once(on("controller.eth[0].plca").signal("controlStateChanged")
                   .is(EthernetPlca::CS_SEND_BEACON).describe("the controller sends a BEACON").within(0.001))
        // 2. The controller signals BEACON to the PHY while doing so.
        .once(on("controller.eth[0].plca").signal("txCmd")
                   .is(EthernetPlca::CMD_BEACON).describe("the controller's TX command is BEACON").within(0.001))
        // 3. node[0] detects the beacon and synchronises its cycle to it.
        .once(on("node[0].eth[0].plca").signal("controlStateChanged")
                   .is(EthernetPlca::CS_SYNCING).describe("node[0] synchronises to the beacon").within(0.001))
        // 4. The transmit opportunity rotates from the controller (curID 0) to node[0] (curID 1).
        .once(on("node[0].eth[0].plca").signal("curID")
                   .is(1).describe("the transmit opportunity reaches node[0]").within(0.001))
        // 5. node[0] has a pending frame, so it COMMITs to transmit in its opportunity.
        .once(on("node[0].eth[0].plca").signal("controlStateChanged")
                   .is(EthernetPlca::CS_COMMIT).describe("node[0] commits to transmit in its opportunity").within(0.001))
        // 6. node[0]'s data state machine transmits the frame.
        .once(on("node[0].eth[0].plca").signal("dataStateChanged")
                   .is(EthernetPlca::DS_TRANSMIT).describe("node[0]'s data FSM transmits the frame").within(0.001))
        // 7. The controller receives node[0]'s frame in turn (its data FSM enters RECEIVE).
        .once(on("controller.eth[0].plca").signal("dataStateChanged")
                   .is(EthernetPlca::DS_RECEIVE).describe("the controller receives node[0]'s frame").within(0.001))
        // 8. Throughout, the control FSM must never enter its ABORT state.
        .never(on("node[0].eth[0].plca").signal("controlStateChanged")
                   .is(EthernetPlca::CS_ABORT).describe("node[0]'s control FSM must not abort").within(0.001));
}

// Should FAIL: the controller runs only a sink app, so it never has a frame to send and its
// control FSM never enters the TRANSMIT state -- the deadline on this state is missed.
Define_ProtocolTest(plca_beacon_cycle_bad)
{
    return ProtocolTest("plca_beacon_cycle_bad")
        .once(on("controller.eth[0].plca").signal("controlStateChanged")
                   .is(EthernetPlca::CS_SEND_BEACON).describe("the controller sends a BEACON").within(0.001))
        .once(on("controller.eth[0].plca").signal("controlStateChanged")
                   .is(EthernetPlca::CS_TRANSMIT).describe("the controller transmits (it never does)").within(0.001));
}

// Mobile IPv6 (RFC 6275) home registration + route optimization -- new orthogonal vocabulary.
// MIPv6 is a message-exchange protocol; this asserts the Mobility Header sequence. The mipv6
// module emits no packet signals, so the messages are observed at the IPv6 boundary by their
// `mobileipv6` protocol tag (where they are still the bare Mobility Header, so PacketFilter can
// read the chunk fields), and attributed to the mipv6 module's point of view for the narration:
// a message mipv6 *sends* is the IPv6 layer's `packetReceivedFromUpper`; one it *receives* is
// `packetSentToUpper`.
Define_ProtocolTest(mipv6_registration_and_ro)
{
    auto sends = [](const char *expr, double w) {
        return on("MN[0]").signal("packetReceivedFromUpper").protocol("mobileipv6")
                 .attributeTo("MN[0].ipv6.mipv6").packet(expr).within(w);
    };
    auto receives = [](const char *expr, double w) {
        return on("MN[0]").signal("packetSentToUpper").protocol("mobileipv6")
                 .attributeTo("MN[0].ipv6.mipv6").packet(expr).within(w);
    };

    return ProtocolTest("mipv6_registration_and_ro")
        // --- Home registration with the Home Agent ---
        .once(sends("BindingUpdate.homeRegistrationFlag == true && BindingUpdate.ackFlag == true", 72))
        .once(receives("BindingAcknowledgement.status == 0", 6))
        // --- Return routability: probe both paths (any order), remembering each init cookie ---
        .unordered({
            sends("HomeTestInit.homeInitCookie >= 0", 7).capture("hoCookie", "HomeTestInit.homeInitCookie"),
            sends("CareOfTestInit.careOfInitCookie >= 0", 7).capture("coCookie", "CareOfTestInit.careOfInitCookie")
        })
        // --- The CN's replies must echo those cookies (return-routability correctness) ---
        .unordered({
            receives("HomeTest.homeInitCookie == {hoCookie}", 4),
            receives("CareOfTest.careOfInitCookie == {coCookie}", 4)
        })
        // --- Route optimization: a non-home Binding Update directly to the CN ---
        .once(sends("BindingUpdate.homeRegistrationFlag == false", 5));
}

// Should FAIL: the Home Agent accepts the registration, so the MN never receives a Binding
// Acknowledgement reporting INSUFFICIENT_RESOURCES (status 130) -- the deadline is missed.
Define_ProtocolTest(mipv6_registration_and_ro_bad)
{
    return ProtocolTest("mipv6_registration_and_ro_bad")
        .once(on("MN[0]").signal("packetReceivedFromUpper").protocol("mobileipv6")
                  .attributeTo("MN[0].ipv6.mipv6")
                  .packet("BindingUpdate.homeRegistrationFlag == true").within(72))
        .once(on("MN[0]").signal("packetSentToUpper").protocol("mobileipv6")
                  .attributeTo("MN[0].ipv6.mipv6")
                  .packet("BindingAcknowledgement.status == 130").within(6)); // INSUFFICIENT_RESOURCES -- never happens
}

} // namespace protocoltest
} // namespace inet
