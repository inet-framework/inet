// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/routing/aodv/AodvSubnetAnnouncementSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"

namespace inet {
namespace aodv {

Register_Serializer(AodvSubnetAnnouncement, AodvSubnetAnnouncementSerializer);

void AodvSubnetAnnouncementSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto announcement = staticPtrCast<const AodvSubnetAnnouncement>(chunk);
    bool ipv6 = announcement->getAddressFamily() == 6;
    if ((!ipv6 && announcement->getAddressFamily() != 4) ||
            announcement->getPrefixLength() == 0 || announcement->getPrefixLength() >= (ipv6 ? 128 : 32) ||
            announcement->getCandidateAddress().getType() != (ipv6 ? L3Address::IPv6 : L3Address::IPv4) ||
            announcement->getPrefix().getType() != announcement->getCandidateAddress().getType() ||
            announcement->getPrefix() != announcement->getPrefix().getPrefix(announcement->getPrefixLength()) ||
            announcement->getChunkLength() != (ipv6 ? B(40) : B(16)))
        throw cRuntimeError("Invalid INET AODV subnet announcement");
    stream.writeByte(announcement->getAddressFamily());
    stream.writeByte((announcement->getEligible() ? 1 : 0) | (announcement->getSequenceValid() ? 2 : 0));
    stream.writeByte(announcement->getPrefixLength());
    stream.writeByte(0);
    stream.writeUint32Be(announcement->getSequenceNumber());
    if (ipv6) {
        stream.writeIpv6Address(announcement->getCandidateAddress().toIpv6());
        stream.writeIpv6Address(announcement->getPrefix().toIpv6());
    }
    else {
        stream.writeIpv4Address(announcement->getCandidateAddress().toIpv4());
        stream.writeIpv4Address(announcement->getPrefix().toIpv4());
    }
}

const Ptr<Chunk> AodvSubnetAnnouncementSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    auto announcement = makeShared<AodvSubnetAnnouncement>();
    auto family = stream.readByte();
    auto flags = stream.readByte();
    auto prefixLength = stream.readByte();
    auto reserved = stream.readByte();
    announcement->setAddressFamily(family);
    announcement->setEligible(flags & 1);
    announcement->setSequenceValid(flags & 2);
    announcement->setPrefixLength(prefixLength);
    announcement->setSequenceNumber(stream.readUint32Be());
    if (family == 4) {
        announcement->setCandidateAddress(L3Address(stream.readIpv4Address()));
        announcement->setPrefix(L3Address(stream.readIpv4Address()));
    }
    else if (family == 6) {
        announcement->setCandidateAddress(L3Address(stream.readIpv6Address()));
        announcement->setPrefix(L3Address(stream.readIpv6Address()));
    }
    else
        announcement->markIncorrect();
    if ((flags & ~3) || reserved || prefixLength == 0 || prefixLength >= (family == 6 ? 128 : 32) ||
            ((family == 4 || family == 6) && announcement->getPrefix() != announcement->getPrefix().getPrefix(prefixLength)))
        announcement->markIncorrect();
    return announcement;
}

} // namespace aodv
} // namespace inet
