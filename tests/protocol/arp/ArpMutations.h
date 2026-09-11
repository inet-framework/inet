//
// Helpers for the ARP level 3 tests: read an address field or the octets of an observed
// ARP packet, read the real addresses of a host, and build a crafted packet to inject.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_ARPMUTATIONS_H
#define __INET_PROTOCOLTEST_ARPMUTATIONS_H

#include <string>
#include <vector>

#include "PacketEvent.h"
#include "PacketField.h"
#include "inet/common/DirectionTag_m.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {
namespace protocoltest {

// ---------------------------------------------------------------------------
// Reading an observed packet
// ---------------------------------------------------------------------------

// A MacAddress field of an observed packet as text, for example
// macAddressField(p, "arp.srcMacAddress").
inline std::string macAddressField(const Packet *packet, const char *fieldPath)
{
    return evalPacketField(packet, fieldPath).pointerValue().get<MacAddress>()->str();
}

// An Ipv4Address field of an observed packet as text, for example
// ipv4AddressField(p, "arp.destIpAddress").
inline std::string ipv4AddressField(const Packet *packet, const char *fieldPath)
{
    return evalPacketField(packet, fieldPath).pointerValue().get<Ipv4Address>()->str();
}

// True when the packet matches a PacketFilter expression; an expression that does not apply
// to the packet (for example arp.* on a datagram) is a non-match, never an error.
inline bool matchesExpression(const Packet *packet, const char *expression)
{
    PacketFilter filter;
    filter.setExpression(expression);
    try {
        return filter.matches(packet);
    }
    catch (const std::exception&) {
        return false;
    }
}

// The octets of the ARP packet inside an observed frame, as the serializer of the protocol
// writes them. The frame a node puts on a link holds the ARP packet as a field object, so
// the octets have to be produced from it; they are the octets the wire carries.
inline std::vector<uint8_t> arpOctets(const Packet *frame)
{
    std::unique_ptr<Packet> copy(frame->dup());
    while (true) {
        const auto& front = copy->peekAtFront<Chunk>();
        if (dynamicPtrCast<const ArpPacket>(front) != nullptr)
            break;
        copy->removeAtFront<Chunk>(front->getChunkLength());
    }
    const auto& arp = copy->peekAtFront<ArpPacket>();
    MemoryOutputStream stream;
    Chunk::serialize(stream, arp);
    return stream.getData();
}

// A 16-bit field of an octet sequence, read most significant byte first.
inline uint16_t octetsUint16(const std::vector<uint8_t>& octets, size_t offset)
{
    return static_cast<uint16_t>((octets.at(offset) << 8) | octets.at(offset + 1));
}

// True when six octets at an offset hold this hardware address. A check of the layout needs
// it: an address at the right offset is the evidence that nothing pads the fields.
inline bool octetsHoldMacAddress(const std::vector<uint8_t>& octets, size_t offset, MacAddress address)
{
    for (unsigned int i = 0; i < MAC_ADDRESS_SIZE; i++)
        if (octets.at(offset + i) != address.getAddressByte(i))
            return false;
    return true;
}

// True when four octets at an offset hold this protocol address, most significant octet
// first.
inline bool octetsHoldIpv4Address(const std::vector<uint8_t>& octets, size_t offset, Ipv4Address address)
{
    uint32_t value = address.getInt();
    for (int i = 0; i < 4; i++)
        if (octets.at(offset + i) != ((value >> (8 * (3 - i))) & 0xFF))
            return false;
    return true;
}

// ---------------------------------------------------------------------------
// The real addresses of a host
// ---------------------------------------------------------------------------

// The network interface module at a path such as "hostB.eth[0]".
inline NetworkInterface *interfaceAt(const char *interfacePath)
{
    auto network = cSimulation::getActiveSimulation()->getSystemModule();
    return check_and_cast<NetworkInterface *>(network->findModuleByPath(interfacePath));
}

inline MacAddress interfaceMacAddress(const char *interfacePath)
{
    return interfaceAt(interfacePath)->getMacAddress();
}

inline Ipv4Address interfaceIpv4Address(const char *interfacePath)
{
    return interfaceAt(interfacePath)->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
}

// ---------------------------------------------------------------------------
// Crafted packets
// ---------------------------------------------------------------------------

// The tags an ARP packet needs to travel up from an interface as an inbound packet: where
// it came from, which protocol it is, and which protocol module handles it.
inline void addInboundArpTags(Packet *packet, const char *receiverInterfacePath, MacAddress sha, MacAddress tha)
{
    auto interfaceId = interfaceAt(receiverInterfacePath)->getInterfaceId();
    packet->addTag<InterfaceInd>()->setInterfaceId(interfaceId);
    auto macAddressInd = packet->addTag<MacAddressInd>();
    macAddressInd->setSrcAddress(sha);
    macAddressInd->setDestAddress(tha);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::arp);
    packet->addTag<DirectionTag>()->setDirection(DIRECTION_INBOUND);
    auto dispatchReq = packet->addTag<DispatchProtocolReq>();
    dispatchReq->setProtocol(&Protocol::arp);
    dispatchReq->setServicePrimitive(SP_INDICATION);
}

// An ARP packet built from its field values, as if it had arrived on the named interface.
// The hardware space, the protocol space and the two length fields are those of an
// Ethernet link that resolves IPv4; buildRawArpPacket changes them.
inline Packet *buildArpPacket(const char *name, const char *receiverInterfacePath, int opcode,
        MacAddress sha, Ipv4Address spa, MacAddress tha, Ipv4Address tpa)
{
    auto packet = new Packet(name);
    auto arp = makeShared<ArpPacket>();
    arp->setOpcode(static_cast<ArpOpcode>(opcode));
    arp->setSrcMacAddress(sha);
    arp->setSrcIpAddress(spa);
    arp->setDestMacAddress(tha);
    arp->setDestIpAddress(tpa);
    packet->insertAtFront(arp);
    addInboundArpTags(packet, receiverInterfacePath, sha, tha);
    return packet;
}

// An ARP packet built octet by octet, as if it had arrived on the named interface. Every
// field of the layout of RFC 826 is an argument, so a check can craft a hardware space or a
// protocol space value that no field object can hold.
inline Packet *buildRawArpPacket(const char *name, const char *receiverInterfacePath,
        uint16_t hardwareSpace, uint16_t protocolSpace, uint8_t hardwareLength, uint8_t protocolLength,
        uint16_t opcode, MacAddress sha, Ipv4Address spa, MacAddress tha, Ipv4Address tpa)
{
    MemoryOutputStream stream;
    stream.writeUint16Be(hardwareSpace);
    stream.writeUint16Be(protocolSpace);
    stream.writeByte(hardwareLength);
    stream.writeByte(protocolLength);
    stream.writeUint16Be(opcode);
    stream.writeMacAddress(sha);
    stream.writeIpv4Address(spa);
    stream.writeMacAddress(tha);
    stream.writeIpv4Address(tpa);
    auto packet = new Packet(name);
    packet->insertAtFront(makeShared<BytesChunk>(stream.getData()));
    addInboundArpTags(packet, receiverInterfacePath, sha, tha);
    return packet;
}

// ---------------------------------------------------------------------------
// Absence
// ---------------------------------------------------------------------------

// True when an ARP packet leaves the named node. Every discard check of ARP is an absence,
// because the protocol has no error message of any kind, so this is the predicate that such
// a check watches for.
inline bool arpPacketLeaves(const PacketEvent& event, const char *node)
{
    std::string macPath = std::string(node) + ".eth[0].mac";
    return event.kind == EventKind::SentToLower && event.sourcePath == macPath
           && matchesExpression(event.packet, "arp.opcode >= 0");
}

} // namespace protocoltest
} // namespace inet

#endif
