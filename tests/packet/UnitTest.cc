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

#include "inet/common/packet/ByteArrayChunk.h"
#include "inet/common/packet/ByteLengthChunk.h"
#include "inet/common/packet/Packet.h"
#include "NewTest.h"
#include "UnitTest_m.h"
#include "UnitTest.h"

namespace inet {

Register_Serializer(CompoundHeaderSerializer);
Register_Serializer(TlvHeaderSerializer);
Register_Serializer(TlvHeader1Serializer);
Register_Serializer(TlvHeader2Serializer);
Define_Module(UnitTest);

std::shared_ptr<Chunk> CompoundHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto compoundHeader = std::make_shared<CompoundHeader>();
    IpHeaderSerializer ipHeaderSerializer;
    auto ipHeader = ipHeaderSerializer.deserialize(stream);
    compoundHeader->append(ipHeader);
    return compoundHeader;
}

void TlvHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    throw cRuntimeError("Invalid operation");
}

std::shared_ptr<Chunk> TlvHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    uint8_t type = stream.readUint8();
    stream.seek(stream.getPosition() - 1);
    switch (type) {
        case 1:
            return TlvHeader1Serializer().deserialize(stream);
        case 2:
            return TlvHeader2Serializer().deserialize(stream);
        default:
            throw cRuntimeError("Invalid TLV type");
    }
}

void TlvHeader1Serializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& tlvHeader = std::static_pointer_cast<const TlvHeader1>(chunk);
    stream.writeUint8(tlvHeader->getType());
    stream.writeUint8(tlvHeader->getByteLength());
    stream.writeUint8(tlvHeader->getBoolValue());
}

std::shared_ptr<Chunk> TlvHeader1Serializer::deserialize(ByteInputStream& stream) const
{
    auto tlvHeader = std::make_shared<TlvHeader1>();
    assert(tlvHeader->getType() == stream.readUint8());
    assert(tlvHeader->getByteLength() == stream.readUint8());
    tlvHeader->setBoolValue(stream.readUint8());
    return tlvHeader;
}

void TlvHeader2Serializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& tlvHeader = std::static_pointer_cast<const TlvHeader2>(chunk);
    stream.writeUint8(tlvHeader->getType());
    stream.writeUint8(tlvHeader->getByteLength());
    stream.writeUint16(tlvHeader->getInt16Value());
}

std::shared_ptr<Chunk> TlvHeader2Serializer::deserialize(ByteInputStream& stream) const
{
    auto tlvHeader = std::make_shared<TlvHeader2>();
    assert(tlvHeader->getType() == stream.readUint8());
    assert(tlvHeader->getByteLength() == stream.readUint8());
    tlvHeader->setInt16Value(stream.readUint16());
    return tlvHeader;
}

static void testMutable()
{
    // 1. packet is mutable after construction
    Packet packet1;
    assert(packet1.isMutable());
    // 2. chunk is mutable after construction
    auto byteLengthChunk1 = std::make_shared<ByteLengthChunk>(10);
    assert(byteLengthChunk1->isMutable());
    // 3. packet and chunk are still mutable after appending
    packet1.append(byteLengthChunk1);
    assert(packet1.isMutable());
    assert(byteLengthChunk1->isMutable());
}

static void testImmutable()
{
    // 1. packet is immutable after making it immutable
    Packet packet1;
    packet1.makeImmutable();
    assert(packet1.isImmutable());
    // 2. chunk is immutable after making it immutable
    auto byteLengthChunk1 = std::make_shared<ByteLengthChunk>(10);
    byteLengthChunk1->makeImmutable();
    assert(byteLengthChunk1->isImmutable());
    // 3. chunks become immutable after making a packet immutable
    Packet packet2;
    auto byteLengthChunk2 = std::make_shared<ByteLengthChunk>(10);
    packet2.append(byteLengthChunk2);
    packet2.makeImmutable();
    assert(packet2.isImmutable());
    assert(byteLengthChunk2->isImmutable());
}

static void testHeader()
{
    // 1. packet contains header after chunk is appended
    Packet packet1;
    packet1.append(std::make_shared<ByteLengthChunk>(10));
    const auto& byteLengthChunk = packet1.peekHeader<ByteLengthChunk>();
    assert(byteLengthChunk != nullptr);
}

static void testTrailer()
{
    // 1. packet contains trailer after chunk is appended
    Packet packet2;
    packet2.append(std::make_shared<ByteLengthChunk>(10));
    const auto& byteLengthChunk = packet2.peekTrailer<ByteLengthChunk>();
    assert(byteLengthChunk != nullptr);
}

static void testComplete()
{
    // 1. chunk is complete after construction
    auto byteLengthChunk1 = std::make_shared<ByteLengthChunk>(10);
    assert(byteLengthChunk1->isComplete());
}

static void testIncomplete()
{
    // 1. packet doesn't provide incomplete header if complete is requested but there's not enough data
    Packet packet1;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    packet1.append(applicationHeader1);
    packet1.makeImmutable();
    Packet fragment1;
    fragment1.append(packet1.peekHeaderAt(0, 5));
    fragment1.makeImmutable();
    assert(!fragment1.hasHeader<ApplicationHeader>());
    const auto& applicationHeader2 = fragment1.peekHeader<ApplicationHeader>();
    assert(applicationHeader2 == nullptr);
    // 2. packet provides incomplete header if requested
    Packet packet2;
    auto tcpHeader1 = std::make_shared<TcpHeader>();
    tcpHeader1->setByteLength(16);
    tcpHeader1->setLengthField(16);
    tcpHeader1->setBitError(BIT_ERROR_CRC);
    tcpHeader1->setSrcPort(1000);
    packet2.append(tcpHeader1);
    packet2.makeImmutable();
    const auto& tcpHeader2 = packet2.popHeader<TcpHeader>(8);
    assert(tcpHeader2->isIncomplete());
    assert(tcpHeader2->getByteLength() == 8);
    assert(tcpHeader2->getBitError() == BIT_ERROR_CRC);
    assert(tcpHeader2->getSrcPort() == 1000);
}

static void testMerge()
{
    // 1. packet provides complete merged header if the whole header is available
    Packet packet1;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    packet1.append(applicationHeader1);
    packet1.makeImmutable();
    const auto& header1 = packet1.peekHeaderAt(0, 5);
    const auto& header2 = packet1.peekHeaderAt(5, 5);
    Packet packet2;
    packet2.append(header1);
    packet2.append(header2);
    packet2.makeImmutable();
    const auto& applicationHeader2 = packet2.peekHeader<ApplicationHeader>();
    assert(applicationHeader2->isComplete());
    assert(applicationHeader2->getByteLength() == 10);
}

static void testNesting()
{
    // 1. packet contains compound header
    Packet packet1;
    auto ipHeader1 = std::make_shared<IpHeader>();
    ipHeader1->setProtocol(Protocol::Tcp);
    auto compoundHeader1 = std::make_shared<CompoundHeader>();
    compoundHeader1->append(ipHeader1);
    packet1.append(compoundHeader1, false);
    packet1.makeImmutable();
    const auto& compoundHeader2 = packet1.peekHeader<CompoundHeader>();
    assert(compoundHeader2 != nullptr);
    // 2. packet provides compound header after serialization
    const auto& byteArrayChunk1 = packet1.peek<ByteArrayChunk>();
    Packet packet2;
    packet2.append(byteArrayChunk1);
    const auto& compoundHeader3 = packet2.peekHeader<CompoundHeader>();
    assert(compoundHeader3 != nullptr);
    auto it = compoundHeader3->createForwardIterator();
    const auto& ipHeader2 = compoundHeader3->peek<IpHeader>(it);
    assert(ipHeader2 != nullptr);
    assert(ipHeader2->getProtocol() == Protocol::Tcp);
}

static void testPolymorphism()
{
    // 1. packet provides headers in a polymorphic way without serialization
    Packet packet1;
    auto tlvHeader1 = std::make_shared<TlvHeader1>();
    tlvHeader1->setBoolValue(true);
    packet1.append(tlvHeader1);
    auto tlvHeader2 = std::make_shared<TlvHeader2>();
    tlvHeader2->setInt16Value(42);
    packet1.append(tlvHeader2);
    packet1.makeImmutable();
    const auto& tlvHeader3 = std::dynamic_pointer_cast<TlvHeader1>(packet1.popHeader<TlvHeader>());
    assert(tlvHeader3 != nullptr);
    assert(tlvHeader3->getBoolValue());
    const auto& tlvHeader4 = std::dynamic_pointer_cast<TlvHeader2>(packet1.popHeader<TlvHeader>());
    assert(tlvHeader4 != nullptr);
    assert(tlvHeader4->getInt16Value() == 42);
    // 1. packet provides serialized headers in a polymorphic way after deserialization
    const auto& byteArrayChunk1 = packet1.peek<ByteArrayChunk>();
    Packet packet2;
    packet2.append(byteArrayChunk1);
    const auto& tlvHeader5 = std::dynamic_pointer_cast<TlvHeader1>(packet2.popHeader<TlvHeader>());
    assert(tlvHeader5 != nullptr);
    assert(tlvHeader5->getBoolValue());
    const auto& tlvHeader6 = std::dynamic_pointer_cast<TlvHeader2>(packet2.popHeader<TlvHeader>());
    assert(tlvHeader6 != nullptr);
    assert(tlvHeader6->getInt16Value() == 42);
}

static void testSerialize()
{
    // 1. serialized bytes is cached after serialization
    ByteOutputStream stream1;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    auto totalSerializedByteLength = ChunkSerializer::totalSerializedByteLength;
    Chunk::serialize(stream1, applicationHeader1);
    auto size = stream1.getSize();
    assert(size != 0);
    assert(totalSerializedByteLength + size == ChunkSerializer::totalSerializedByteLength);
    totalSerializedByteLength = ChunkSerializer::totalSerializedByteLength;
    Chunk::serialize(stream1, applicationHeader1);
    assert(stream1.getSize() == 2 * size);
    assert(totalSerializedByteLength == ChunkSerializer::totalSerializedByteLength);
    // 2. serialized bytes is cached after deserialization
    ByteInputStream stream2(stream1.getBytes());
    auto totalDeserializedByteLength = ChunkSerializer::totalDeserializedByteLength;
    auto applicationHeader2 = std::dynamic_pointer_cast<ApplicationHeader>(Chunk::deserialize(stream2, typeid(ApplicationHeader)));
    assert(totalDeserializedByteLength + size == ChunkSerializer::totalDeserializedByteLength);
    totalSerializedByteLength = ChunkSerializer::totalSerializedByteLength;
    Chunk::serialize(stream1, applicationHeader2);
    assert(stream1.getSize() == 3 * size);
    assert(totalSerializedByteLength == ChunkSerializer::totalSerializedByteLength);
    // 3. serialized bytes is deleted after change
    applicationHeader1->setSomeData(42);
    totalSerializedByteLength = ChunkSerializer::totalSerializedByteLength;
    Chunk::serialize(stream1, applicationHeader1);
    assert(totalSerializedByteLength + size == ChunkSerializer::totalSerializedByteLength);
    applicationHeader2->setSomeData(42);
    totalSerializedByteLength = ChunkSerializer::totalSerializedByteLength;
    Chunk::serialize(stream1, applicationHeader2);
    assert(totalSerializedByteLength + size == ChunkSerializer::totalSerializedByteLength);
}

static void testPeekChunk()
{
    // 1. ByteLengthChunk always returns ByteLengthChunk
    auto byteLengthChunk1 = std::make_shared<ByteLengthChunk>(10);
    byteLengthChunk1->makeImmutable();
    const auto& byteLengthChunk2 = std::dynamic_pointer_cast<ByteLengthChunk>(byteLengthChunk1->peek(0, 5));
    assert(byteLengthChunk2 != nullptr);
    assert(byteLengthChunk2->getByteLength() == 5);
    // 2. ByteArrayChunk always returns ByteArrayChunk
    auto byteArrayChunk1 = std::make_shared<ByteArrayChunk>();
    byteArrayChunk1->setBytes({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    byteArrayChunk1->makeImmutable();
    const auto& byteArrayChunk2 = std::dynamic_pointer_cast<ByteArrayChunk>(byteArrayChunk1->peek(0, 5));
    assert(byteArrayChunk2 != nullptr);
    assert(std::equal(byteArrayChunk2->getBytes().begin(), byteArrayChunk2->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4}).begin()));
    // 3. SliceChunk returns a SliceChunk containing the requested slice of the chunk that is used by the original SliceChunk
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->makeImmutable();
    auto sliceChunk1 = std::make_shared<SliceChunk>(applicationHeader1, 0, 10);
    sliceChunk1->makeImmutable();
    auto sliceChunk2 = std::dynamic_pointer_cast<SliceChunk>(sliceChunk1->peek(0, 5));
    assert(sliceChunk1->getChunk() == sliceChunk2->getChunk());
    // 4. SequenceChunk may return an element chunk, a SliceChunk of an element chunk, a SequenceChunk potentially containing SliceChunks at both ends
    auto sequenceChunk = std::make_shared<SequenceChunk>();
    // TODO:
    // 5. any other chunk returns a SliceChunk
    auto applicationHeader2 = std::make_shared<ApplicationHeader>();
    applicationHeader2->makeImmutable();
    auto sliceChunk3 = std::dynamic_pointer_cast<SliceChunk>(applicationHeader2->peek(0, 5));
    assert(sliceChunk3 != nullptr);
}

static void testPeekPacket()
{
    // 1. packet provides ByteLengthChunks by default if it contains a ByteLengthChunk only
    Packet packet1;
    packet1.append(std::make_shared<ByteLengthChunk>(10));
    packet1.append(std::make_shared<ByteLengthChunk>(10));
    packet1.append(std::make_shared<ByteLengthChunk>(10));
    packet1.makeImmutable();
    const auto& byteLengthChunk1 = std::dynamic_pointer_cast<ByteLengthChunk>(packet1.popHeader(15));
    const auto& byteLengthChunk2 = std::dynamic_pointer_cast<ByteLengthChunk>(packet1.popHeader(15));
    assert(byteLengthChunk1 != nullptr);
    assert(byteLengthChunk2 != nullptr);
    // 2. packet provides ByteArrayChunks by default if it contains a ByteArrayChunk only
    Packet packet2;
    packet2.append(std::make_shared<ByteArrayChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet2.append(std::make_shared<ByteArrayChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet2.append(std::make_shared<ByteArrayChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& byteLengthChunk3 = std::dynamic_pointer_cast<ByteArrayChunk>(packet2.popHeader(15));
    const auto& byteLengthChunk4 = std::dynamic_pointer_cast<ByteArrayChunk>(packet2.popHeader(15));
    assert(byteLengthChunk3 != nullptr);
    assert(byteLengthChunk4 != nullptr);
}

static void testPeekBuffer()
{
    // 1. buffer provides ByteLengthChunks by default if it contains a ByteLengthChunk only
    Buffer buffer1;
    buffer1.push(std::make_shared<ByteLengthChunk>(10));
    buffer1.push(std::make_shared<ByteLengthChunk>(10));
    buffer1.push(std::make_shared<ByteLengthChunk>(10));
    const auto& byteLengthChunk1 = std::dynamic_pointer_cast<ByteLengthChunk>(buffer1.pop(15));
    const auto& byteLengthChunk2 = std::dynamic_pointer_cast<ByteLengthChunk>(buffer1.pop(15));
    assert(byteLengthChunk1 != nullptr);
    assert(byteLengthChunk2 != nullptr);
    // 2. buffer provides ByteArrayChunks by default if it contains a ByteArrayChunk only
    Buffer buffer2;
    buffer2.push(std::make_shared<ByteArrayChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    buffer2.push(std::make_shared<ByteArrayChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    buffer2.push(std::make_shared<ByteArrayChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& byteLengthChunk3 = std::dynamic_pointer_cast<ByteArrayChunk>(buffer2.pop(15));
    const auto& byteLengthChunk4 = std::dynamic_pointer_cast<ByteArrayChunk>(buffer2.pop(15));
    assert(byteLengthChunk3 != nullptr);
    assert(byteLengthChunk4 != nullptr);
}

static void testClone()
{
    // 1. copy of immutable packet shares data
    Packet packet1;
    std::shared_ptr<ByteLengthChunk> byteLengthChunk1 = std::make_shared<ByteLengthChunk>(10);
    packet1.append(byteLengthChunk1);
    packet1.makeImmutable();
    auto packet2 = packet1.dup();
    assert(packet2->getByteLength() == 10);
    assert(byteLengthChunk1.use_count() == 3);
    delete packet2;
    // 2. copy of mutable packet copies data
    Packet packet3;
    auto byteLengthChunk2 = std::make_shared<ByteLengthChunk>(10);
    packet3.append(byteLengthChunk2);
    auto packet4 = packet3.dup();
    byteLengthChunk2->setByteLength(20);
    assert(packet4->getByteLength() == 10);
    assert(byteLengthChunk2.use_count() == 2);
    delete packet4;
    // 3. copy of immutable chunk in mutable packet shares data
    Packet packet5;
    std::shared_ptr<ByteLengthChunk> byteLengthChunk3 = std::make_shared<ByteLengthChunk>(10);
    packet5.append(byteLengthChunk3);
    byteLengthChunk3->makeImmutable();
    auto packet6 = packet5.dup();
    assert(packet6->getByteLength() == 10);
    assert(byteLengthChunk3.use_count() == 3);
    delete packet6;
}

void UnitTest::initialize()
{
    testMutable();
    testImmutable();
    testHeader();
    testTrailer();
    testComplete();
    testIncomplete();
    testMerge();
    testNesting();
    testPolymorphism();
    testSerialize();
    testPeekChunk();
    testPeekPacket();
    testPeekBuffer();
    testClone();
}

} // namespace
