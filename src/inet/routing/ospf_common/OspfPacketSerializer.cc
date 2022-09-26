//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/ospf_common/OspfPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/ospf_common/OspfPacketBase_m.h"
#ifdef INET_WITH_OSPFv2
#include "inet/routing/ospfv2/Ospfv2PacketSerializer.h"
#endif // #ifdef INET_WITH_OSPFv2

namespace inet {
namespace ospf {

Register_Serializer(OspfPacketBase, OspfPacketSerializer);

void OspfPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    throw cRuntimeError("OspfPacketBase is not serializable, should use specific OSPF chunks");
}

const Ptr<Chunk> OspfPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPos = stream.getPosition();
    int ospfVer = stream.readUint8();

    // TODO should register Ospfv<version>Serializer classes to OspfSerializer and deserializer choose a serializer class based on version field
    switch (ospfVer) {
#ifdef INET_WITH_OSPFv2
        case 2:
            stream.seek(startPos);
            return ospfv2::Ospfv2PacketSerializer().deserialize(stream);
#endif // #ifdef INET_WITH_OSPFv2
#ifdef INET_WITH_OSPFv3
        case 3:
            // TODO stream.seek(startPos);
            // TODO return ospfv3::Ospfv3PacketSerializer().deserialize(stream);
#endif // #ifdef INET_WITH_OSPFv3
        default: {
            auto ospfPacket = makeShared<OspfPacketBase>();
            ospfPacket->markIncorrect();
            ospfPacket->setVersion(ospfVer);
            int ospfType = stream.readUint8();
            ospfPacket->setType(static_cast<OspfPacketType>(ospfType));
            uint16_t packetLength = stream.readUint16Be();
            ospfPacket->setPacketLengthField(packetLength);
            ospfPacket->setChunkLength(B(packetLength));
            ospfPacket->setRouterID(stream.readIpv4Address());
            ospfPacket->setAreaID(stream.readIpv4Address());
            ospfPacket->setCrc(stream.readUint16Be());
            ospfPacket->setCrcMode(CRC_COMPUTED);
            uint16_t curLength = B(stream.getPosition() - startPos).get();
            if (packetLength > curLength)
                stream.readByteRepeatedly(0, packetLength - curLength);
            return ospfPacket;
        }
    }
}

} // namespace ospf
} // namespace inet

