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

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/ChunkBuffer.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/ReassemblyBuffer.h"
#include "inet/common/packet/ReorderBuffer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/TimeTag_m.h"
#include "UnitTest_m.h"
#include "UnitTest.h"

namespace inet {

Register_Serializer(ApplicationHeader, ApplicationHeaderSerializer);
Register_Serializer(TcpHeader, TcpHeaderSerializer);
Register_Serializer(IpHeader, IpHeaderSerializer);
Register_Serializer(EthernetHeader, EthernetHeaderSerializer);
Register_Serializer(EthernetTrailer, EthernetTrailerSerializer);
Register_Serializer(CompoundHeader, CompoundHeaderSerializer);
Register_Serializer(TlvHeader, TlvHeaderSerializer);
Register_Serializer(TlvHeaderBool, TlvHeaderBoolSerializer);
Register_Serializer(TlvHeaderInt, TlvHeaderIntSerializer);
Define_Module(UnitTest);

#define ASSERT_ERROR(code, message) try { code; ASSERT(false); } catch (std::exception& e) { ASSERT((int)std::string(e.what()).find(message) != -1); }

static std::vector<uint8_t> makeVector(int length)
{
    std::vector<uint8_t> bytes;
    for (int i = 0; i < length; i++)
        bytes.push_back(i);
    return bytes;
}

static const Ptr<ByteCountChunk> makeImmutableByteCountChunk(B length)
{
    auto chunk = makeShared<ByteCountChunk>(length);
    chunk->markImmutable();
    return chunk;
}

static const Ptr<BytesChunk> makeImmutableBytesChunk(const std::vector<uint8_t>& bytes)
{
    auto chunk = makeShared<BytesChunk>(bytes);
    chunk->markImmutable();
    return chunk;
}

static const Ptr<ApplicationHeader> makeImmutableApplicationHeader(int someData)
{
    auto chunk = makeShared<ApplicationHeader>();
    chunk->setSomeData(someData);
    chunk->markImmutable();
    return chunk;
}

static const Ptr<TcpHeader> makeImmutableTcpHeader()
{
    auto chunk = makeShared<TcpHeader>();
    chunk->setChunkLength(B(20));
    chunk->markImmutable();
    return chunk;
}

static const Ptr<IpHeader> makeImmutableIpHeader()
{
    auto chunk = makeShared<IpHeader>();
    chunk->markImmutable();
    return chunk;
}

static const Ptr<EthernetHeader> makeImmutableEthernetHeader()
{
    auto chunk = makeShared<EthernetHeader>();
    chunk->markImmutable();
    return chunk;
}

static const Ptr<EthernetTrailer> makeImmutableEthernetTrailer()
{
    auto chunk = makeShared<EthernetTrailer>();
    chunk->markImmutable();
    return chunk;
}

void ApplicationHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& applicationHeader = staticPtrCast<const ApplicationHeader>(chunk);
    auto position = stream.getLength();
    stream.writeUint16Be(applicationHeader->getSomeData());
    stream.writeByteRepeatedly(0, B(applicationHeader->getChunkLength() - stream.getLength() + position).get());
}

const Ptr<Chunk> ApplicationHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto applicationHeader = makeShared<ApplicationHeader>();
    auto position = stream.getPosition();
    applicationHeader->setSomeData(stream.readUint16Be());
    stream.readByteRepeatedly(0, B(applicationHeader->getChunkLength() - stream.getPosition() + position).get());
    return applicationHeader;
}

void TcpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& tcpHeader = staticPtrCast<const TcpHeader>(chunk);
    auto position = stream.getLength();
    if (tcpHeader->getCrcMode() != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize TCP header");
    stream.writeUint16Be(tcpHeader->getLengthField());
    stream.writeUint16Be(tcpHeader->getSrcPort());
    stream.writeUint16Be(tcpHeader->getDestPort());
    stream.writeUint16Be(tcpHeader->getCrc());
    stream.writeByteRepeatedly(0, B(tcpHeader->getChunkLength() - stream.getLength() + position).get());
}

const Ptr<Chunk> TcpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto tcpHeader = makeShared<TcpHeader>();
    auto position = stream.getPosition();
    auto remainingLength = stream.getRemainingLength();
    auto lengthField = B(stream.readUint16Be());
    if (lengthField > remainingLength)
        tcpHeader->markIncomplete();
    auto length = lengthField < remainingLength ? lengthField : B(remainingLength);
    tcpHeader->setChunkLength(B(length));
    tcpHeader->setLengthField(B(lengthField).get());
    tcpHeader->setSrcPort(stream.readUint16Be());
    tcpHeader->setDestPort(stream.readUint16Be());
    tcpHeader->setCrcMode(CRC_COMPUTED);
    tcpHeader->setCrc(stream.readUint16Be());
    stream.readByteRepeatedly(0, B(length - stream.getPosition() + position).get());
    return tcpHeader;
}

void IpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ipHeader = staticPtrCast<const IpHeader>(chunk);
    auto position = stream.getLength();
    stream.writeUint16Be((int16_t)ipHeader->getProtocol());
    stream.writeByteRepeatedly(0, B(ipHeader->getChunkLength() - stream.getLength() + position).get());
}

const Ptr<Chunk> IpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ipHeader = makeShared<IpHeader>();
    auto position = stream.getPosition();
    Protocol protocol = (Protocol)stream.readUint16Be();
    if (protocol != Protocol::Tcp && protocol != Protocol::Ip && protocol != Protocol::Ethernet)
        ipHeader->markImproperlyRepresented();
    ipHeader->setProtocol(protocol);
    stream.readByteRepeatedly(0, B(ipHeader->getChunkLength() - stream.getPosition() + position).get());
    return ipHeader;
}

void EthernetHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ethernetHeader = staticPtrCast<const EthernetHeader>(chunk);
    auto position = stream.getLength();
    stream.writeUint16Be((int16_t)ethernetHeader->getProtocol());
    stream.writeByteRepeatedly(0, B(ethernetHeader->getChunkLength() - stream.getLength() + position).get());
}

const Ptr<Chunk> EthernetHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ethernetHeader = makeShared<EthernetHeader>();
    auto position = stream.getPosition();
    ethernetHeader->setProtocol((Protocol)stream.readUint16Be());
    stream.readByteRepeatedly(0, B(ethernetHeader->getChunkLength() - stream.getPosition() + position).get());
    return ethernetHeader;
}

void EthernetTrailerSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ethernetTrailer = staticPtrCast<const EthernetTrailer>(chunk);
    auto position = stream.getLength();
    stream.writeUint16Be(ethernetTrailer->getCrc());
    stream.writeByteRepeatedly(0, B(ethernetTrailer->getChunkLength() - stream.getLength() + position).get());
}

const Ptr<Chunk> EthernetTrailerSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ethernetTrailer = makeShared<EthernetTrailer>();
    auto position = stream.getPosition();
    ethernetTrailer->setCrc(stream.readUint16Be());
    stream.readByteRepeatedly(0, B(ethernetTrailer->getChunkLength() - stream.getPosition() + position).get());
    return ethernetTrailer;
}

const Ptr<Chunk> CompoundHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto compoundHeader = makeShared<CompoundHeader>();
    IpHeaderSerializer ipHeaderSerializer;
    auto ipHeader = ipHeaderSerializer.deserialize(stream);
    compoundHeader->insertAtBack(ipHeader);
    return compoundHeader;
}

void TlvHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    throw cRuntimeError("Invalid operation");
}

const Ptr<Chunk> TlvHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t type = stream.readUint8();
    stream.seek(stream.getPosition() - B(1));
    switch (type) {
        case 1:
            return TlvHeaderBoolSerializer().deserialize(stream);
        case 2:
            return TlvHeaderIntSerializer().deserialize(stream);
        default:
            throw cRuntimeError("Invalid TLV type");
    }
}

void TlvHeaderBoolSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& tlvHeader = staticPtrCast<const TlvHeaderBool>(chunk);
    stream.writeUint8(tlvHeader->getType());
    stream.writeUint8(B(tlvHeader->getChunkLength()).get());
    stream.writeUint8(tlvHeader->getBoolValue());
}

const Ptr<Chunk> TlvHeaderBoolSerializer::deserialize(MemoryInputStream& stream) const
{
    auto tlvHeader = makeShared<TlvHeaderBool>();
    ASSERT(tlvHeader->getType() == stream.readUint8());
    auto x = B(tlvHeader->getChunkLength());
    auto y = B(stream.readUint8());
    ASSERT(x == y);
    tlvHeader->setBoolValue(stream.readUint8());
    return tlvHeader;
}

void TlvHeaderIntSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& tlvHeader = staticPtrCast<const TlvHeaderInt>(chunk);
    stream.writeUint8(tlvHeader->getType());
    stream.writeUint8(B(tlvHeader->getChunkLength()).get());
    stream.writeUint16Be(tlvHeader->getInt16Value());
}

const Ptr<Chunk> TlvHeaderIntSerializer::deserialize(MemoryInputStream& stream) const
{
    auto tlvHeader = makeShared<TlvHeaderInt>();
    ASSERT(tlvHeader->getType() == stream.readUint8());
    ASSERT(B(tlvHeader->getChunkLength()) == B(stream.readUint8()));
    tlvHeader->setInt16Value(stream.readUint16Be());
    return tlvHeader;
}

static void testMutable()
{
    // 1. chunk is mutable after construction
    auto byteCountChunk1 = makeShared<ByteCountChunk>(B(10));
    ASSERT(byteCountChunk1->isMutable());
}

static void testImmutable()
{
    // 1. chunk is immutable after marking it immutable
    auto byteCountChunk1 = makeShared<ByteCountChunk>(B(10));
    byteCountChunk1->markImmutable();
    ASSERT(byteCountChunk1->isImmutable());

    // 2. chunk is not modifiable when it is immutable
    auto byteCountChunk2 = makeImmutableByteCountChunk(B(10));
    ASSERT_ERROR(byteCountChunk2->setLength(B(1)), "chunk is immutable");
    auto bytesChunk1 = makeImmutableBytesChunk(makeVector(10));
    ASSERT_ERROR(bytesChunk1->setByte(1, 0), "chunk is immutable");
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    ASSERT_ERROR(applicationHeader1->setSomeData(0), "chunk is immutable");
}

static void testComplete()
{
    // 1. chunk is complete after construction
    auto byteCountChunk1 = makeShared<ByteCountChunk>(B(10));
    ASSERT(byteCountChunk1->isComplete());
}

static void testIncomplete()
{
    // 1. packet doesn't provide incomplete header if complete is requested but there's not enough data
    Packet packet1;
    packet1.insertAtBack(makeImmutableApplicationHeader(42));
    Packet fragment1;
    fragment1.insertAtBack(packet1.peekAt(B(0), B(5)));
    ASSERT(!fragment1.hasAtFront<ApplicationHeader>());
    ASSERT_ERROR(fragment1.peekAtFront<ApplicationHeader>(), "incomplete chunk is not allowed");

    // 2. packet provides incomplete variable length header if requested
    Packet packet2;
    auto tcpHeader1 = makeShared<TcpHeader>();
    tcpHeader1->setChunkLength(B(16));
    tcpHeader1->setLengthField(16);
    tcpHeader1->setCrcMode(CRC_COMPUTED);
    tcpHeader1->setSrcPort(1000);
    tcpHeader1->setDestPort(1000);
    tcpHeader1->markImmutable();
    packet2.insertAtBack(tcpHeader1);
    const auto& tcpHeader2 = packet2.popAtFront<TcpHeader>(B(4), Chunk::PF_ALLOW_INCOMPLETE);
    ASSERT(tcpHeader2->isIncomplete());
    ASSERT(tcpHeader2->getChunkLength() == B(4));
    ASSERT(tcpHeader2->getCrcMode() == CRC_COMPUTED);
    ASSERT(tcpHeader2->getSrcPort() == 1000);

    // 3. packet provides incomplete variable length serialized header
    Packet packet3;
    auto tcpHeader3 = makeShared<TcpHeader>();
    tcpHeader3->setChunkLength(B(8));
    tcpHeader3->setLengthField(16);
    tcpHeader3->setCrcMode(CRC_COMPUTED);
    tcpHeader3->markImmutable();
    packet3.insertAtBack(tcpHeader3);
    const auto& bytesChunk1 = packet3.peekAllAsBytes();
    ASSERT(bytesChunk1->getChunkLength() == B(8));

    // 4. packet provides incomplete variable length deserialized header
    Packet packet4;
    packet4.insertAtBack(bytesChunk1);
    const auto& tcpHeader4 = packet4.peekAtFront<TcpHeader>(b(-1), Chunk::PF_ALLOW_INCOMPLETE);
    ASSERT(tcpHeader4->isIncomplete());
    ASSERT(tcpHeader4->getChunkLength() == B(8));
    ASSERT(tcpHeader4->getLengthField() == 16);
}

static void testCorrect()
{
    // 1. chunk is correct after construction
    auto byteCountChunk1 = makeShared<ByteCountChunk>(B(10));
    ASSERT(byteCountChunk1->isCorrect());
}

static void testIncorrect()
{
    // 1. chunk is incorrect after marking it incorrect
    auto applicationHeader1 = makeShared<ApplicationHeader>();
    applicationHeader1->markIncorrect();
    ASSERT(applicationHeader1->isIncorrect());
}

static void testProperlyRepresented()
{
    // 1. chunk is proper after construction
    auto byteCountChunk1 = makeShared<ByteCountChunk>(B(10));
    ASSERT(byteCountChunk1->isProperlyRepresented());
}

static void testImproperlyRepresented()
{
    // 1. chunk is improperly represented after deserialization of a non-representable packet
    Packet packet1;
    auto ipHeader1 = makeShared<IpHeader>();
    ipHeader1->markImmutable();
    packet1.insertAtBack(ipHeader1);
    ASSERT(ipHeader1->isProperlyRepresented());
    auto bytesChunk1 = staticPtrCast<BytesChunk>(packet1.peekAllAsBytes()->dupShared());
    bytesChunk1->setByte(0, 42);
    bytesChunk1->markImmutable();
    Packet packet2(nullptr, bytesChunk1);
    const auto& ipHeader2 = packet2.peekAtFront<IpHeader>(b(-1), Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    ASSERT(ipHeader2->isImproperlyRepresented());
}

static void testEmpty()
{
    // 1. peeking an empty packet is an error
    Packet packet1;
    ASSERT_ERROR(packet1.peekAtFront<IpHeader>(), "empty chunk is not allowed");
    ASSERT_ERROR(packet1.peekAtBack<IpHeader>(B(0)), "empty chunk is not allowed");

    // 2. inserting an empty chunk is an error
    Packet packet2;
    ASSERT_ERROR(packet2.insertAtFront(makeShared<ByteCountChunk>(B(0))), "chunk is empty");
    ASSERT_ERROR(packet2.insertAtBack(makeShared<ByteCountChunk>(B(0))), "chunk is empty");
}

static void testHeader()
{
    // 1. packet contains header after chunk is inserted
    Packet packet1;
    packet1.insertAtFront(makeImmutableByteCountChunk(B(10)));
    const auto& chunk1 = packet1.peekAtFront();
    ASSERT(chunk1 != nullptr);
    ASSERT(chunk1->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk1) != nullptr);
    const auto& chunk2 = packet1.peekAtFront<ByteCountChunk>();
    ASSERT(chunk2 != nullptr);
    ASSERT(chunk2->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk2) != nullptr);

    // 2. packet moves header pointer after pop
    const auto& chunk3 = packet1.popAtFront<ByteCountChunk>();
    ASSERT(chunk3 != nullptr);
    ASSERT(chunk3->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk3) != nullptr);
    ASSERT(packet1.getFrontOffset() == B(10));
    packet1.setFrontOffset(B(0));
    packet1.popAtFront(B(10));
    ASSERT(packet1.getFrontOffset() == packet1.getTotalLength());

    // 3. packet provides headers in reverse insertion order
    Packet packet2;
    packet2.insertAtFront(makeImmutableBytesChunk(makeVector(10)));
    packet2.insertAtFront(makeImmutableByteCountChunk(B(10)));
    const auto& chunk4 = packet2.popAtFront<ByteCountChunk>();
    const auto& chunk5 = packet2.popAtFront<BytesChunk>();
    ASSERT(chunk4 != nullptr);
    ASSERT(chunk4->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk4) != nullptr);
    ASSERT(chunk5 != nullptr);
    ASSERT(chunk5->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk5) != nullptr);
    const auto& bytesChunk1 = staticPtrCast<const BytesChunk>(chunk5);
    ASSERT(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(10).begin()));

    // 4. packet provides header from bytes
    Packet packet3;
    auto bytesChunk2 = makeShared<BytesChunk>();
    bytesChunk2->setBytes({2, 4, 0, 42});
    bytesChunk2->markImmutable();
    packet3.insertAtFront(bytesChunk2);
    auto tlvHeader1 = packet3.peekAtFront<TlvHeaderInt>();
    ASSERT(tlvHeader1->getInt16Value() == 42);

    // 5. packet provides mutable headers without duplication if possible
    Packet packet4;
    packet4.insertAtFront(makeImmutableBytesChunk(makeVector(10)));
    const auto& chunk6 = packet4.peekAtFront<BytesChunk>().get();
    const auto& chunk7 = packet4.removeAtFront<BytesChunk>(B(10));
    ASSERT(chunk7.get() == chunk6);
    ASSERT(chunk7->isMutable());
    ASSERT(chunk7->getChunkLength() == B(10));
    ASSERT(packet4.getTotalLength() == B(0));
    const auto& bytesChunk3 = staticPtrCast<const BytesChunk>(chunk7);
    ASSERT(std::equal(bytesChunk3->getBytes().begin(), bytesChunk3->getBytes().end(), makeVector(10).begin()));
}

static void testTrailer()
{
    // 1. packet contains trailer after chunk is inserted
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    const auto& chunk1 = packet1.peekAtBack(B(10));
    ASSERT(chunk1 != nullptr);
    ASSERT(chunk1->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk1) != nullptr);
    const auto& chunk2 = packet1.peekAtBack<ByteCountChunk>(B(10));
    ASSERT(chunk2 != nullptr);
    ASSERT(chunk2->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk2) != nullptr);

    // 2. packet moves trailer pointer after pop
    const auto& chunk3 = packet1.popAtBack<ByteCountChunk>(B(10));
    ASSERT(chunk3 != nullptr);
    ASSERT(chunk3->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk3) != nullptr);
    ASSERT(packet1.getBackOffset() == b(0));
    packet1.setBackOffset(packet1.getTotalLength());
    packet1.popAtBack(B(10));
    ASSERT(packet1.getBackOffset() == b(0));

    // 3. packet provides trailers in reverse order
    Packet packet2;
    packet2.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet2.insertAtBack(makeImmutableByteCountChunk(B(10)));
    const auto& chunk4 = packet2.popAtBack<ByteCountChunk>(B(10));
    const auto& chunk5 = packet2.popAtBack<BytesChunk>(B(10));
    ASSERT(chunk4 != nullptr);
    ASSERT(chunk4->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk4) != nullptr);
    ASSERT(chunk5 != nullptr);
    ASSERT(chunk5->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk5) != nullptr);
    const auto& bytesChunk1 = staticPtrCast<const BytesChunk>(chunk5);
    ASSERT(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(10).begin()));

    // 4. packet provides trailer from bytes but only when length is provided
    Packet packet3;
    auto bytesChunk2 = makeShared<BytesChunk>();
    bytesChunk2->setBytes({2, 4, 0, 42});
    bytesChunk2->markImmutable();
    packet3.insertAtBack(bytesChunk2);
    // TODO: ASSERT_ERROR(packet3.peekAtBack<TlvHeaderInt>(), "isForward()");
    auto tlvTrailer1 = packet3.peekAtBack<TlvHeaderInt>(B(4));
    ASSERT(tlvTrailer1->getInt16Value() == 42);

    // 5. packet provides mutable trailers without duplication if possible
    Packet packet4;
    packet4.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    const auto& chunk6 = packet4.peekAtBack<BytesChunk>(B(10)).get();
    const auto& chunk7 = packet4.removeAtBack<BytesChunk>(B(10));
    ASSERT(chunk7.get() == chunk6);
    ASSERT(chunk7->isMutable());
    ASSERT(chunk7->getChunkLength() == B(10));
    ASSERT(packet4.getTotalLength() == B(0));
    const auto& bytesChunk3 = staticPtrCast<const BytesChunk>(chunk7);
    ASSERT(std::equal(bytesChunk3->getBytes().begin(), bytesChunk3->getBytes().end(), makeVector(10).begin()));
}

static void testFrontPopOffset()
{
    // 1. TODO
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet1.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet1.insertAtBack(makeImmutableApplicationHeader(42));
    packet1.insertAtBack(makeImmutableIpHeader());
    packet1.setFrontOffset(B(0));
    const auto& chunk1 = packet1.peekAtFront();
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk1));
    packet1.setFrontOffset(B(10));
    const auto& chunk2 = packet1.peekAtFront();
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk2));
    packet1.setFrontOffset(B(20));
    const auto& chunk3 = packet1.peekAtFront();
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk3));
    packet1.setFrontOffset(B(30));
    const auto& chunk4 = packet1.peekAtFront();
    ASSERT(dynamicPtrCast<const IpHeader>(chunk4));
    packet1.setFrontOffset(B(50));
    ASSERT_ERROR(packet1.peekAtFront(), "empty chunk is not allowed");
}

static void testBackPopOffset()
{
    // 1. TODO
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet1.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet1.insertAtBack(makeImmutableApplicationHeader(42));
    packet1.insertAtBack(makeImmutableIpHeader());
    packet1.setBackOffset(B(50));
    const auto& chunk1 = packet1.peekAtBack(B(20));
    ASSERT(dynamicPtrCast<const IpHeader>(chunk1));
    packet1.setBackOffset(B(30));
    const auto& chunk2 = packet1.peekAtBack(B(10));
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk2));
    packet1.setBackOffset(B(20));
    const auto& chunk3 = packet1.peekAtBack(B(10));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk3));
    packet1.setBackOffset(B(10));
    const auto& chunk4 = packet1.peekAtBack(B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk4));
    packet1.setBackOffset(B(0));
    ASSERT_ERROR(packet1.peekAtBack(B(0)), "empty chunk is not allowed");
    ASSERT_ERROR(packet1.peekAtBack(B(1)), "length is invalid");
}

static void testUpdate()
{
    // 1. update destructively
    Packet packet1;
    packet1.insertAtFront(makeImmutableApplicationHeader(42));
    packet1.insertAtBack(makeImmutableTcpHeader());
    // 2. update at front
    ASSERT(packet1.peekAtFront<ApplicationHeader>()->getSomeData() == 42);
    packet1.updateAtFront<ApplicationHeader>([] (const Ptr<ApplicationHeader>& applicationHeader) {
        applicationHeader->setSomeData(0);
    });
    ASSERT(packet1.peekAtFront<ApplicationHeader>()->getSomeData() == 0);
    // 3. update at back
    ASSERT(packet1.peekAtBack<TcpHeader>(B(20))->getCrc() == 0);
    packet1.updateAtBack<TcpHeader>([] (const Ptr<TcpHeader>& tcpHeader) {
        tcpHeader->setCrc(42);
    }, B(20));
    ASSERT(packet1.peekAtBack<TcpHeader>(B(20))->getCrc() == 42);
    // 4. udpate copy of the original
    auto& packet2 = *packet1.dup();
    // 5. update at front
    ASSERT(packet2.peekAtFront<ApplicationHeader>()->getSomeData() == 0);
    packet2.updateAtFront<ApplicationHeader>([] (const Ptr<ApplicationHeader>& applicationHeader) {
        applicationHeader->setSomeData(42);
    });
    ASSERT(packet1.peekAtFront<ApplicationHeader>()->getSomeData() == 0);
    ASSERT(packet2.peekAtFront<ApplicationHeader>()->getSomeData() == 42);
    // 6. update at back
    ASSERT(packet2.peekAtBack<TcpHeader>(B(20))->getCrc() == 42);
    packet2.updateAtBack<TcpHeader>([] (const Ptr<TcpHeader>& tcpHeader) {
        tcpHeader->setCrc(0);
    }, B(20));
    ASSERT(packet1.peekAtBack<TcpHeader>(B(20))->getCrc() == 42);
    ASSERT(packet2.peekAtBack<TcpHeader>(B(20))->getCrc() == 0);

    // 7. update in the middle
    Packet packet3;
    packet3.insertAtFront(makeImmutableApplicationHeader(42));
    packet3.insertAtFront(makeImmutableTcpHeader());
    packet3.insertAtFront(makeImmutableIpHeader());
    ASSERT(packet3.peekAt<TcpHeader>(B(20))->getCrc() == 0);
    packet3.updateAt<TcpHeader>([] (const Ptr<TcpHeader>& tcpHeader) {
        tcpHeader->setCrc(42);
    }, B(20));
    ASSERT(packet3.peekAt<TcpHeader>(B(20))->getCrc() == 42);
}

static void testEncapsulation()
{
    // 1. packet contains all chunks of encapsulated packet as is
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet1.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    // encapsulation packet with header and trailer
    auto& packet2 = packet1;
    packet2.insertAtFront(makeImmutableEthernetHeader());
    packet2.insertAtBack(makeImmutableEthernetTrailer());
    const auto& ethernetHeader1 = packet2.popAtFront<EthernetHeader>();
    const auto& ethernetTrailer1 = packet2.popAtBack<EthernetTrailer>(B(2));
    const auto& byteCountChunk1 = packet2.peekDataAt(B(0), B(10));
    const auto& bytesChunk1 = packet2.peekDataAt(B(10), B(10));
    const auto& dataChunk1 = packet2.peekDataAsBytes();
    ASSERT(ethernetHeader1 != nullptr);
    ASSERT(ethernetTrailer1 != nullptr);
    ASSERT(byteCountChunk1 != nullptr);
    ASSERT(bytesChunk1 != nullptr);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(byteCountChunk1) != nullptr);
    ASSERT(dynamicPtrCast<const BytesChunk>(bytesChunk1) != nullptr);
    ASSERT(byteCountChunk1->getChunkLength() == B(10));
    ASSERT(bytesChunk1->getChunkLength() == B(10));
    ASSERT(dataChunk1->getChunkLength() == B(20));
}

static void testAggregation()
{
    // 1. packet contains all chunks of aggregated packets as is
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    Packet packet2;
    packet2.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    Packet packet3;
    packet3.insertAtBack(makeImmutableIpHeader());
    // aggregate other packets
    packet3.insertAtBack(packet1.peekAt(b(0), packet2.getTotalLength()));
    packet3.insertAtBack(packet2.peekAt(b(0), packet2.getTotalLength()));
    const auto& ipHeader1 = packet3.popAtFront<IpHeader>();
    const auto& chunk1 = packet3.peekDataAt(B(0), B(10));
    const auto& chunk2 = packet3.peekDataAt(B(10), B(10));
    ASSERT(ipHeader1 != nullptr);
    ASSERT(chunk1 != nullptr);
    ASSERT(chunk1->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk1) != nullptr);
    ASSERT(chunk2 != nullptr);
    ASSERT(chunk2->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk2) != nullptr);
    const auto& bytesChunk1 = staticPtrCast<const BytesChunk>(chunk2);
    ASSERT(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(10).begin()));
}

static void testFragmentation()
{
    // 1. packet contains fragment of another packet
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet1.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    Packet packet2;
    packet2.insertAtBack(makeImmutableIpHeader());
    // insert fragment of another packet
    packet2.insertAtBack(packet1.peekAt(B(7), B(10)));
    const auto& ipHeader1 = packet2.popAtFront<IpHeader>();
    const auto& fragment1 = packet2.peekDataAt(b(0), packet2.getDataLength());
    const auto& chunk1 = fragment1->peek(B(0), B(3));
    const auto& chunk2 = fragment1->peek(B(3), B(7));
    ASSERT(packet2.getTotalLength() == B(30));
    ASSERT(ipHeader1 != nullptr);
    ASSERT(dynamicPtrCast<const IpHeader>(ipHeader1) != nullptr);
    ASSERT(fragment1 != nullptr);
    ASSERT(fragment1->getChunkLength() == B(10));
    ASSERT(chunk1 != nullptr);
    ASSERT(chunk1->getChunkLength() == B(3));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk1) != nullptr);
    ASSERT(chunk2 != nullptr);
    ASSERT(chunk2->getChunkLength() == B(7));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk2) != nullptr);
    const auto& bytesChunk1 = staticPtrCast<const BytesChunk>(chunk2);
    ASSERT(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(7).begin()));
}

static void testPolymorphism()
{
    // 1. packet provides headers in a polymorphic way without serialization
    Packet packet1;
    auto tlvHeader1 = makeShared<TlvHeaderBool>();
    tlvHeader1->setBoolValue(true);
    tlvHeader1->markImmutable();
    packet1.insertAtBack(tlvHeader1);
    auto tlvHeader2 = makeShared<TlvHeaderInt>();
    tlvHeader2->setInt16Value(42);
    tlvHeader2->markImmutable();
    packet1.insertAtBack(tlvHeader2);
    const auto& tlvHeader3 = packet1.popAtFront<TlvHeader>();
    ASSERT(tlvHeader3 != nullptr);
    ASSERT(tlvHeader3->getChunkLength() == B(3));
    ASSERT(dynamicPtrCast<const TlvHeaderBool>(tlvHeader3) != nullptr);
    const auto & tlvHeaderBool1 = staticPtrCast<const TlvHeaderBool>(tlvHeader3);
    ASSERT(tlvHeaderBool1->getBoolValue());
    const auto& tlvHeader4 = packet1.popAtFront<TlvHeader>();
    ASSERT(tlvHeader4 != nullptr);
    ASSERT(tlvHeader4->getChunkLength() == B(4));
    ASSERT(dynamicPtrCast<const TlvHeaderInt>(tlvHeader4) != nullptr);
    const auto & tlvHeaderInt1 = staticPtrCast<const TlvHeaderInt>(tlvHeader4);
    ASSERT(tlvHeaderInt1->getInt16Value() == 42);

    // 2. packet provides deserialized headers in a polymorphic way after serialization
    Packet packet2(nullptr, packet1.peekAllAsBytes());
    const auto& tlvHeader5 = packet2.popAtFront<TlvHeader>();
    ASSERT(tlvHeader5 != nullptr);
    ASSERT(tlvHeader5->getChunkLength() == B(3));
    ASSERT(dynamicPtrCast<const TlvHeaderBool>(tlvHeader5) != nullptr);
    const auto & tlvHeaderBool2 = staticPtrCast<const TlvHeaderBool>(tlvHeader5);
    ASSERT(tlvHeaderBool2->getBoolValue());
    const auto& tlvHeader6 = packet2.popAtFront<TlvHeader>();
    ASSERT(tlvHeader6 != nullptr);
    ASSERT(tlvHeader6->getChunkLength() == B(4));
    ASSERT(dynamicPtrCast<const TlvHeaderInt>(tlvHeader6) != nullptr);
    const auto & tlvHeaderInt2 = staticPtrCast<const TlvHeaderInt>(tlvHeader6);
    ASSERT(tlvHeaderInt2->getInt16Value() == 42);
}

static void testStreaming()
{
    // 1. bits
    MemoryOutputStream outputStreamBits;
    outputStreamBits.writeBit(true);
    outputStreamBits.writeBitRepeatedly(false, 10);
    std::vector<bool> writeBitsVector = {true, false, true, false, true, false, true, false, true, false};
    outputStreamBits.writeBits(writeBitsVector);
    std::vector<bool> writeBitsData;
    outputStreamBits.copyData(writeBitsData);
    ASSERT(outputStreamBits.getLength() == b(21));
    MemoryInputStream inputStreamBits(outputStreamBits.getData(), outputStreamBits.getLength());
    ASSERT(inputStreamBits.getLength() == b(21));
    ASSERT(inputStreamBits.readBit() == true);
    ASSERT(inputStreamBits.readBitRepeatedly(false, 10));
    std::vector<bool> readBitsVector;
    inputStreamBits.readBits(readBitsVector, b(10));
    ASSERT(std::equal(readBitsVector.begin(), readBitsVector.end(), writeBitsVector.begin()));
    std::vector<bool> readBitsData;
    inputStreamBits.copyData(readBitsData);
    ASSERT(std::equal(readBitsData.begin(), readBitsData.end(), writeBitsData.begin()));
    ASSERT(!inputStreamBits.isReadBeyondEnd());
    ASSERT(inputStreamBits.getRemainingLength() == b(0));
    inputStreamBits.readBit();
    ASSERT(inputStreamBits.isReadBeyondEnd());
    ASSERT(inputStreamBits.getRemainingLength() == B(0));

    // 2. bytes
    MemoryOutputStream outputStreamBytes;
    outputStreamBytes.writeByte(42);
    outputStreamBytes.writeByteRepeatedly(21, 10);
    auto writeBytesVector = makeVector(10);
    outputStreamBytes.writeBytes(writeBytesVector);
    uint8_t writeBytesBuffer[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    outputStreamBytes.writeBytes(writeBytesBuffer, B(10));
    std::vector<bool> writeBytesData;
    outputStreamBytes.copyData(writeBytesData);
    ASSERT(outputStreamBytes.getLength() == B(31));
    MemoryInputStream inputStreamBytes(outputStreamBytes.getData());
    ASSERT(inputStreamBytes.getLength() == B(31));
    ASSERT(inputStreamBytes.readByte() == 42);
    ASSERT(inputStreamBytes.readByteRepeatedly(21, 10));
    std::vector<uint8_t> readBytesVector;
    inputStreamBytes.readBytes(readBytesVector, B(10));
    ASSERT(std::equal(readBytesVector.begin(), readBytesVector.end(), writeBytesVector.begin()));
    uint8_t readBytesBuffer[10];
    inputStreamBytes.readBytes(readBytesBuffer, B(10));
    ASSERT(!memcmp(writeBytesBuffer, readBytesBuffer, 10));
    std::vector<bool> readBytesData;
    inputStreamBytes.copyData(readBytesData);
    ASSERT(std::equal(readBytesData.begin(), readBytesData.end(), writeBytesData.begin()));
    ASSERT(!inputStreamBytes.isReadBeyondEnd());
    ASSERT(inputStreamBytes.getRemainingLength() == B(0));
    inputStreamBytes.readByte();
    ASSERT(inputStreamBytes.isReadBeyondEnd());
    ASSERT(inputStreamBytes.getRemainingLength() == B(0));

    // 3. bit-byte conversion
    MemoryOutputStream outputStreamConversion;
    outputStreamConversion.writeBits({false, false, false, false, true, true, true, true});
    outputStreamConversion.writeBits({true, true, true, true, false, false, false, false});
    MemoryInputStream inputStreamConversion(outputStreamConversion.getData());
    std::vector<uint8_t> data;
    inputStreamConversion.readBytes(data, B(2));
    ASSERT(data[0] == 0x0F);
    ASSERT(data[1] == 0xF0);

    // 4. uint8_t
    uint64_t uint8 = 0x42;
    MemoryOutputStream outputStream1;
    outputStream1.writeUint8(uint8);
    MemoryInputStream inputStream1(outputStream1.getData());
    ASSERT(inputStream1.readUint8() == uint8);
    ASSERT(!inputStream1.isReadBeyondEnd());
    ASSERT(inputStream1.getRemainingLength() == b(0));

    // 5. uint16_t
    uint64_t uint16 = 0x4242;
    MemoryOutputStream outputStream2;
    outputStream2.writeUint16Be(uint16);
    MemoryInputStream inputStream2(outputStream2.getData());
    ASSERT(inputStream2.readUint16Be() == uint16);
    ASSERT(!inputStream2.isReadBeyondEnd());
    ASSERT(inputStream2.getRemainingLength() == b(0));

    // 6. uint32_t
    uint64_t uint32 = 0x42424242;
    MemoryOutputStream outputStream3;
    outputStream3.writeUint32Be(uint32);
    MemoryInputStream inputStream3(outputStream3.getData());
    ASSERT(inputStream3.readUint32Be() == uint32);
    ASSERT(!inputStream3.isReadBeyondEnd());
    ASSERT(inputStream3.getRemainingLength() == b(0));

    // 7. uint64_t
    uint64_t uint64 = 0x4242424242424242L;
    MemoryOutputStream outputStream4;
    outputStream4.writeUint64Be(uint64);
    MemoryInputStream inputStream4(outputStream4.getData());
    ASSERT(inputStream4.readUint64Be() == uint64);
    ASSERT(!inputStream4.isReadBeyondEnd());
    ASSERT(inputStream4.getRemainingLength() == b(0));

    // 8. MacAddress
    MacAddress macAddress("0A:AA:01:02:03:04");
    MemoryOutputStream outputStream5;
    outputStream5.writeMacAddress(macAddress);
    MemoryInputStream inputStream5(outputStream5.getData());
    ASSERT(inputStream5.readMacAddress() ==macAddress);
    ASSERT(!inputStream5.isReadBeyondEnd());
    ASSERT(inputStream5.getRemainingLength() == b(0));

    // 9. Ipv4Address
    Ipv4Address ipv4Address("192.168.10.1");
    MemoryOutputStream outputStream6;
    outputStream6.writeIpv4Address(ipv4Address);
    MemoryInputStream inputStream6(outputStream6.getData());
    ASSERT(inputStream6.readIpv4Address() == ipv4Address);
    ASSERT(!inputStream6.isReadBeyondEnd());
    ASSERT(inputStream6.getRemainingLength() == b(0));

    // 10. Ipv6Address
    Ipv6Address ipv6Address("1011:1213:1415:1617:1819:2021:2223:2425");
    MemoryOutputStream outputStream7;
    outputStream7.writeIpv6Address(ipv6Address);
    MemoryInputStream inputStream7(outputStream7.getData());
    ASSERT(inputStream7.readIpv6Address() == ipv6Address);
    ASSERT(!inputStream7.isReadBeyondEnd());
    ASSERT(inputStream7.getRemainingLength() == b(0));
}

static void testSerialization()
{
    // 1. serialized bytes is cached after serialization
    MemoryOutputStream stream1;
    auto applicationHeader1 = makeShared<ApplicationHeader>();
    auto totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader1);
    auto size = stream1.getLength();
    ASSERT(size != B(0));
    ASSERT(totalSerializedLength + size == ChunkSerializer::totalSerializedLength);
    totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader1);
    ASSERT(stream1.getLength() == size * 2);
    ASSERT(totalSerializedLength == ChunkSerializer::totalSerializedLength);

    // 2. serialized bytes is cached after deserialization
    MemoryInputStream stream2(stream1.getData());
    auto totalDeserializedLength = ChunkSerializer::totalDeserializedLength;
    const auto& chunk1 = Chunk::deserialize(stream2, typeid(ApplicationHeader));
    ASSERT(chunk1 != nullptr);
    ASSERT(B(chunk1->getChunkLength()) == B(size));
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk1) != nullptr);
    auto applicationHeader2 = staticPtrCast<ApplicationHeader>(chunk1);
    ASSERT(totalDeserializedLength + size == ChunkSerializer::totalDeserializedLength);
    totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader2);
    ASSERT(stream1.getLength() == size * 3);
    ASSERT(totalSerializedLength == ChunkSerializer::totalSerializedLength);

    // 3. serialized bytes is deleted after change
    applicationHeader1->setSomeData(42);
    totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader1);
    ASSERT(totalSerializedLength + size == ChunkSerializer::totalSerializedLength);
    applicationHeader2->setSomeData(42);
    totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader2);
    ASSERT(totalSerializedLength + size == ChunkSerializer::totalSerializedLength);
}

static void testConversion()
{
    // 1. implicit non-conversion via serialization is an error by default (would unnecessary slow down simulation)
    Packet packet1;
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    packet1.insertAtBack(applicationHeader1->Chunk::peek<BytesChunk>(B(0), B(5)));
    packet1.insertAtBack(applicationHeader1->Chunk::peek(B(5), B(5)));
    ASSERT_ERROR(packet1.peekAtFront<ApplicationHeader>(B(10)), "serialization is disabled");

    // 2. implicit conversion via serialization is an error by default (would result in hard to debug errors)
    Packet packet2;
    packet2.insertAtBack(makeImmutableIpHeader());
    ASSERT_ERROR(packet2.peekAtFront<ApplicationHeader>(), "serialization is disabled");
}

static void testIteration()
{
    // 1. packet provides inserted chunks
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet1.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet1.insertAtBack(makeImmutableApplicationHeader(42));
    int index1 = 0;
    auto chunk1 = packet1.popAtFront();
    while (chunk1 != nullptr) {
        ASSERT(chunk1 != nullptr);
        ASSERT(chunk1->getChunkLength() == B(10));
        index1++;
        chunk1 = packet1.popAtFront(b(-1), Chunk::PF_ALLOW_NULLPTR);
    }
    ASSERT(index1 == 3);

    // 2. SequenceChunk optimizes forward iteration to indexing
    auto sequenceChunk1 = makeShared<SequenceChunk>();
    sequenceChunk1->insertAtBack(makeImmutableByteCountChunk(B(10)));
    sequenceChunk1->insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    sequenceChunk1->insertAtBack(makeImmutableApplicationHeader(42));
    sequenceChunk1->markImmutable();
    int index2 = 0;
    auto iterator2 = Chunk::ForwardIterator(b(0), 0);
    auto chunk2 = sequenceChunk1->peek(iterator2);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk2) != nullptr);
    while (chunk2 != nullptr) {
        ASSERT(iterator2.getIndex() == index2);
        ASSERT(iterator2.getPosition() == B(index2 * 10));
        ASSERT(chunk2 != nullptr);
        ASSERT(chunk2->getChunkLength() == B(10));
        index2++;
        if (chunk2 != nullptr)
            sequenceChunk1->moveIterator(iterator2, chunk2->getChunkLength());
        chunk2 = sequenceChunk1->peek(iterator2, Chunk::unspecifiedLength, Chunk::PF_ALLOW_NULLPTR);
    }
    ASSERT(index2 == 3);

    // 3. SequenceChunk optimizes backward iteration to indexing
    auto sequenceChunk2 = makeShared<SequenceChunk>();
    sequenceChunk2->insertAtBack(makeImmutableByteCountChunk(B(10)));
    sequenceChunk2->insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    sequenceChunk2->insertAtBack(makeImmutableApplicationHeader(42));
    sequenceChunk2->markImmutable();
    int index3 = 0;
    auto iterator3 = Chunk::BackwardIterator(b(0), 0);
    auto chunk3 = sequenceChunk1->peek(iterator3);
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk3) != nullptr);
    while (chunk3 != nullptr) {
        ASSERT(iterator3.getIndex() == index3);
        ASSERT(iterator3.getPosition() == B(index3 * 10));
        ASSERT(chunk3 != nullptr);
        ASSERT(chunk3->getChunkLength() == B(10));
        index3++;
        if (chunk3 != nullptr)
            sequenceChunk1->moveIterator(iterator3, chunk3->getChunkLength());
        chunk3 = sequenceChunk1->peek(iterator3, Chunk::unspecifiedLength, Chunk::PF_ALLOW_NULLPTR);
    }
    ASSERT(index2 == 3);
}

static void testCorruption()
{
    // 1. data corruption with constant bit error rate
    double random[] = {0.1, 0.7, 0.9};
    double ber = 1E-2;
    Packet packet1;
    const auto& chunk1 = makeImmutableByteCountChunk(B(10));
    const auto& chunk2 = makeImmutableBytesChunk(makeVector(10));
    const auto& chunk3 = makeImmutableApplicationHeader(42);
    packet1.insertAtBack(chunk1);
    packet1.insertAtBack(chunk2);
    packet1.insertAtBack(chunk3);
    int index = 0;
    auto chunk = packet1.popAtFront();
    Packet packet2;
    while (chunk != nullptr) {
        const auto& clone = chunk->dupShared();
        b length = chunk->getChunkLength();
        if (random[index++] >= std::pow(1 - ber, length.get()))
            clone->markIncorrect();
        clone->markImmutable();
        packet2.insertAtBack(clone);
        chunk = packet1.popAtFront(b(-1), Chunk::PF_ALLOW_NULLPTR);
    }
    ASSERT(packet2.popAtFront(b(-1), Chunk::PF_ALLOW_INCORRECT)->isCorrect());
    ASSERT(packet2.popAtFront(b(-1), Chunk::PF_ALLOW_INCORRECT)->isIncorrect());
    ASSERT(packet2.popAtFront(b(-1), Chunk::PF_ALLOW_INCORRECT)->isIncorrect());
}

static void testDuplication()
{
    // 1. copy of immutable packet shares chunk
    Packet packet1;
    const Ptr<ByteCountChunk> byteCountChunk1 = makeImmutableByteCountChunk(B(10));
    packet1.insertAtBack(byteCountChunk1);
    auto packet2 = packet1.dup();
    ASSERT(packet2->getTotalLength() == B(10));
    ASSERT(byteCountChunk1.use_count() == 3); // 1 here + 2 in the packets
    delete packet2;
}

static void testDuality()
{
    // 1. packet provides header in both fields and bytes representation
    Packet packet1;
    packet1.insertAtBack(makeImmutableApplicationHeader(42));
    const auto& applicationHeader1 = packet1.peekAtFront<ApplicationHeader>();
    const auto& bytesChunk1 = packet1.peekAtFront<BytesChunk>(B(10));
    ASSERT(applicationHeader1 != nullptr);
    ASSERT(applicationHeader1->getChunkLength() == B(10));
    ASSERT(bytesChunk1 != nullptr);
    ASSERT(bytesChunk1->getChunkLength() == B(10));

    // 2. packet provides header in both fields and bytes representation after serialization
    Packet packet2(nullptr, packet1.peekAllAsBytes());
    const auto& applicationHeader2 = packet2.peekAtFront<ApplicationHeader>();
    const auto& bytesChunk2 = packet2.peekAtFront<BytesChunk>(B(10));
    ASSERT(applicationHeader2 != nullptr);
    ASSERT(applicationHeader2->getChunkLength() == B(10));
    ASSERT(bytesChunk2 != nullptr);
    ASSERT(bytesChunk2->getChunkLength() == B(10));
    ASSERT(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), bytesChunk1->getBytes().begin()));
    ASSERT(applicationHeader2->getSomeData() == applicationHeader2->getSomeData());
}

static void testMerging()
{
    // 1. packet provides complete merged header if the whole header is available
    Packet packet1;
    packet1.insertAtBack(makeImmutableApplicationHeader(42));
    Packet packet2;
    packet2.insertAtBack(packet1.peekAt(B(0), B(5)));
    packet2.insertAtBack(packet1.peekAt(B(5), B(5)));
    const auto& chunk1 = packet2.peekAtFront();
    ASSERT(chunk1 != nullptr);
    ASSERT(chunk1->isComplete());
    ASSERT(chunk1->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk1) != nullptr);
    const auto& chunk2 = packet2.peekAtFront<ApplicationHeader>();
    ASSERT(chunk2->isComplete());
    ASSERT(chunk2->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk2) != nullptr);

    // 2. packet provides compacts ByteCountChunks
    Packet packet3;
    packet3.insertAtBack(makeImmutableByteCountChunk(B(5)));
    packet3.insertAtBack(makeImmutableByteCountChunk(B(5)));
    const auto& chunk3 = packet3.peekAt(b(0), packet3.getTotalLength());
    const auto& chunk4 = packet3.peekAt<ByteCountChunk>(b(0), packet3.getTotalLength());
    ASSERT(chunk3 != nullptr);
    ASSERT(chunk3->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk3) != nullptr);
    ASSERT(chunk4 != nullptr);
    ASSERT(chunk4->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk4) != nullptr);

    // 2. packet provides compacts ByteChunks
    Packet packet4;
    packet4.insertAtBack(makeImmutableBytesChunk(makeVector(5)));
    packet4.insertAtBack(makeImmutableBytesChunk(makeVector(5)));
    const auto& chunk5 = packet4.peekAt(b(0), packet4.getTotalLength());
    const auto& chunk6 = packet4.peekAllAsBytes();
    ASSERT(chunk5 != nullptr);
    ASSERT(chunk5->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk5) != nullptr);
    const auto& bytesChunk1 = staticPtrCast<const BytesChunk>(chunk5);
    ASSERT(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 0, 1, 2, 3, 4}).begin()));
    ASSERT(chunk6 != nullptr);
    ASSERT(chunk6->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk6) != nullptr);
    const auto& bytesChunk2 = staticPtrCast<const BytesChunk>(chunk6);
    ASSERT(std::equal(bytesChunk2->getBytes().begin(), bytesChunk2->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 0, 1, 2, 3, 4}).begin()));
}

static void testSlicing()
{
    // 1. ByteCountChunk always returns ByteCountChunk
    auto byteCountChunk1 = makeImmutableByteCountChunk(B(10));
    const auto& chunk1 = byteCountChunk1->peek(B(0), B(10));
    const auto& chunk2 = byteCountChunk1->peek(B(0), B(5));
    ASSERT(chunk1 == byteCountChunk1);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk1) != nullptr);
    ASSERT(chunk2 != nullptr);
    ASSERT(chunk2->getChunkLength() == B(5));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk2) != nullptr);

    // 2. BytesChunk returns BytesChunk
    auto bytesChunk1 = makeImmutableBytesChunk(makeVector(10));
    const auto& chunk3 = bytesChunk1->peek(B(0), B(10));
    const auto& chunk4 = bytesChunk1->peek(B(0), B(5));
    ASSERT(chunk3 != nullptr);
    ASSERT(chunk3->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk3) != nullptr);
    const auto& bytesChunk2 = staticPtrCast<const BytesChunk>(chunk3);
    ASSERT(std::equal(bytesChunk2->getBytes().begin(), bytesChunk2->getBytes().end(), makeVector(10).begin()));
    ASSERT(chunk4 != nullptr);
    ASSERT(chunk4->getChunkLength() == B(5));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk4) != nullptr);
    const auto& bytesChunk3 = staticPtrCast<const BytesChunk>(chunk4);
    ASSERT(std::equal(bytesChunk3->getBytes().begin(), bytesChunk3->getBytes().end(), makeVector(5).begin()));

    // 2b. BytesChunk sometimes returns SliceChunk because the position or offset is not aligned to byte boundary
    auto bytesChunk2b = makeImmutableBytesChunk(makeVector(10));
    const auto& chunk2b1 = bytesChunk2b->peek(B(0), b(20));
    ASSERT(chunk2b1 != nullptr);
    ASSERT(chunk2b1->getChunkLength() == b(20));
    ASSERT(dynamicPtrCast<const SliceChunk>(chunk2b1) != nullptr);
    const auto& chunk2b2 = bytesChunk2b->peek(b(20), b(60));
    ASSERT(chunk2b2 != nullptr);
    ASSERT(chunk2b2->getChunkLength() == b(60));
    ASSERT(dynamicPtrCast<const SliceChunk>(chunk2b2) != nullptr);

    // 3a. SliceChunk returns a SliceChunk containing the requested slice of the chunk that is used by the original SliceChunk
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    auto sliceChunk1 = makeShared<SliceChunk>(applicationHeader1, b(0), B(10));
    sliceChunk1->markImmutable();
    const auto& chunk5 = sliceChunk1->peek(B(5), B(5));
    ASSERT(chunk5 != nullptr);
    ASSERT(chunk5->getChunkLength() == B(5));
    ASSERT(dynamicPtrCast<const SliceChunk>(chunk5) != nullptr);
    auto sliceChunk2 = staticPtrCast<const SliceChunk>(chunk5);
    ASSERT(sliceChunk2->getChunk() == sliceChunk1->getChunk());
    ASSERT(sliceChunk2->getOffset() == B(5));
    ASSERT(sliceChunk2->getLength() == B(5));

    // 4a. SequenceChunk may return an element chunk
    auto sequenceChunk1 = makeShared<SequenceChunk>();
    sequenceChunk1->insertAtBack(byteCountChunk1);
    sequenceChunk1->insertAtBack(bytesChunk1);
    sequenceChunk1->insertAtBack(applicationHeader1);
    sequenceChunk1->markImmutable();
    const auto& chunk6 = sequenceChunk1->peek(B(0), B(10));
    const auto& chunk7 = sequenceChunk1->peek(B(10), B(10));
    const auto& chunk8 = sequenceChunk1->peek(B(20), B(10));
    ASSERT(chunk6 != nullptr);
    ASSERT(chunk6->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk6) != nullptr);
    ASSERT(chunk7 != nullptr);
    ASSERT(chunk7->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk7) != nullptr);
    ASSERT(chunk8 != nullptr);
    ASSERT(chunk8->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk8) != nullptr);

    // 4b. SequenceChunk may return a (simplified) SliceChunk of an element chunk
    const auto& chunk9 = sequenceChunk1->peek(B(0), B(5));
    const auto& chunk10 = sequenceChunk1->peek(B(15), B(5));
    const auto& chunk11 = sequenceChunk1->peek(B(20), B(5));
    ASSERT(chunk9 != nullptr);
    ASSERT(chunk9->getChunkLength() == B(5));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk9) != nullptr);
    ASSERT(chunk10 != nullptr);
    ASSERT(chunk10->getChunkLength() == B(5));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk10) != nullptr);
    ASSERT(chunk11 != nullptr);
    ASSERT(chunk11->getChunkLength() == B(5));
    ASSERT(dynamicPtrCast<const SliceChunk>(chunk11) != nullptr);

    // 4c. SequenceChunk may return a new SequenceChunk
    const auto& chunk12 = sequenceChunk1->peek(B(5), B(10));
    ASSERT(chunk12 != nullptr);
    ASSERT(chunk12->getChunkLength() == B(10));
    ASSERT(dynamicPtrCast<const SequenceChunk>(chunk12) != nullptr);
    const auto& sequenceChunk2 = staticPtrCast<const SequenceChunk>(chunk12);
    ASSERT(sequenceChunk1 != sequenceChunk2);
    ASSERT(sequenceChunk2->getChunks().size() == 2);

    // 5. any other chunk returns a SliceChunk
    auto applicationHeader2 = makeImmutableApplicationHeader(42);
    const auto& chunk13 = applicationHeader2->peek(B(0), B(5));
    ASSERT(chunk13 != nullptr);
    ASSERT(chunk13->getChunkLength() == B(5));
    ASSERT(dynamicPtrCast<const SliceChunk>(chunk13) != nullptr);
    const auto& sliceChunk4 = dynamicPtrCast<const SliceChunk>(chunk13);
    ASSERT(sliceChunk4->getChunk() == applicationHeader2);
    ASSERT(sliceChunk4->getOffset() == b(0));
    ASSERT(sliceChunk4->getLength() == B(5));
}

static void testNesting()
{
    // 1. packet contains compound header as is
    Packet packet1;
    auto ipHeader1 = makeShared<IpHeader>();
    ipHeader1->setProtocol(Protocol::Tcp);
    ipHeader1->markImmutable();
    auto compoundHeader1 = makeShared<CompoundHeader>();
    compoundHeader1->insertAtBack(ipHeader1);
    compoundHeader1->markImmutable();
    packet1.insertAtBack(compoundHeader1);
    const auto& compoundHeader2 = packet1.peekAtFront<CompoundHeader>(compoundHeader1->getChunkLength());
    ASSERT(compoundHeader2 != nullptr);

    // 2. packet provides compound header after serialization
    Packet packet2(nullptr, packet1.peekAllAsBytes());
    const auto& compoundHeader3 = packet2.peekAtFront<CompoundHeader>();
    ASSERT(compoundHeader3 != nullptr);
    auto it = Chunk::ForwardIterator(b(0), 0);
    const auto& ipHeader2 = compoundHeader3->Chunk::peek<IpHeader>(it);
    ASSERT(ipHeader2 != nullptr);
    ASSERT(ipHeader2->getProtocol() == Protocol::Tcp);
}

static void testPeeking()
{
    // 1. packet provides ByteCountChunks by default if it contains a ByteCountChunk only
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet1.insertAtBack(makeImmutableByteCountChunk(B(10)));
    const auto& chunk1 = packet1.popAtFront(B(15));
    const auto& chunk2 = packet1.popAtFront(B(15));
    ASSERT(chunk1 != nullptr);
    ASSERT(chunk1->getChunkLength() == B(15));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk1) != nullptr);
    ASSERT(chunk2 != nullptr);
    ASSERT(chunk2->getChunkLength() == B(15));
    ASSERT(dynamicPtrCast<const ByteCountChunk>(chunk2) != nullptr);

    // 2. packet provides BytesChunks by default if it contains a BytesChunk only
    Packet packet2;
    packet2.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet2.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet2.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    const auto& chunk3 = packet2.popAtFront(B(15));
    const auto& chunk4 = packet2.popAtFront(B(15));
    ASSERT(chunk3 != nullptr);
    ASSERT(chunk3->getChunkLength() == B(15));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk3) != nullptr);
    ASSERT(chunk4 != nullptr);
    ASSERT(chunk4->getChunkLength() == B(15));
    ASSERT(dynamicPtrCast<const BytesChunk>(chunk4) != nullptr);
}

static void testSequenceSerialization()
{
    { // 1. test sequence doesn't serializes front if not necessary
    Packet packet;
    auto header = makeShared<HeaderWithoutSerializer>();
    packet.insertAtFront(header);
    packet.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet.popAtFront<HeaderWithoutSerializer>();
    const auto& bytesChunk = packet.peekDataAsBytes();
    ASSERT(bytesChunk != nullptr);
    ASSERT(bytesChunk->getChunkLength() == B(20));
    }

    { // 2. test sequence doesn't serializes back if not necessary
    Packet packet;
    auto header = makeShared<HeaderWithoutSerializer>();
    packet.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet.insertAtBack(makeImmutableByteCountChunk(B(10)));
    packet.insertAtBack(header);
    packet.popAtBack<HeaderWithoutSerializer>(B(8));
    const auto& bytesChunk = packet.peekDataAsBytes();
    ASSERT(bytesChunk != nullptr);
    ASSERT(bytesChunk->getChunkLength() == B(20));
    }

    { // 3. test sequence serialization fails if no serializer is present
    Packet packet;
    auto header = makeShared<HeaderWithoutSerializer>();
    packet.insertAtBack(makeImmutableBytesChunk(makeVector(10)));
    packet.insertAtBack(header);
    packet.insertAtBack(makeImmutableByteCountChunk(B(10)));
    const auto& dataChunk = packet.peekData();
    ASSERT(dataChunk != nullptr);
    ASSERT(dataChunk->getChunkLength() == B(28));
    ASSERT_ERROR(packet.peekDataAsBytes(), "Cannot find serializer");
    }
}

static void testSequence()
{
    // 1. sequence merges immutable slices
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    auto sequenceChunk1 = makeShared<SequenceChunk>();
    sequenceChunk1->insertAtBack(applicationHeader1->peek(B(0), B(5)));
    sequenceChunk1->insertAtBack(applicationHeader1->peek(B(5), B(5)));
    const auto& chunk1 = sequenceChunk1->peek(b(0));
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk1) != nullptr);

    // 2. sequence merges mutable slices
    auto sequenceChunk2 = makeShared<SequenceChunk>();
    const auto& sliceChunk1 = makeShared<SliceChunk>(applicationHeader1, B(0), B(5));
    sliceChunk1->markImmutable();
    sequenceChunk2->insertAtBack(sliceChunk1);
    const auto& sliceChunk2 = makeShared<SliceChunk>(applicationHeader1, B(5), B(5));
    sliceChunk2->markImmutable();
    sequenceChunk2->insertAtBack(sliceChunk2);
    const auto& chunk2 = sequenceChunk2->peek(b(0));
    ASSERT(dynamicPtrCast<const ApplicationHeader>(chunk2) != nullptr);
}

static void testChunkQueue()
{
    // 1. queue provides ByteCountChunks by default if it contains a ByteCountChunk only
    ChunkQueue queue1;
    auto byteCountChunk1 = makeImmutableByteCountChunk(B(10));
    queue1.push(byteCountChunk1);
    queue1.push(byteCountChunk1);
    queue1.push(byteCountChunk1);
    const auto& byteCountChunk2 = dynamicPtrCast<const ByteCountChunk>(queue1.pop(B(15)));
    const auto& byteCountChunk3 = dynamicPtrCast<const ByteCountChunk>(queue1.pop(B(15)));
    ASSERT(byteCountChunk2 != nullptr);
    ASSERT(byteCountChunk3 != nullptr);

    // 2. queue provides BytesChunks by default if it contains a BytesChunk only
    ChunkQueue queue2;
    auto bytesChunk1 = makeImmutableBytesChunk(makeVector(10));
    queue2.push(bytesChunk1);
    queue2.push(bytesChunk1);
    queue2.push(bytesChunk1);
    const auto& bytesChunk2 = dynamicPtrCast<const BytesChunk>(queue2.pop(B(15)));
    const auto& bytesChunk3 = dynamicPtrCast<const BytesChunk>(queue2.pop(B(15)));
    ASSERT(bytesChunk2 != nullptr);
    ASSERT(bytesChunk3 != nullptr);

    // 3. queue provides reassembled header
    ChunkQueue queue3;
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    queue3.push(applicationHeader1->peek(B(0), B(5)));
    queue3.push(applicationHeader1->peek(B(5), B(5)));
    ASSERT(queue3.has<ApplicationHeader>());
    const auto& applicationHeader2 = queue3.pop<ApplicationHeader>();
    ASSERT(applicationHeader2 != nullptr);
    ASSERT(applicationHeader2->getSomeData() == 42);
}

static void testChunkBuffer(cRNG *rng)
{
    // 1. single chunk
    ChunkBuffer buffer1;
    auto byteCountChunk1 = makeImmutableByteCountChunk(B(10));
    buffer1.replace(b(0), byteCountChunk1);
    ASSERT(buffer1.getNumRegions() == 1);
    ASSERT(buffer1.getRegionData(0) != nullptr);

    // 2. consecutive chunks
    ChunkBuffer buffer2;
    buffer2.replace(B(0), byteCountChunk1);
    buffer2.replace(B(10), byteCountChunk1);
    const auto& byteCountChunk2 = dynamicPtrCast<const ByteCountChunk>(buffer2.getRegionData(0));
    ASSERT(buffer2.getNumRegions() == 1);
    ASSERT(byteCountChunk2 != nullptr);
    ASSERT(byteCountChunk2->getChunkLength() == B(20));

    // 3. consecutive slice chunks
    ChunkBuffer buffer3;
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    buffer3.replace(B(0), applicationHeader1->peek(B(0), B(5)));
    buffer3.replace(B(5), applicationHeader1->peek(B(5), B(5)));
    const auto& applicationHeader2 = dynamicPtrCast<const ApplicationHeader>(buffer3.getRegionData(0));
    ASSERT(buffer3.getNumRegions() == 1);
    ASSERT(applicationHeader2 != nullptr);
    ASSERT(applicationHeader2->getSomeData() == 42);

    // 4. out of order consecutive chunks
    ChunkBuffer buffer4;
    buffer4.replace(B(0), byteCountChunk1);
    buffer4.replace(B(20), byteCountChunk1);
    buffer4.replace(B(10), byteCountChunk1);
    const auto& byteCountChunk3 = dynamicPtrCast<const ByteCountChunk>(buffer4.getRegionData(0));
    ASSERT(buffer4.getNumRegions() == 1);
    ASSERT(byteCountChunk3 != nullptr);
    ASSERT(byteCountChunk3->getChunkLength() == B(30));

    // 5. out of order consecutive slice chunks
    ChunkBuffer buffer5;
    buffer5.replace(B(0), applicationHeader1->peek(B(0), B(3)));
    buffer5.replace(B(7), applicationHeader1->peek(B(7), B(3)));
    buffer5.replace(B(3), applicationHeader1->peek(B(3), B(4)));
    const auto& applicationHeader3 = dynamicPtrCast<const ApplicationHeader>(buffer5.getRegionData(0));
    ASSERT(buffer5.getNumRegions() == 1);
    ASSERT(applicationHeader3 != nullptr);
    ASSERT(applicationHeader3->getSomeData() == 42);

    // 6. heterogeneous chunks
    ChunkBuffer buffer6;
    auto byteArrayChunk1 = makeImmutableBytesChunk(makeVector(10));
    buffer6.replace(B(0), byteCountChunk1);
    buffer6.replace(B(10), byteArrayChunk1);
    ASSERT(buffer6.getNumRegions() == 1);
    ASSERT(buffer6.getRegionData(0) != nullptr);

    // 7. completely overwriting a chunk
    ChunkBuffer buffer7;
    auto byteCountChunk4 = makeImmutableByteCountChunk(B(8));
    buffer7.replace(B(1), byteCountChunk4);
    buffer7.replace(B(0), byteArrayChunk1);
    const auto& bytesChunk1 = dynamicPtrCast<const BytesChunk>(buffer7.getRegionData(0));
    ASSERT(buffer7.getNumRegions() == 1);
    ASSERT(bytesChunk1 != nullptr);

    // 8. partially overwriting multiple chunks
    ChunkBuffer buffer8;
    buffer8.replace(B(0), byteCountChunk1);
    buffer8.replace(B(10), byteCountChunk1);
    buffer8.replace(B(3), byteArrayChunk1);
    ASSERT(buffer8.getNumRegions() == 1);
    const auto& sequenceChunk1 = dynamicPtrCast<const SequenceChunk>(buffer8.getRegionData(0));
    ASSERT(sequenceChunk1 != nullptr);
    const auto& byteCountChunk5 = dynamicPtrCast<const ByteCountChunk>(sequenceChunk1->peek(B(0), B(3)));
    ASSERT(byteCountChunk5 != nullptr);
    ASSERT(byteCountChunk5->getChunkLength() == B(3));
    const auto& byteCountChunk6 = dynamicPtrCast<const ByteCountChunk>(sequenceChunk1->peek(B(13), B(7)));
    ASSERT(byteCountChunk6 != nullptr);
    ASSERT(byteCountChunk6->getChunkLength() == B(7));
    const auto& bytesChunk2 = dynamicPtrCast<const BytesChunk>(sequenceChunk1->peek(B(3), B(10)));
    ASSERT(bytesChunk2 != nullptr);
    ASSERT(std::equal(bytesChunk2->getBytes().begin(), bytesChunk2->getBytes().end(), makeVector(10).begin()));

    // 9. random test
    bool debug = false;
    B bufferSize = B(1000);
    B maxChunkLength = B(100);
    ChunkBuffer buffer9;
    int *buffer10 = new int[bufferSize.get()];
    memset(buffer10, -1, bufferSize.get() * sizeof(int));
    for (int c = 0; c < 1000; c++) {
        // replace data
        B chunkOffset = B(rng->intRand((bufferSize - maxChunkLength).get()));
        B chunkLength = B(rng->intRand(maxChunkLength.get()) + 1);
        auto chunk = makeShared<BytesChunk>();
        std::vector<uint8_t> bytes;
        for (B i = B(0); i < chunkLength; i++)
            bytes.push_back(i.get() & 0xFF);
        chunk->setBytes(bytes);
        chunk->markImmutable();
        if (debug)
            std::cout << "Replace " << c << ": offset = " << chunkOffset << ", chunk = " << chunk << std::endl;
        buffer9.replace(B(chunkOffset), chunk);
        for (B i = B(0); i < chunkLength; i++)
            *(buffer10 + chunkOffset.get() + i.get()) = i.get() & 0xFF;

        // clear data
        chunkOffset = B(rng->intRand((bufferSize - maxChunkLength).get()));
        chunkLength = B(rng->intRand(maxChunkLength.get()) + 1);
        buffer9.clear(B(chunkOffset), chunkLength);
        for (B i = B(0); i < chunkLength; i++)
            *(buffer10 + chunkOffset.get() + i.get()) = -1;

        // compare data
        if (debug) {
            std::cout << "ChunkBuffer: " << buffer9 << std::endl;
            std::cout << "PlainBuffer: ";
            for (B i = B(0); i < bufferSize; i++)
                printf("%d", *(buffer10 + i.get()));
            std::cout << std::endl << std::endl;
        }
        B previousEndOffset = B(0);
        for (int i = 0; i < buffer9.getNumRegions(); i++) {
            auto data = dynamicPtrCast<const BytesChunk>(buffer9.getRegionData(i));
            auto startOffset = B(buffer9.getRegionStartOffset(i));
            for (B j = previousEndOffset; j < startOffset; j++)
                ASSERT(*(buffer10 + j.get()) == -1);
            for (B j = B(0); j < B(data->getChunkLength()); j++)
                ASSERT(data->getByte(j.get()) == *(buffer10 + startOffset.get() + j.get()));
            previousEndOffset = startOffset + data->getChunkLength();
        }
        for (B j = previousEndOffset; j < bufferSize; j++)
            ASSERT(*(buffer10 + j.get()) == -1);
    }
    delete [] buffer10;
}

static void testReassemblyBuffer()
{
    // 1. single chunk
    ReassemblyBuffer buffer1(B(10));
    auto byteCountChunk1 = makeImmutableByteCountChunk(B(10));
    buffer1.replace(b(0), byteCountChunk1);
    ASSERT(buffer1.isComplete());
    const auto& data1 = buffer1.getReassembledData();
    ASSERT(data1 != nullptr);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(data1) != nullptr);
    ASSERT(data1->getChunkLength() == B(10));

    // 2. consecutive chunks
    ReassemblyBuffer buffer2(B(20));
    buffer2.replace(b(0), byteCountChunk1);
    ASSERT(!buffer2.isComplete());
    buffer2.replace(B(10), byteCountChunk1);
    ASSERT(buffer2.isComplete());
    const auto& data2 = buffer2.getReassembledData();
    ASSERT(data2 != nullptr);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(data2) != nullptr);
    ASSERT(data2->getChunkLength() == B(20));

    // 3. out of order consecutive chunks
    ReassemblyBuffer buffer3(B(30));
    buffer3.replace(b(0), byteCountChunk1);
    ASSERT(!buffer3.isComplete());
    buffer3.replace(B(20), byteCountChunk1);
    ASSERT(!buffer3.isComplete());
    buffer3.replace(B(10), byteCountChunk1);
    ASSERT(buffer3.isComplete());
    const auto& data3 = buffer3.getReassembledData();
    ASSERT(data3 != nullptr);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(data3) != nullptr);
    ASSERT(data3->getChunkLength() == B(30));
}

static void testReorderBuffer()
{
    // 1. single chunk
    ReorderBuffer buffer1(B(1000));
    auto byteCountChunk1 = makeImmutableByteCountChunk(B(10));
    buffer1.replace(B(1000), byteCountChunk1);
    const auto& data1 = buffer1.popAvailableData();
    ASSERT(data1 != nullptr);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(data1) != nullptr);
    ASSERT(data1->getChunkLength() == B(10));
    ASSERT(buffer1.getExpectedOffset() == B(1010));

    // 2. consecutive chunks
    ReorderBuffer buffer2(B(1000));
    buffer2.replace(B(1000), byteCountChunk1);
    buffer2.replace(B(1010), byteCountChunk1);
    const auto& data2 = buffer2.popAvailableData();
    ASSERT(data2 != nullptr);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(data2) != nullptr);
    ASSERT(data2->getChunkLength() == B(20));
    ASSERT(buffer2.getExpectedOffset() == B(1020));

    // 3. out of order consecutive chunks
    ReorderBuffer buffer3(B(1000));
    buffer3.replace(B(1020), byteCountChunk1);
    ASSERT(buffer2.popAvailableData() == nullptr);
    buffer3.replace(B(1000), byteCountChunk1);
    buffer3.replace(B(1010), byteCountChunk1);
    const auto& data3 = buffer3.popAvailableData();
    ASSERT(data3 != nullptr);
    ASSERT(dynamicPtrCast<const ByteCountChunk>(data3) != nullptr);
    ASSERT(data3->getChunkLength() == B(30));
    ASSERT(buffer3.getExpectedOffset() == B(1030));
}

static void testTagSet()
{
    // 1. getNumTags
    TagSet tagSet1;
    ASSERT(tagSet1.getNumTags() == 0);
    tagSet1.addTag<CreationTimeTag>();
    ASSERT(tagSet1.getNumTags() == 1);
    tagSet1.removeTag<CreationTimeTag>();
    ASSERT(tagSet1.getNumTags() == 0);

    // 2. getTag
    TagSet tagSet2;
    const auto& tag1 = tagSet2.addTag<CreationTimeTag>();
    const auto& tag2 = tagSet2.getTag(0);
    ASSERT(tag2 != nullptr);
    ASSERT(tag2 == tag1);

    // 3. clearTags
    TagSet tagSet3;
    tagSet3.clearTags();
    ASSERT(tagSet3.getNumTags() == 0);
    tagSet3.addTag<CreationTimeTag>();
    tagSet3.clearTags();
    ASSERT(tagSet3.getNumTags() == 0);

    // 4. findTag
    TagSet tagSet4;
    ASSERT(tagSet4.findTag<CreationTimeTag>() == nullptr);
    const auto& tag3 = tagSet4.addTag<CreationTimeTag>();
    const auto& tag4 = tagSet4.findTag<CreationTimeTag>();
    ASSERT(tag4 != nullptr);
    ASSERT(tag4 == tag3);
    tagSet4.removeTag<CreationTimeTag>();
    ASSERT(tagSet4.findTag<CreationTimeTag>() == nullptr);

    // 5. getTag
    TagSet tagSet5;
    ASSERT_ERROR(tagSet5.getTag<CreationTimeTag>(), "is absent");
    const auto& tag5 = tagSet5.addTag<CreationTimeTag>();
    const auto& tag6 = tagSet5.getTag<CreationTimeTag>();
    ASSERT(tag6 != nullptr);
    ASSERT(tag6 == tag5);
    tagSet5.removeTag<CreationTimeTag>();
    ASSERT_ERROR(tagSet5.getTag<CreationTimeTag>(), "is absent");

    // 6. addTag
    TagSet tagSet6;
    const auto& tag7 = tagSet6.addTag<CreationTimeTag>();
    ASSERT(tag7 != nullptr);
    ASSERT(tagSet6.getNumTags() == 1);
    ASSERT_ERROR(tagSet6.addTag<CreationTimeTag>(), "is present");

    // 7. addTagIfAbsent
    TagSet tagSet7;
    const auto& tag8 = tagSet7.addTagIfAbsent<CreationTimeTag>().get();
    const auto& tag9 = tagSet7.addTagIfAbsent<CreationTimeTag>().get();
    ASSERT(tag9 != nullptr);
    ASSERT(tag9 == tag8);
    ASSERT(tagSet7.getNumTags() == 1);

    // 8. removeTag
    TagSet tagSet8;
    ASSERT_ERROR(tagSet8.removeTag<CreationTimeTag>(), "is absent");
    const auto& tag10 = tagSet8.addTag<CreationTimeTag>().get();
    const auto& tag11 = tagSet8.removeTag<CreationTimeTag>().get();
    ASSERT(tag11 != nullptr);
    ASSERT(tag11 == tag10);
    ASSERT(tagSet8.getNumTags() == 0);

    // 9. removeTagIfPresent
    TagSet tagSet9;
    tagSet9.removeTagIfPresent<CreationTimeTag>();
    const auto& tag12 = tagSet9.addTag<CreationTimeTag>().get();
    const auto& tag13 = tagSet9.removeTagIfPresent<CreationTimeTag>().get();
    ASSERT(tag13 != nullptr);
    ASSERT(tag13 == tag12);
    ASSERT(tagSet9.getNumTags() == 0);
    ASSERT(tagSet9.removeTagIfPresent<CreationTimeTag>() == nullptr);

    // 10. copyTags
    TagSet tagSet10;
    TagSet tagSet11;
    tagSet11.copyTags(tagSet10);
    ASSERT(tagSet11.getNumTags() == 0);
    tagSet10.addTag<CreationTimeTag>();
    tagSet11.copyTags(tagSet10);
    ASSERT(tagSet11.getNumTags() == 1);
}

static void testRegionTagSet()
{
    { // 1. getNumTags
    RegionTagSet regionTagSet;
    ASSERT(regionTagSet.getNumTags() == 0);
    regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    ASSERT(regionTagSet.getNumTags() == 1);
    regionTagSet.removeTag<CreationTimeTag>(b(0), b(1000));
    ASSERT(regionTagSet.getNumTags() == 0);
    }

    { // 2. getTag
    RegionTagSet regionTagSet;
    const auto& tag1 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000)).get();
    const auto& tag2 = regionTagSet.getTag(0).get();
    ASSERT(tag2 != nullptr);
    ASSERT(tag2 == tag1);
    }

    { // 3. clearTags
    RegionTagSet regionTagSet;
    regionTagSet.clearTags(b(0), b(1000));
    ASSERT(regionTagSet.getNumTags() == 0);
    regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    regionTagSet.clearTags(b(0), b(1000));
    ASSERT(regionTagSet.getNumTags() == 0);
    }

    { // 4. findTag
    RegionTagSet regionTagSet;
    ASSERT(regionTagSet.findTag<CreationTimeTag>(b(0), b(1000)) == nullptr);
    const auto& tag1 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    const auto& tag2 = regionTagSet.findTag<CreationTimeTag>(b(0), b(1000));
    ASSERT(tag2 != nullptr);
    ASSERT(tag2 == tag1);
    regionTagSet.removeTag<CreationTimeTag>(b(0), b(1000));
    ASSERT(regionTagSet.findTag<CreationTimeTag>(b(0), b(1000)) == nullptr);
    }

    { // 5. getTag
    RegionTagSet regionTagSet;
    ASSERT_ERROR(regionTagSet.getTag<CreationTimeTag>(b(0), b(1000)), "is absent");
    const auto& tag1 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    const auto& tag2 = regionTagSet.getTag<CreationTimeTag>(b(0), b(1000));
    ASSERT(tag2 != nullptr);
    ASSERT(tag2 == tag1);
    regionTagSet.removeTag<CreationTimeTag>(b(0), b(1000));
    ASSERT_ERROR(regionTagSet.getTag<CreationTimeTag>(b(0), b(1000)), "is absent");
    }

    { // 6. getAllTags
    RegionTagSet regionTagSet;
    ASSERT(regionTagSet.getAllTags<CreationTimeTag>(b(0), b(1000)).size() == 0);
    const auto& tag1 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    tag1->setCreationTime(42);
    const auto& tags = regionTagSet.getAllTags<CreationTimeTag>(b(0), b(1000));
    ASSERT(tags.size() == 1);
    ASSERT(tags[0].getOffset() == b(0) && tags[0].getLength() == b(1000));
    const auto& tag2 = tags[0].getTag();
    ASSERT(tag2 != nullptr);
    ASSERT(tag2->getCreationTime() == 42);
    regionTagSet.removeTag<CreationTimeTag>(b(0), b(1000));
    ASSERT(regionTagSet.getAllTags<CreationTimeTag>(b(0), b(1000)).size() == 0);
    const auto& tag3 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    tag3->setCreationTime(42);
    const auto& tag4 = regionTagSet.addTag<CreationTimeTag>(b(1000), b(1000));
    tag4->setCreationTime(81);
    const auto& tags2 = regionTagSet.getAllTagsForUpdate<CreationTimeTag>(b(500), b(1000));
    ASSERT(tags2.size() == 2);
    ASSERT(tags2[0].getOffset() == b(500) && tags2[0].getLength() == b(500));
    ASSERT(tags2[1].getOffset() == b(1000) && tags2[1].getLength() == b(500));
    const auto& tag5 = tags2[0].getTag();
    const auto& tag6 = tags2[1].getTag();
    ASSERT(tag5 != nullptr);
    ASSERT(tag5->getCreationTime() == 42);
    ASSERT(tag6 != nullptr);
    ASSERT(tag6->getCreationTime() == 81);
    ASSERT(regionTagSet.getNumTags() == 4);
    ASSERT(regionTagSet.getRegionTag(0).getOffset() == b(0) && regionTagSet.getRegionTag(0).getLength() == b(500));
    ASSERT(regionTagSet.getRegionTag(1).getOffset() == b(500) && regionTagSet.getRegionTag(1).getLength() == b(500));
    ASSERT(regionTagSet.getRegionTag(2).getOffset() == b(1000) && regionTagSet.getRegionTag(1).getLength() == b(500));
    ASSERT(regionTagSet.getRegionTag(3).getOffset() == b(1500) && regionTagSet.getRegionTag(1).getLength() == b(500));
    }

    { // 7. addTag
    RegionTagSet regionTagSet;
    const auto& tag1 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    ASSERT(tag1 != nullptr);
    ASSERT(regionTagSet.getNumTags() == 1);
    ASSERT_ERROR(regionTagSet.addTag<CreationTimeTag>(b(0), b(1000)), "is present");
    ASSERT_ERROR(regionTagSet.addTag<CreationTimeTag>(b(100), b(800)), "Overlapping");
    ASSERT_ERROR(regionTagSet.addTag<CreationTimeTag>(b(-100), b(200)), "Overlapping");
    ASSERT_ERROR(regionTagSet.addTag<CreationTimeTag>(b(900), b(200)), "Overlapping");
    ASSERT(regionTagSet.addTag<CreationTimeTag>(b(-100), b(100)) != nullptr);
    ASSERT(regionTagSet.addTag<CreationTimeTag>(b(1000), b(100)) != nullptr);
    ASSERT(regionTagSet.addTag<CreationTimeTag>(b(2000), b(1000)) != nullptr);
    ASSERT(regionTagSet.addTag<CreationTimeTag>(b(-2000), b(1000)) != nullptr);
    }

    { // 8. addTagIfAbsent
    RegionTagSet regionTagSet;
    const auto& tag1 = regionTagSet.addTagIfAbsent<CreationTimeTag>(b(0), b(1000)).get();
    const auto& tag2 = regionTagSet.addTagIfAbsent<CreationTimeTag>(b(0), b(1000)).get();
    ASSERT(tag2 != nullptr);
    ASSERT(tag2 == tag1);
    ASSERT(regionTagSet.getNumTags() == 1);
    ASSERT_ERROR(regionTagSet.addTagIfAbsent<CreationTimeTag>(b(100), b(800)), "Overlapping");
    ASSERT_ERROR(regionTagSet.addTagIfAbsent<CreationTimeTag>(b(-100), b(200)), "Overlapping");
    ASSERT_ERROR(regionTagSet.addTagIfAbsent<CreationTimeTag>(b(900), b(200)), "Overlapping");
    ASSERT(regionTagSet.addTagIfAbsent<CreationTimeTag>(b(-100), b(100)) != nullptr);
    ASSERT(regionTagSet.addTagIfAbsent<CreationTimeTag>(b(1000), b(100)) != nullptr);
    ASSERT(regionTagSet.addTagIfAbsent<CreationTimeTag>(b(2000), b(1000)) != nullptr);
    ASSERT(regionTagSet.addTagIfAbsent<CreationTimeTag>(b(-2000), b(1000)) != nullptr);
    }

    { // 9. removeTag
    RegionTagSet regionTagSet;
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(0), b(1000)), "is absent");
    const auto& tag1 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000)).get();
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(100), b(800)), "Overlapping");
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(-100), b(200)), "Overlapping");
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(900), b(200)), "Overlapping");
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(-100), b(100)), "is absent");
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(1000), b(100)), "is absent");
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(2000), b(1000)), "is absent");
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(-2000), b(1000)), "is absent");
    const auto& tag2 = regionTagSet.removeTag<CreationTimeTag>(b(0), b(1000)).get();
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(0), b(1000)), "is absent");
    ASSERT(tag2 != nullptr);
    ASSERT(tag2 == tag1);
    ASSERT(regionTagSet.getNumTags() == 0);
    }

    { // 10. removeTagIfPresent
    RegionTagSet regionTagSet;
    ASSERT(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(0), b(1000)) == nullptr);
    const auto& tag1 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000)).get();
    ASSERT_ERROR(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(100), b(800)), "Overlapping");
    ASSERT_ERROR(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(-100), b(200)), "Overlapping");
    ASSERT_ERROR(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(900), b(200)), "Overlapping");
    ASSERT(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(-100), b(100)) == nullptr);
    ASSERT(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(1000), b(100)) == nullptr);
    ASSERT(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(2000), b(1000)) == nullptr);
    ASSERT(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(-2000), b(1000)) == nullptr);
    const auto& tag2 = regionTagSet.removeTagIfPresent<CreationTimeTag>(b(0), b(1000)).get();
    ASSERT(tag2 != nullptr);
    ASSERT(tag2 == tag1);
    ASSERT(regionTagSet.getNumTags() == 0);
    ASSERT(regionTagSet.removeTagIfPresent<CreationTimeTag>(b(0), b(1000)) == nullptr);
    }

    { // 11. removeTagsWherePresent
    RegionTagSet regionTagSet;
    ASSERT_ERROR(regionTagSet.removeTag<CreationTimeTag>(b(0), b(1000)), "is absent");
    const auto& tag1 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    tag1->setCreationTime(42);
    const auto& tags1 = regionTagSet.removeTagsWherePresent<CreationTimeTag>(b(0), b(1000));
    ASSERT(tags1.size() == 1);
    ASSERT(tags1[0].getOffset() == b(0) && tags1[0].getLength() == b(1000));
    const auto& tag2 = tags1[0].getTag();
    ASSERT(tag2 != nullptr);
    ASSERT(tag2->getCreationTime() == 42);
    ASSERT(regionTagSet.getNumTags() == 0);
    const auto& tag3 = regionTagSet.addTag<CreationTimeTag>(b(0), b(1000));
    tag3->setCreationTime(42);
    const auto& tag4 = regionTagSet.addTag<CreationTimeTag>(b(1000), b(1000));
    tag4->setCreationTime(81);
    const auto& tags2 = regionTagSet.removeTagsWherePresent<CreationTimeTag>(b(500), b(1000));
    ASSERT(tags2.size() == 2);
    ASSERT(tags2[0].getOffset() == b(500) && tags2[0].getLength() == b(500));
    ASSERT(tags2[1].getOffset() == b(1000) && tags2[1].getLength() == b(500));
    const auto& tag5 = tags2[0].getTag();
    const auto& tag6 = tags2[1].getTag();
    ASSERT(tag5 != nullptr);
    ASSERT(tag5->getCreationTime() == 42);
    ASSERT(tag6 != nullptr);
    ASSERT(tag6->getCreationTime() == 81);
    ASSERT(regionTagSet.getNumTags() == 2);
    ASSERT(regionTagSet.getRegionTag(0).getOffset() == b(0) && regionTagSet.getRegionTag(0).getLength() == b(500));
    ASSERT(regionTagSet.getRegionTag(1).getOffset() == b(1500) && regionTagSet.getRegionTag(1).getLength() == b(500));
    }

    { // 12. copyTags
    RegionTagSet regionTagSet1;
    RegionTagSet regionTagSet2;
    regionTagSet2.copyTags(regionTagSet1, b(0), b(0), b(1000));
    ASSERT(regionTagSet2.getNumTags() == 0);
    regionTagSet1.addTag<CreationTimeTag>(b(0), b(1000));
    regionTagSet2.copyTags(regionTagSet1, b(0), b(0), b(1000));
    ASSERT(regionTagSet2.getNumTags() == 1);
    }
}

static void testChunkRegionTags()
{
    // FieldsChunk
    auto chunk1 = makeShared<IpHeader>();
    auto tag1 = chunk1->addTag<CreationTimeTag>(B(0), B(20));
    tag1->setCreationTime(42);
    chunk1->markImmutable();
    const auto& tag2 = chunk1->findTag<CreationTimeTag>();
    ASSERT(tag2 != nullptr);
    ASSERT(tag2->getCreationTime() == 42);

    // SliceChunk
    const auto& sliceChunk1 = chunk1->peek(B(10), B(10));
    ASSERT(sliceChunk1->getNumTags() == 1);
    const auto& tag3 = sliceChunk1->findTag<CreationTimeTag>();
    ASSERT(tag3 != nullptr);
    ASSERT(tag3->getCreationTime() == 42);

    // SequenceChunk
    auto sequenceChunk = makeShared<SequenceChunk>();
    sequenceChunk->insertAtBack(chunk1);
    ASSERT(sequenceChunk->getNumTags() == 1);
    const auto& tag4 = sequenceChunk->findTag<CreationTimeTag>();
    ASSERT(tag4 != nullptr);
    ASSERT(tag4->getCreationTime() == 42);
}

static void testPacketTags()
{
    // 1. application creates packet
    Packet packet1;
    ASSERT_ERROR(packet1.getTag<CreationTimeTag>(), "is absent");
    packet1.insertAtBack(makeImmutableByteCountChunk(B(1000)));
    const auto& tag1 = packet1.addTag<CreationTimeTag>();
    ASSERT(tag1 != nullptr);
    // 2. source TCP encapsulates packet
    packet1.insertAtFront(makeImmutableTcpHeader());
    const auto& tag2 = packet1.getTag<CreationTimeTag>();
    ASSERT(tag2 == tag1);
    // 3. destination TCP decapsulates packet
    packet1.popAtFront<TcpHeader>();
    const auto& tag3 = packet1.getTag<CreationTimeTag>();
    ASSERT(tag3 == tag2);
}

static void testPacketRegionTags()
{
    {
    // 1. copy on write for duplicated packet
    Packet packet1;
    packet1.insertAtBack(makeImmutableByteCountChunk(B(1000)));
    auto timeTag = packet1.addRegionTag<CreationTimeTag>(B(0), B(1000));
    timeTag->setCreationTime(42);
    auto packet2 = packet1.dup();
    packet2->mapAllRegionTagsForUpdate<CreationTimeTag>(B(0), B(1000), [&] (b o, b l, const Ptr<CreationTimeTag>& timeTag) {
        timeTag->setCreationTime(0);
    });
    ASSERT(packet1.getRegionTag<CreationTimeTag>(B(0), B(1000))->getCreationTime() == 42);
    ASSERT(packet2->getRegionTag<CreationTimeTag>(B(0), B(1000))->getCreationTime() == 0);
    }
}

static void testRegionTags()
{
    { // scenario showing how TCP could compute end to end delay
    // 1. source application creates packet
    auto chunk1 = makeShared<ByteCountChunk>(B(1500));
    auto tag1 = chunk1->addTag<CreationTimeTag>(B(0), B(1500));
    tag1->setCreationTime(42);
    // TODO: tag1->markImmutable();
    chunk1->markImmutable();
    Packet packet1;
    packet1.insertAtBack(chunk1);
    // 2. source application sends packet to TCP, which enqueues data
    ChunkQueue queue1;
    queue1.push(packet1.peekData());
    // 3. source TCP creates TCP segment
    Packet packet2;
    packet2.insertAtBack(queue1.pop(B(1000)));
    // 4. source TCP sends TCP segment, destination TCP enqueues data
    ReorderBuffer buffer1;
    buffer1.setExpectedOffset(B(0));
    buffer1.replace(B(0), packet2.peekData());
    // 5. destination TCP sends available data to application
    Packet packet3;
    packet3.insertAtBack(buffer1.popAvailableData());
    // 6. destination application processes received data
    const auto& chunk2 = packet3.peekData();
    auto regions1 = chunk2->getAllTags<CreationTimeTag>();
    ASSERT(regions1.size() == 1);
    ASSERT(regions1[0].getOffset() == B(0));
    ASSERT(regions1[0].getLength() == B(1000));
    ASSERT(regions1[0].getTag()->getCreationTime() == 42);

    // 7. source application creates another packet
    auto chunk3 = makeShared<ByteCountChunk>(B(1500));
    auto tag2 = chunk3->addTag<CreationTimeTag>(B(0), B(1500));
    tag2->setCreationTime(81);
    // TODO: tag1->markImmutable();
    chunk3->markImmutable();
    Packet packet4;
    packet4.insertAtBack(chunk3);
    // 8. source application sends packet to TCP, which enqueues data
    queue1.push(packet4.peekData());
    // 9. source TCP creates TCP segment
    Packet packet5;
    packet5.insertAtBack(queue1.pop(B(1000)));
    // 10. source TCP sends TCP segment, destination TCP enqueues data
    buffer1.replace(B(1000), packet5.peekData());
    // 11. destination TCP sends available data to application
    Packet packet6;
    packet6.insertAtBack(buffer1.popAvailableData());
    // 12. destination application processes received data
    const auto& chunk4 = packet6.peekData();
    auto regions2 = chunk4->getAllTags<CreationTimeTag>();
    ASSERT(regions2.size() == 2);
    ASSERT(regions2[0].getOffset() == B(0));
    ASSERT(regions2[0].getLength() == B(500));
    ASSERT(regions2[0].getTag()->getCreationTime() == 42);
    ASSERT(regions2[1].getOffset() == B(500));
    ASSERT(regions2[1].getLength() == B(500));
    ASSERT(regions2[1].getTag()->getCreationTime() == 81);
    }
}

void UnitTest::initialize()
{
    testMutable();
    testImmutable();
    testComplete();
    testIncomplete();
    testCorrect();
    testIncorrect();
    testProperlyRepresented();
    testImproperlyRepresented();
    testEmpty();
    testHeader();
    testTrailer();
    testFrontPopOffset();
    testBackPopOffset();
    testUpdate();
    testEncapsulation();
    testAggregation();
    testFragmentation();
    testPolymorphism();
    testStreaming();
    testSerialization();
    testConversion();
    testIteration();
    testCorruption();
    testDuplication();
    testDuality();
    testMerging();
    testSlicing();
    testNesting();
    testPeeking();
    testSequence();
    testSequenceSerialization();
    testChunkQueue();
    testChunkBuffer(getRNG(0));
    testReassemblyBuffer();
    testReorderBuffer();
    testTagSet();
    testRegionTagSet();
    testChunkRegionTags();
    testPacketTags();
    testPacketRegionTags();
    testRegionTags();
}

} // namespace
