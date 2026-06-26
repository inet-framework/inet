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

namespace inet {
namespace protocoltest {

using inet::tcp::TcpHeader;

// host1 sends a UDP datagram to port 5000; host2 receives it. Should PASS.
Define_ProtocolTest(udp_basic_pass)
{
    return ProtocolTest("udp_basic_pass")
        .expect(on("host1").sentToLower().layer(Layer::Transport)
                  .match("udp.destPort == 5000").within(0.2))
        .expect(on("host2").receivedFromLower().layer(Layer::Transport)
                  .match("udp.destPort == 5000").within(0.1));
}

// Expects a datagram to a port nobody uses -- the deadline is missed. Should FAIL.
Define_ProtocolTest(udp_basic_fail)
{
    return ProtocolTest("udp_basic_fail")
        .expect(on("host1").sentToLower()
                  .match("udp.destPort == 9999").within(0.3));
}

// TCP three-way handshake, observed from the initiator (host1) at the transport layer:
// host1 sends the SYN, receives the SYN+ACK, and sends the final ACK, with the ack
// numbers following seq+1 at each step. {name} in a match expression is the value
// captured by an earlier step.
Define_ProtocolTest(tcp_handshake_seq)
{
    return ProtocolTest("tcp_handshake_seq")
        // host1 sends the SYN; remember its initial sequence number.
        .expect(on("host1").sentToLower().layer(Layer::Transport)
                  .match("tcp.synBit == true && tcp.ackBit == false")
                  .describe("host1's SYN")
                  .capture("isn", "tcp.sequenceNo")
                  .within(0.2))
        // host1 receives the SYN+ACK acknowledging isn+1; remember the peer's ISN.
        .expect(on("host1").receivedFromLower().layer(Layer::Transport)
                  .match("tcp.synBit == true && tcp.ackBit == true && tcp.ackNo == {isn} + 1")
                  .describe("the SYN+ACK acknowledging host1's ISN+1")
                  .capture("peerIsn", "tcp.sequenceNo")
                  .within(0.5))
        // host1 sends the final ACK acknowledging the peer's ISN+1.
        .expect(on("host1").sentToLower().layer(Layer::Transport)
                  .match("tcp.ackBit == true && tcp.synBit == false && tcp.ackNo == {peerIsn} + 1")
                  .describe("host1's final ACK acknowledging the peer's ISN+1")
                  .within(0.5));
}

// Should FAIL: asserts a wrong ack relationship (isn+2) for the SYN+ACK host1 receives.
Define_ProtocolTest(tcp_handshake_seq_bad)
{
    return ProtocolTest("tcp_handshake_seq_bad")
        .expect(on("host1").sentToLower().layer(Layer::Transport)
                  .match("tcp.synBit == true && tcp.ackBit == false")
                  .capture("isn", "tcp.sequenceNo")
                  .within(0.2))
        .expect(on("host1").receivedFromLower().layer(Layer::Transport)
                  .match("tcp.synBit == true && tcp.ackBit == true && tcp.ackNo == {isn} + 2") // wrong
                  .within(0.5));
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
        .expect(on("host2").sentToLower().layer(Layer::Transport)
                  .match("udp.srcPort == 5000").within(0.2));
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
        .expect(on("host1").sentToLower().layer(Layer::Transport)
                  .match("tcp.synBit == true && tcp.ackBit == false")
                  .describe("host1's SYN (SYN set, ACK clear)")
                  .capture("isn", "tcp.sequenceNo")
                  .capture("clientPort", "tcp.srcPort")
                  .within(1.0))
        // 2. Reactively inject a SYN+ACK acknowledging host1's ISN+1.
        .inject(inject("host1").into("eth[0]", "upperLayerOut").after(0.001)
                  .describe("a SYN+ACK acknowledging host1's ISN+1").packet(buildSynAck))
        // 3. host1's final ACK, acknowledging the peer's ISN+1 (5000+1).
        .expect(on("host1").sentToLower().layer(Layer::Transport)
                  .match("tcp.ackBit == true && tcp.synBit == false && tcp.ackNo == 5001")
                  .describe("host1's final ACK (acknowledging the peer's ISN+1)")
                  .within(1.0));
}

} // namespace protocoltest
} // namespace inet
