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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/ospf_common/OspfPacketBase_m.h"
#include "inet/routing/ospf_common/OspfPacketSerializer.h"
#ifdef WITH_OSPFv2
#include "inet/routing/ospfv2/Ospfv2PacketSerializer.h"
#endif // #ifdef WITH_OSPFv2

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

    //TODO should register Ospfv<version>Serializer classes to OspfSerializer and deserializer choose a serializer class based on version field
    switch (ospfVer) {
#ifdef WITH_OSPFv2
        case 2:
            stream.seek(startPos);
            return ospfv2::Ospfv2PacketSerializer().deserialize(stream);
            break;
#endif // #ifdef WITH_OSPFv2
#ifdef WITH_OSPFv3
        case 3:
            //TODO stream.seek(startPos);
            //TODO return ospfv3::Ospfv3PacketSerializer().deserialize(stream);
#endif // #ifdef WITH_OSPFv3
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

