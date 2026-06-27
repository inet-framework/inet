//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// VRRPv2 (RFC 3768) ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/vrrp/VrrpSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/vrrp/VrrpAdvertisement_m.h"

namespace inet {
namespace vrrp {

Register_Serializer(VrrpAdvertisement, VrrpSerializer);

void VrrpSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& adv = staticPtrCast<const VrrpAdvertisement>(chunk);
    // version (high nibble) and type (low nibble) share the first octet
    stream.writeByte(((adv->getVersion() & 0x0f) << 4) | (adv->getType() & 0x0f));
    stream.writeByte(adv->getVrid());
    stream.writeByte(adv->getPriority());
    stream.writeByte(adv->getCountIpAddrs());
    stream.writeByte(adv->getAuthType());
    stream.writeByte(adv->getAdverInt());
    stream.writeUint16Be(adv->getChecksum());
    for (size_t i = 0; i < adv->getAddressesArraySize(); i++)
        stream.writeIpv4Address(adv->getAddresses(i));
}

const Ptr<Chunk> VrrpSerializer::deserialize(MemoryInputStream& stream) const
{
    auto adv = makeShared<VrrpAdvertisement>();
    uint8_t versionType = stream.readByte();
    adv->setVersion(versionType >> 4);
    adv->setType(versionType & 0x0f);
    adv->setVrid(stream.readByte());
    adv->setPriority(stream.readByte());
    uint8_t count = stream.readByte();
    adv->setCountIpAddrs(count);
    adv->setAuthType(stream.readByte());
    adv->setAdverInt(stream.readByte());
    adv->setChecksum(stream.readUint16Be());
    adv->setAddressesArraySize(count);
    for (uint8_t i = 0; i < count; i++)
        adv->setAddresses(i, stream.readIpv4Address());
    return adv;
}

} // namespace vrrp
} // namespace inet
