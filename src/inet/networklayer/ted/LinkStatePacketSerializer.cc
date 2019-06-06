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
#include "inet/networklayer/ted/LinkStatePacket_m.h"
#include "inet/networklayer/ted/LinkStatePacketSerializer.h"

namespace inet {

Register_Serializer(LinkStateMsg, LinkStatePacketSerializer);

namespace {

void write64BitDoubleValue(MemoryOutputStream& stream, const double val) {
    uint8_t rawBytes[8];
    std::memcpy(rawBytes, &val, 8);
    stream.writeBytes(rawBytes, B(8));
}

double read64BitDoubleValue(MemoryInputStream& stream) {
    uint8_t rawBytes[8];
    stream.readBytes(rawBytes, B(8));
    double val;
    std::memcpy(&val, rawBytes, 8);
    return val;
}

}

void LinkStatePacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& linkStateMsg = staticPtrCast<const LinkStateMsg>(chunk);
    size_t size = linkStateMsg->getLinkInfoArraySize();
    stream.writeByte(size);
    for (size_t i = 0; i < size; ++i) {
        auto& linkInfo = linkStateMsg->getLinkInfo(i);
        stream.writeIpv4Address(linkInfo.advrouter);
        stream.writeIpv4Address(linkInfo.linkid);
        stream.writeIpv4Address(linkInfo.local);
        stream.writeIpv4Address(linkInfo.remote);
        write64BitDoubleValue(stream, linkInfo.metric);
        write64BitDoubleValue(stream, linkInfo.MaxBandwidth);
        for (uint8_t e = 0; e < 8; ++e) {
            write64BitDoubleValue(stream, linkInfo.UnResvBandwidth[e]);
        }
        stream.writeUint64Be(linkInfo.timestamp.inUnit(SIMTIME_MS));
        stream.writeUint32Be(linkInfo.sourceId);
        stream.writeUint32Be(linkInfo.messageId);
        stream.writeBit(linkInfo.state);
        stream.writeBitRepeatedly(false, 7);
    }
    stream.writeBit(linkStateMsg->getRequest());
    stream.writeBitRepeatedly(false, 7);
    stream.writeUint32Be(linkStateMsg->getCommand());
}

const Ptr<Chunk> LinkStatePacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto linkStateMsg = makeShared<LinkStateMsg>();
    size_t size = stream.readByte();
    linkStateMsg->setLinkInfoArraySize(size);
    for (size_t i = 0; i < size; ++i) {
        TeLinkStateInfo* linkInfo = new TeLinkStateInfo();
        linkInfo->advrouter = Ipv4Address(stream.readIpv4Address());
        linkInfo->linkid = Ipv4Address(stream.readIpv4Address());
        linkInfo->local = Ipv4Address(stream.readIpv4Address());
        linkInfo->remote = Ipv4Address(stream.readIpv4Address());
        linkInfo->metric = read64BitDoubleValue(stream);
        linkInfo->MaxBandwidth = read64BitDoubleValue(stream);
        for (uint8_t e = 0; e < 8; ++e) {
            linkInfo->UnResvBandwidth[e] = read64BitDoubleValue(stream);
        }
        linkInfo->timestamp = SimTime(stream.readUint64Be(), SIMTIME_MS);
        linkInfo->sourceId = stream.readUint32Be();
        linkInfo->messageId = stream.readUint32Be();
        linkInfo->state = stream.readBit();
        stream.readBitRepeatedly(false, 7);
        linkStateMsg->setLinkInfo(i, *linkInfo);
    }
    linkStateMsg->setRequest(stream.readBit());
    stream.readBitRepeatedly(false, 7);
    linkStateMsg->setCommand(stream.readUint32Be());
    linkStateMsg->setChunkLength(B(size * 113 + 6));
    return linkStateMsg;
}

} // namespace inet
