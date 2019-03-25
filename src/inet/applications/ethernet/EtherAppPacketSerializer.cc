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

#include "inet/applications/ethernet/EtherApp_m.h"
#include "inet/applications/ethernet/EtherAppPacketSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(EtherAppReq, EtherAppReqSerializer);
Register_Serializer(EtherAppResp, EtherAppRespSerializer);

void EtherAppReqSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& etherAppReq = staticPtrCast<const EtherAppReq>(chunk);
    stream.writeUint32Be(B(etherAppReq->getChunkLength()).get());
    stream.writeUint32Be(etherAppReq->getRequestId());
    stream.writeUint32Be(etherAppReq->getResponseBytes());
    int64_t remainders = B(etherAppReq->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("EtherAppReq length = %d smaller than required %d bytes", (int)B(etherAppReq->getChunkLength()).get(), (int)B(stream.getLength() - startPosition).get());
    stream.writeByteRepeatedly('?', remainders);
}

const Ptr<Chunk> EtherAppReqSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto etherAppReq = makeShared<EtherAppReq>();
    B dataLength = B(stream.readUint32Be());
    etherAppReq->setChunkLength(dataLength);
    etherAppReq->setRequestId(stream.readUint32Be());
    etherAppReq->setResponseBytes(stream.readUint32Be());
    B remainders = dataLength - (stream.getPosition() - startPosition);
    ASSERT(remainders >= B(0));
    stream.readByteRepeatedly('?', B(remainders).get());
    return etherAppReq;
}

void EtherAppRespSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& etherAppResp = staticPtrCast<const EtherAppResp>(chunk);
    stream.writeUint32Be(B(etherAppResp->getChunkLength()).get());
    stream.writeUint32Be(etherAppResp->getRequestId());
    stream.writeUint32Be(etherAppResp->getNumFrames());
    int64_t remainders = B(etherAppResp->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("EtherAppResp length = %d smaller than required %d bytes", (int)B(etherAppResp->getChunkLength()).get(), (int)B(stream.getLength() - startPosition).get());
    stream.writeByteRepeatedly('?', remainders);
}

const Ptr<Chunk> EtherAppRespSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto etherAppResp = makeShared<EtherAppResp>();
    B dataLength = B(stream.readUint32Be());
    etherAppResp->setChunkLength(dataLength);
    etherAppResp->setRequestId(stream.readUint32Be());
    etherAppResp->setNumFrames(stream.readUint32Be());
    B remainders = dataLength - (stream.getPosition() - startPosition);
    ASSERT(remainders >= B(0));
    stream.readByteRepeatedly('?', B(remainders).get());
    return etherAppResp;
}

} // namespace inet

