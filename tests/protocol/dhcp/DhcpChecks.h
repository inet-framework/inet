//
// Helpers for the DHCP standards tests: read a DHCP message out of a frame, read the option
// area of that message as octets, and build a crafted message for injection.
//
// Why a helper header and not filter expressions. A filter expression reaches the fields of
// the DHCP message (DhcpMessage.op, DhcpMessage.options.messageType), but three things the
// checks need are out of its reach: an address field is a pointer value and not a string, so
// it cannot be compared in an expression; the absence of an option is a property of a field
// that always exists in the message class; and the order of the options is a property of the
// octets and not of the fields.
//
// How an absent option is decided. The message class holds the union of the options as plain
// fields, so nothing in it says "this option is not here". The serializer decides, and its
// rule is per option: an address option is absent when the address is unspecified, a time
// option when the value is zero, a list option when the array is empty, and the type option
// when the value is -1 (DhcpMessageSerializer::serializeFields). The predicates below follow
// that rule, so `hasOption(m, LEASE_TIME)` is true exactly when the octets on the wire hold
// code 51. Where a check needs the octets themselves, scanDhcpOptions gives them.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_DHCPCHECKS_H
#define __INET_PROTOCOLTEST_DHCPCHECKS_H

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "PacketEvent.h"
#include "inet/applications/dhcp/DhcpMessage_m.h"
#include "inet/common/DirectionTag_m.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace protocoltest {

// The DHCP ports of RFC 2131 section 4.1.
const int DHCP_SERVER_PORT = 67;
const int DHCP_CLIENT_PORT = 68;

// The fixed part of a DHCP message, in octets: RFC 2131 figure 1 without the options field.
const int DHCP_FIXED_PART = 236;

// The first chunk of type T in a frame, or nullptr. The frame a check observes at a MAC
// starts with the link-layer header and carries the DHCP message behind the IP and UDP
// headers; the same frame at a transport module starts with the UDP header. One loop serves
// both, and it stops at the first chunk it cannot split (a serialized region), which is why
// it returns nullptr instead of throwing.
template<typename T>
inline Ptr<const T> findChunk(const Packet *packet)
{
    if (packet == nullptr)
        return nullptr;
    std::unique_ptr<Packet> copy(packet->dup());
    try {
        while (copy->getDataLength() > b(0)) {
            const auto& front = copy->peekAtFront<Chunk>(b(-1), Chunk::PF_ALLOW_NULLPTR | Chunk::PF_ALLOW_INCORRECT);
            if (front == nullptr)
                return nullptr;
            auto wanted = dynamicPtrCast<const T>(front);
            if (wanted != nullptr)
                return wanted;
            copy->removeAtFront<Chunk>(front->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
        }
    }
    catch (const std::exception&) {
        return nullptr;
    }
    return nullptr;
}

// The DHCP message a frame carries, or nullptr when it carries none.
inline Ptr<const DhcpMessage> dhcpOf(const Packet *packet)
{
    return findChunk<DhcpMessage>(packet);
}

// True when the frame carries a DHCP message of this type. Every check that names a message
// goes through this predicate, because RFC 2131 section 3 identifies a message by its
// 'DHCP message type' option and not by its name in the simulation.
inline bool isDhcp(const Packet *packet, DhcpMessageType type)
{
    auto dhcp = dhcpOf(packet);
    return dhcp != nullptr && dhcp->getOptions().getMessageType() == type;
}

// True when the frame carries any DHCP message.
inline bool isAnyDhcp(const Packet *packet)
{
    return dhcpOf(packet) != nullptr;
}

// Whether the option with this code would appear in the octets of the message. The rule per
// code is the serializer's; see the note at the head of this file.
inline bool hasOption(const Ptr<const DhcpMessage>& dhcp, DhcpOptionCode code)
{
    if (dhcp == nullptr)
        return false;
    const DhcpOptions& o = dhcp->getOptions();
    switch (code) {
        case DHCP_MSG_TYPE: return o.getMessageType() != static_cast<DhcpMessageType>(-1);
        case HOSTNAME: return o.getHostName() != nullptr && o.getHostName()[0] != '\0';
        case PARAM_LIST: return o.getParameterRequestListArraySize() > 0;
        case CLIENT_ID: return !o.getClientIdentifier().isUnspecified();
        case REQUESTED_IP: return !o.getRequestedIp().isUnspecified();
        case SUBNET_MASK: return !o.getSubnetMask().isUnspecified();
        case ROUTER: return o.getRouterArraySize() > 0;
        case DNS: return o.getDnsArraySize() > 0;
        case NTP_SRV: return o.getNtpArraySize() > 0;
        case SERVER_ID: return !o.getServerIdentifier().isUnspecified();
        case RENEWAL_TIME: return !o.getRenewalTime().isZero();
        case REBIND_TIME: return !o.getRebindingTime().isZero();
        case LEASE_TIME: return !o.getLeaseTime().isZero();
        default: return false;
    }
}

inline bool hasOption(const Packet *packet, DhcpOptionCode code)
{
    return hasOption(dhcpOf(packet), code);
}

// The IPv4 header of a frame, or nullptr.
inline Ptr<const Ipv4Header> ipv4Of(const Packet *packet)
{
    return findChunk<Ipv4Header>(packet);
}

// The UDP header of a frame, or nullptr.
inline Ptr<const UdpHeader> udpOf(const Packet *packet)
{
    return findChunk<UdpHeader>(packet);
}

// An IPv4 address as dotted-quad text, always: the default printer of the class writes
// "<unspec>" for 0.0.0.0, and the checks name the address the way RFC 2131 does.
inline std::string quad(const Ipv4Address& address)
{
    return address.str(false);
}

// The IPv4 source address of a frame, as text; empty when the frame carries no IPv4 header.
inline std::string ipv4Source(const Packet *packet)
{
    auto header = ipv4Of(packet);
    return header == nullptr ? "" : quad(header->getSrcAddress());
}

// The IPv4 destination address of a frame, as text.
inline std::string ipv4Destination(const Packet *packet)
{
    auto header = ipv4Of(packet);
    return header == nullptr ? "" : quad(header->getDestAddress());
}

// The UDP destination port of a frame, or -1.
inline int udpDestinationPort(const Packet *packet)
{
    auto header = udpOf(packet);
    return header == nullptr ? -1 : header->getDestinationPort();
}

// The link-layer destination address of a frame, as text; empty when the frame has no
// Ethernet header. A check of RFC 2131 section 4.1 needs it, because the rule names the
// link-layer destination beside the IP destination.
inline std::string macDestination(const Packet *packet)
{
    auto header = findChunk<EthernetMacHeader>(packet);
    return header == nullptr ? "" : header->getDest().str();
}

// True when the frame goes to the link-layer broadcast address.
inline bool isLinkBroadcast(const Packet *packet)
{
    auto header = findChunk<EthernetMacHeader>(packet);
    return header != nullptr && header->getDest().isBroadcast();
}

// The octets of the DHCP message a frame carries. The message travels as a field-based chunk,
// so the octets do not exist on the wire until something serializes them; this helper does
// what a receiver's parser would do, which is what the framing rules of RFC 2132 section 2
// are about.
inline std::vector<uint8_t> dhcpOctets(const Packet *packet)
{
    auto dhcp = dhcpOf(packet);
    if (dhcp == nullptr)
        return {};
    MemoryOutputStream stream;
    Chunk::serialize(stream, dhcp);
    return stream.getData();
}

// The result of walking the option area of a DHCP message by the rule of RFC 2132 section 2:
// a tag octet, then a length octet for every code except 0 and 255, then that many octets of
// value.
struct DhcpOptionScan {
    bool cookieOk = false;             // the four octets 99, 130, 83, 99 start the area
    bool parsed = false;               // the walk reached the end option with nothing left over
    bool endsWithEnd = false;          // the last option of the area is code 255
    std::vector<int> codes;            // the codes in the order the octets carry them
    std::vector<int> offsets;          // the offset of each code, from the start of the message
    std::vector<std::vector<uint8_t>> values; // the value octets of each code

    bool has(int code) const { return std::find(codes.begin(), codes.end(), code) != codes.end(); }

    int offsetOf(int code) const
    {
        auto it = std::find(codes.begin(), codes.end(), code);
        return it == codes.end() ? -1 : offsets.at(it - codes.begin());
    }

    std::vector<uint8_t> valueOf(int code) const
    {
        auto it = std::find(codes.begin(), codes.end(), code);
        return it == codes.end() ? std::vector<uint8_t>() : values.at(it - codes.begin());
    }

    int countOf(int code) const { return std::count(codes.begin(), codes.end(), code); }

    bool anyCodeTwice() const
    {
        for (auto code : codes)
            if (countOf(code) > 1)
                return true;
        return false;
    }
};

// Walks the option area of the DHCP message a frame carries. The area starts at octet 236,
// after the fixed part of RFC 2131 figure 1.
inline DhcpOptionScan scanDhcpOptions(const Packet *packet)
{
    DhcpOptionScan scan;
    auto octets = dhcpOctets(packet);
    if (octets.size() < static_cast<size_t>(DHCP_FIXED_PART) + 4)
        return scan;
    size_t i = DHCP_FIXED_PART;
    scan.cookieOk = octets[i] == 99 && octets[i + 1] == 130 && octets[i + 2] == 83 && octets[i + 3] == 99;
    i += 4;
    while (i < octets.size()) {
        int code = octets[i];
        scan.codes.push_back(code);
        scan.offsets.push_back(static_cast<int>(i));
        if (code == 255) { // the end option: one octet, no length
            scan.values.push_back({});
            scan.endsWithEnd = true;
            scan.parsed = (i + 1 == octets.size());
            return scan;
        }
        if (code == 0) { // the pad option: one octet, no length
            scan.values.push_back({});
            i += 1;
            continue;
        }
        if (i + 1 >= octets.size())
            return scan; // a tag with no length octet: the area does not parse
        size_t length = octets[i + 1];
        if (i + 2 + length > octets.size())
            return scan; // a value that runs past the end of the message
        scan.values.push_back(std::vector<uint8_t>(octets.begin() + i + 2, octets.begin() + i + 2 + length));
        i += 2 + length;
    }
    return scan; // the area ended without an end option
}

// Builds a DHCP message with the fixed part filled in, ready for the options to be set. The
// caller sets the options and then calls finishDhcp, which computes the length the serializer
// expects. Only the fields RFC 2131 table 1 defines are set here; everything else keeps the
// default of the message class.
inline Ptr<DhcpMessage> makeDhcp(DhcpOpcode op, uint32_t xid, const MacAddress& chaddr)
{
    auto dhcp = makeShared<DhcpMessage>();
    dhcp->setOp(op);
    dhcp->setHtype(1); // Ethernet, per the ARP section of the Assigned Numbers RFC
    dhcp->setHlen(6);
    dhcp->setHops(0);
    dhcp->setXid(xid);
    dhcp->setSecs(0);
    dhcp->setBroadcast(false);
    dhcp->setCiaddr(Ipv4Address());
    dhcp->setYiaddr(Ipv4Address());
    dhcp->setGiaddr(Ipv4Address());
    dhcp->setChaddr(chaddr);
    dhcp->setSname("");
    dhcp->setFile("");
    return dhcp;
}

// Sets the chunk length of a crafted message to the number of octets its options will need.
// The arithmetic is the serializer's, and it must agree with it: the serializer asserts that
// the two match.
inline void finishDhcp(const Ptr<DhcpMessage>& dhcp)
{
    const DhcpOptions& o = dhcp->getOptions();
    int length = DHCP_FIXED_PART + 4 + 1; // the fixed part, the magic cookie, the end option
    if (hasOption(dhcp, DHCP_MSG_TYPE))
        length += 3;
    if (hasOption(dhcp, HOSTNAME))
        length += 2 + strlen(o.getHostName());
    if (hasOption(dhcp, PARAM_LIST))
        length += 2 + o.getParameterRequestListArraySize();
    if (hasOption(dhcp, CLIENT_ID))
        length += 2 + 1 + 6;
    if (hasOption(dhcp, REQUESTED_IP))
        length += 6;
    if (hasOption(dhcp, SUBNET_MASK))
        length += 6;
    if (hasOption(dhcp, ROUTER))
        length += 2 + 4 * o.getRouterArraySize();
    if (hasOption(dhcp, DNS))
        length += 2 + 4 * o.getDnsArraySize();
    if (hasOption(dhcp, NTP_SRV))
        length += 2 + 4 * o.getNtpArraySize();
    if (hasOption(dhcp, SERVER_ID))
        length += 6;
    if (hasOption(dhcp, RENEWAL_TIME))
        length += 6;
    if (hasOption(dhcp, REBIND_TIME))
        length += 6;
    if (hasOption(dhcp, LEASE_TIME))
        length += 6;
    dhcp->setChunkLength(B(length));
}

// The interface identifier of a named interface of a named network node. An injected packet
// enters above the MAC, so nothing has told it which interface it arrived on; a DHCP server
// drops a message whose arrival interface is not the one it serves, and so does much else in
// the stack. The identifier the crafted packet needs is the one a real arriving frame would
// carry, and only the node's own interface table knows it.
inline int interfaceIdOf(const char *nodeName, const char *interfaceName)
{
    auto network = cSimulation::getActiveSimulation()->getSystemModule();
    auto node = network->getSubmodule(nodeName);
    if (node == nullptr)
        throw cRuntimeError("ProtocolTest: no node '%s' in the network", nodeName);
    auto interfaceTable = check_and_cast<IInterfaceTable *>(node->getSubmodule("interfaceTable"));
    auto interface = interfaceTable->findInterfaceByName(interfaceName);
    if (interface == nullptr)
        throw cRuntimeError("ProtocolTest: no interface '%s' at node '%s'", interfaceName, nodeName);
    return interface->getInterfaceId();
}

// Wraps a crafted DHCP message in the UDP and IPv4 headers it needs and tags it as an inbound
// datagram, so that pushing it into a node's upperLayerOut gate has the same effect as its
// arrival from the wire. The pattern is the one of the framework's own injection self tests.
inline Packet *wrapDhcp(const char *name, const Ptr<DhcpMessage>& dhcp,
        const Ipv4Address& source, const Ipv4Address& destination, int sourcePort, int destinationPort,
        const char *arrivalNode = nullptr, const char *arrivalInterface = "eth0")
{
    auto packet = new Packet(name);
    packet->insertAtBack(dhcp);

    auto udpHeader = makeShared<UdpHeader>();
    udpHeader->setSourcePort(sourcePort);
    udpHeader->setDestinationPort(destinationPort);
    udpHeader->setTotalLengthField(B(8) + dhcp->getChunkLength());
    udpHeader->setChecksum(0);
    udpHeader->setChecksumMode(CHECKSUM_DECLARED_CORRECT);
    packet->insertAtFront(udpHeader);

    auto ipv4Header = makeShared<Ipv4Header>();
    ipv4Header->setSrcAddress(source);
    ipv4Header->setDestAddress(destination);
    ipv4Header->setProtocolId(IP_PROT_UDP);
    ipv4Header->setTimeToLive(64);
    ipv4Header->setTotalLengthField(B(20) + B(8) + dhcp->getChunkLength());
    ipv4Header->setChecksum(0);
    ipv4Header->setChecksumMode(CHECKSUM_DECLARED_CORRECT);
    packet->insertAtFront(ipv4Header);

    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    packet->addTag<DirectionTag>()->setDirection(DIRECTION_INBOUND);
    auto dispatchReq = packet->addTag<DispatchProtocolReq>();
    dispatchReq->setProtocol(&Protocol::ipv4);
    dispatchReq->setServicePrimitive(SP_INDICATION);
    if (arrivalNode != nullptr)
        packet->addTag<InterfaceInd>()->setInterfaceId(interfaceIdOf(arrivalNode, arrivalInterface));
    return packet;
}

} // namespace protocoltest
} // namespace inet

#endif
