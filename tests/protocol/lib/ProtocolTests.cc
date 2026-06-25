//
// Protocol Test Framework for INET -- Phase 1 sample programs.
//
// These exercise the matching engine against the two-host UDP demo network. They
// are registered by name; select one via the ProtocolTester's `testName` parameter.
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

// --- Phase 2: lambda predicates + captures over a TCP handshake ---
//
// Asserts the seq/ack relationship of the three-way handshake using only incoming
// (receivedFromLower) TCP segments, correlated across the two endpoints by captures.
// (INET's Tcp module does not emit packetSentToLower, so outgoing segments are not
// observed at the transport layer -- see plan Phase 2 findings.)

static intval_t tcpSeq(const PacketEvent& e) { return (intval_t)e.packet->peekAtFront<TcpHeader>()->getSequenceNo(); }
static intval_t tcpAck(const PacketEvent& e) { return (intval_t)e.packet->peekAtFront<TcpHeader>()->getAckNo(); }

// Should PASS: the handshake's ack numbers follow seq+1 at each step.
Define_ProtocolTest(tcp_handshake_seq)
{
    return ProtocolTest("tcp_handshake_seq")
        // host2 receives host1's SYN; remember host1's initial sequence number.
        .expect(on("host2").receivedFromLower().layer(Layer::Transport)
                  .match([](const MatchContext& m) {
                      auto tcp = m.event.packet->peekAtFront<TcpHeader>();
                      return tcp->getSynBit() && !tcp->getAckBit();
                  })
                  .capture("isn", [](const PacketEvent& e) { return cValue(tcpSeq(e)); })
                  .within(0.2))
        // host1 receives the SYN+ACK; it must acknowledge isn+1. Remember host2's ISN.
        .expect(on("host1").receivedFromLower().layer(Layer::Transport)
                  .match([](const MatchContext& m) {
                      auto tcp = m.event.packet->peekAtFront<TcpHeader>();
                      return tcp->getSynBit() && tcp->getAckBit()
                          && tcpAck(m.event) == m.get("isn").intValue() + 1;
                  })
                  .capture("peerIsn", [](const PacketEvent& e) { return cValue(tcpSeq(e)); })
                  .within(0.5))
        // host2 receives the final ACK; it must acknowledge peerIsn+1.
        .expect(on("host2").receivedFromLower().layer(Layer::Transport)
                  .match([](const MatchContext& m) {
                      auto tcp = m.event.packet->peekAtFront<TcpHeader>();
                      return tcp->getAckBit() && !tcp->getSynBit()
                          && tcpAck(m.event) == m.get("peerIsn").intValue() + 1;
                  })
                  .within(0.5));
}

// Should FAIL: asserts a wrong ack relationship (isn+2) for the SYN+ACK.
Define_ProtocolTest(tcp_handshake_seq_bad)
{
    return ProtocolTest("tcp_handshake_seq_bad")
        .expect(on("host2").receivedFromLower().layer(Layer::Transport)
                  .match([](const MatchContext& m) {
                      auto tcp = m.event.packet->peekAtFront<TcpHeader>();
                      return tcp->getSynBit() && !tcp->getAckBit();
                  })
                  .capture("isn", [](const PacketEvent& e) { return cValue(tcpSeq(e)); })
                  .within(0.2))
        .expect(on("host1").receivedFromLower().layer(Layer::Transport)
                  .match([](const MatchContext& m) {
                      auto tcp = m.event.packet->peekAtFront<TcpHeader>();
                      return tcp->getSynBit() && tcp->getAckBit()
                          && tcpAck(m.event) == m.get("isn").intValue() + 2; // wrong on purpose
                  })
                  .within(0.5));
}

// --- Phase 3: time-scheduled packet injection ---
//
// Crafts an IP/UDP datagram and pushes it up host2's Ethernet interface as if it had
// just arrived from the wire (the interface's inbound seam, like INET emulation uses).
// host2 runs a UdpEchoApp, so it echoes the datagram back; the program asserts that
// host2 then emits a UDP segment to the lower layer (the echo).

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
        .inject(inject("host2").into("eth[0]", "upperLayerOut").at(0.5).packet(buildInjectedUdpDatagram))
        .expect(on("host2").sentToLower().layer(Layer::Transport)
                  .match("udp.srcPort == 5000").notBefore(0.5).within(0.6));
}

} // namespace protocoltest
} // namespace inet
