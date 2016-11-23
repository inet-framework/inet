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

#include "inet/common/packet/BytesChunk.h"
#include "inet/common/packet/LengthChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/ReassemblyBuffer.h"
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
    stream.writeUint8(tlvHeader->getChunkLength());
    stream.writeUint8(tlvHeader->getBoolValue());
}

std::shared_ptr<Chunk> TlvHeader1Serializer::deserialize(ByteInputStream& stream) const
{
    auto tlvHeader = std::make_shared<TlvHeader1>();
    assert(tlvHeader->getType() == stream.readUint8());
    assert(tlvHeader->getChunkLength() == stream.readUint8());
    tlvHeader->setBoolValue(stream.readUint8());
    return tlvHeader;
}

void TlvHeader2Serializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& tlvHeader = std::static_pointer_cast<const TlvHeader2>(chunk);
    stream.writeUint8(tlvHeader->getType());
    stream.writeUint8(tlvHeader->getChunkLength());
    stream.writeUint16(tlvHeader->getInt16Value());
}

std::shared_ptr<Chunk> TlvHeader2Serializer::deserialize(ByteInputStream& stream) const
{
    auto tlvHeader = std::make_shared<TlvHeader2>();
    assert(tlvHeader->getType() == stream.readUint8());
    assert(tlvHeader->getChunkLength() == stream.readUint8());
    tlvHeader->setInt16Value(stream.readUint16());
    return tlvHeader;
}

static void testMutable()
{
    // 1. packet is mutable after construction
    Packet packet1;
    assert(packet1.isMutable());
    // 2. chunk is mutable after construction
    auto lengthChunk1 = std::make_shared<LengthChunk>(10);
    assert(lengthChunk1->isMutable());
    // 3. packet and chunk are still mutable after appending
    packet1.append(lengthChunk1);
    assert(packet1.isMutable());
    assert(lengthChunk1->isMutable());
}

static void testImmutable()
{
    // 1. chunk is immutable after making it immutable
    auto lengthChunk1 = std::make_shared<LengthChunk>(10);
    lengthChunk1->makeImmutable();
    assert(lengthChunk1->isImmutable());
    // 2. chunks become immutable after making a packet immutable
    Packet packet2;
    auto lengthChunk2 = std::make_shared<LengthChunk>(10);
    packet2.append(lengthChunk2);
    packet2.makeImmutable();
    assert(packet2.isImmutable());
    assert(lengthChunk2->isImmutable());
}

static void testHeader()
{
    // 1. packet contains header after chunk is appended
    Packet packet1;
    packet1.append(std::make_shared<LengthChunk>(10));
    const auto& lengthChunk1 = packet1.peekHeader<LengthChunk>();
    assert(lengthChunk1 != nullptr);
    // 2. packet provides headers in normal order
    Packet packet2;
    packet2.append(std::make_shared<LengthChunk>(10));
    packet2.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& lengthChunk2 = packet2.popHeader<LengthChunk>();
    const auto& bytesChunk1 = packet2.popHeader<BytesChunk>();
    assert(lengthChunk2 != nullptr);
    assert(lengthChunk2->getChunkLength() == 10);
    assert(bytesChunk1 != nullptr);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}).begin()));
}

static void testTrailer()
{
    // 1. packet contains trailer after chunk is appended
    Packet packet1;
    packet1.append(std::make_shared<LengthChunk>(10));
    const auto& lengthChunk1 = packet1.peekTrailer<LengthChunk>();
    assert(lengthChunk1 != nullptr);
    // 2. packet provides trailers in reverse order
    Packet packet2;
    packet2.append(std::make_shared<LengthChunk>(10));
    packet2.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& bytesChunk1 = packet2.popTrailer<BytesChunk>();
    const auto& lengthChunk2 = packet2.popTrailer<LengthChunk>();
    assert(bytesChunk1 != nullptr);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}).begin()));
    assert(lengthChunk2 != nullptr);
    assert(lengthChunk2->getChunkLength() == 10);
}

static void testComplete()
{
    // 1. chunk is complete after construction
    auto lengthChunk1 = std::make_shared<LengthChunk>(10);
    assert(lengthChunk1->isComplete());
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
    tcpHeader1->setChunkLength(16);
    tcpHeader1->setLengthField(16);
    tcpHeader1->setBitError(BIT_ERROR_CRC);
    tcpHeader1->setSrcPort(1000);
    packet2.append(tcpHeader1);
    packet2.makeImmutable();
    const auto& tcpHeader2 = packet2.popHeader<TcpHeader>(8);
    assert(tcpHeader2->isIncomplete());
    assert(tcpHeader2->getChunkLength() == 8);
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
    assert(applicationHeader2->getChunkLength() == 10);
}

static void testNesting()
{
    // 1. packet contains compound header
    Packet packet1;
    auto ipHeader1 = std::make_shared<IpHeader>();
    ipHeader1->setProtocol(Protocol::Tcp);
    auto compoundHeader1 = std::make_shared<CompoundHeader>();
    compoundHeader1->append(ipHeader1);
    packet1.append(compoundHeader1);
    packet1.makeImmutable();
    const auto& compoundHeader2 = packet1.peekHeader<CompoundHeader>();
    assert(compoundHeader2 != nullptr);
    // 2. packet provides compound header after serialization
    Packet packet2(packet1.peek<BytesChunk>());
    const auto& compoundHeader3 = packet2.peekHeader<CompoundHeader>();
    assert(compoundHeader3 != nullptr);
    auto it = compoundHeader3->createForwardIterator();
    const auto& ipHeader2 = compoundHeader3->Chunk::peek<IpHeader>(it);
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
    Packet packet2(packet1.peek<BytesChunk>());
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
    auto totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader1);
    auto size = stream1.getSize();
    assert(size != 0);
    assert(totalSerializedLength + size == ChunkSerializer::totalSerializedLength);
    totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader1);
    assert(stream1.getSize() == 2 * size);
    assert(totalSerializedLength == ChunkSerializer::totalSerializedLength);
    // 2. serialized bytes is cached after deserialization
    ByteInputStream stream2(stream1.getBytes());
    auto totalDeserializedLength = ChunkSerializer::totalDeserializedLength;
    auto applicationHeader2 = std::dynamic_pointer_cast<ApplicationHeader>(Chunk::deserialize(stream2, typeid(ApplicationHeader)));
    assert(totalDeserializedLength + size == ChunkSerializer::totalDeserializedLength);
    totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader2);
    assert(stream1.getSize() == 3 * size);
    assert(totalSerializedLength == ChunkSerializer::totalSerializedLength);
    // 3. serialized bytes is deleted after change
    applicationHeader1->setSomeData(42);
    totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader1);
    assert(totalSerializedLength + size == ChunkSerializer::totalSerializedLength);
    applicationHeader2->setSomeData(42);
    totalSerializedLength = ChunkSerializer::totalSerializedLength;
    Chunk::serialize(stream1, applicationHeader2);
    assert(totalSerializedLength + size == ChunkSerializer::totalSerializedLength);
}

static void testPeekChunk()
{
    // 1. LengthChunk always returns LengthChunk
    auto lengthChunk1 = std::make_shared<LengthChunk>(10);
    lengthChunk1->makeImmutable();
    const auto& lengthChunk2 = std::dynamic_pointer_cast<LengthChunk>(lengthChunk1->peek(0, 5));
    assert(lengthChunk2 != nullptr);
    assert(lengthChunk2->getChunkLength() == 5);
    // 2. BytesChunk always returns BytesChunk
    auto bytesChunk1 = std::make_shared<BytesChunk>();
    bytesChunk1->setBytes({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    bytesChunk1->makeImmutable();
    const auto& bytesChunk2 = std::dynamic_pointer_cast<BytesChunk>(bytesChunk1->peek(0, 5));
    assert(bytesChunk2 != nullptr);
    assert(std::equal(bytesChunk2->getBytes().begin(), bytesChunk2->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4}).begin()));
    // 3. SliceChunk returns a SliceChunk containing the requested slice of the chunk that is used by the original SliceChunk
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->makeImmutable();
    auto sliceChunk1 = std::make_shared<SliceChunk>(applicationHeader1, 0, 10);
    sliceChunk1->makeImmutable();
    auto sliceChunk2 = std::dynamic_pointer_cast<SliceChunk>(sliceChunk1->peek(0, 5));
    assert(sliceChunk1->getChunk() == sliceChunk2->getChunk());
    // 4a. SequenceChunk may return an element chunk
    auto sequenceChunk1 = std::make_shared<SequenceChunk>();
    sequenceChunk1->append(lengthChunk1);
    sequenceChunk1->append(bytesChunk1);
    sequenceChunk1->append(applicationHeader1);
    sequenceChunk1->makeImmutable();
    const auto& lengthChunk3 = std::dynamic_pointer_cast<LengthChunk>(sequenceChunk1->peek(0, 10));
    const auto& bytesChunk3 = std::dynamic_pointer_cast<BytesChunk>(sequenceChunk1->peek(10, 10));
    const auto& applicationHeader2 = std::dynamic_pointer_cast<ApplicationHeader>(sequenceChunk1->peek(20, 10));
    assert(lengthChunk3 != nullptr);
    assert(bytesChunk3 != nullptr);
    assert(applicationHeader2 != nullptr);
    // 4b. SequenceChunk may return a SliceChunk of an element chunk
    const auto& sliceChunk3 = std::dynamic_pointer_cast<SliceChunk>(sequenceChunk1->peek(0, 5));
    const auto& sliceChunk4 = std::dynamic_pointer_cast<SliceChunk>(sequenceChunk1->peek(15, 5));
    const auto& sliceChunk5 = std::dynamic_pointer_cast<SliceChunk>(sequenceChunk1->peek(20, 5));
    assert(sliceChunk3 != nullptr);
    assert(sliceChunk4 != nullptr);
    assert(sliceChunk5 != nullptr);
    // 4c. SequenceChunk may return a SliceChunk using the original SequenceChunk
    const auto& sliceChunk6 = std::dynamic_pointer_cast<SliceChunk>(sequenceChunk1->peek(5, 20));
    assert(sliceChunk6 != nullptr);
    // 5. any other chunk returns a SliceChunk
    auto applicationHeader3 = std::make_shared<ApplicationHeader>();
    applicationHeader3->makeImmutable();
    auto sliceChunk7 = std::dynamic_pointer_cast<SliceChunk>(applicationHeader3->peek(0, 5));
    assert(sliceChunk7 != nullptr);
}

static void testPeekPacket()
{
    // 1. packet provides LengthChunks by default if it contains a LengthChunk only
    Packet packet1;
    packet1.append(std::make_shared<LengthChunk>(10));
    packet1.append(std::make_shared<LengthChunk>(10));
    packet1.append(std::make_shared<LengthChunk>(10));
    packet1.makeImmutable();
    const auto& lengthChunk1 = std::dynamic_pointer_cast<LengthChunk>(packet1.popHeader(15));
    const auto& lengthChunk2 = std::dynamic_pointer_cast<LengthChunk>(packet1.popHeader(15));
    assert(lengthChunk1 != nullptr);
    assert(lengthChunk2 != nullptr);
    // 2. packet provides BytesChunks by default if it contains a BytesChunk only
    Packet packet2;
    packet2.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet2.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet2.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& lengthChunk3 = std::dynamic_pointer_cast<BytesChunk>(packet2.popHeader(15));
    const auto& lengthChunk4 = std::dynamic_pointer_cast<BytesChunk>(packet2.popHeader(15));
    assert(lengthChunk3 != nullptr);
    assert(lengthChunk4 != nullptr);
}

static void testPeekBuffer()
{
    // 1. buffer provides LengthChunks by default if it contains a LengthChunk only
    Buffer buffer1;
    buffer1.push(std::make_shared<LengthChunk>(10));
    buffer1.push(std::make_shared<LengthChunk>(10));
    buffer1.push(std::make_shared<LengthChunk>(10));
    const auto& lengthChunk1 = std::dynamic_pointer_cast<LengthChunk>(buffer1.pop(15));
    const auto& lengthChunk2 = std::dynamic_pointer_cast<LengthChunk>(buffer1.pop(15));
    assert(lengthChunk1 != nullptr);
    assert(lengthChunk2 != nullptr);
    // 2. buffer provides BytesChunks by default if it contains a BytesChunk only
    Buffer buffer2;
    buffer2.push(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    buffer2.push(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    buffer2.push(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& lengthChunk3 = std::dynamic_pointer_cast<BytesChunk>(buffer2.pop(15));
    const auto& lengthChunk4 = std::dynamic_pointer_cast<BytesChunk>(buffer2.pop(15));
    assert(lengthChunk3 != nullptr);
    assert(lengthChunk4 != nullptr);
}

static void testClone()
{
    // 1. copy of immutable packet shares data
    Packet packet1;
    std::shared_ptr<LengthChunk> lengthChunk1 = std::make_shared<LengthChunk>(10);
    packet1.append(lengthChunk1);
    packet1.makeImmutable();
    auto packet2 = packet1.dup();
    assert(packet2->getByteLength() == 10);
    assert(lengthChunk1.use_count() == 3);
    delete packet2;
    // 2. copy of mutable packet copies data
    Packet packet3;
    auto lengthChunk2 = std::make_shared<LengthChunk>(10);
    packet3.append(lengthChunk2);
    auto packet4 = packet3.dup();
    lengthChunk2->setByteLength(20);
    assert(packet4->getByteLength() == 10);
    assert(lengthChunk2.use_count() == 2);
    delete packet4;
    // 3. copy of immutable chunk in mutable packet shares data
    Packet packet5;
    std::shared_ptr<LengthChunk> lengthChunk3 = std::make_shared<LengthChunk>(10);
    packet5.append(lengthChunk3);
    lengthChunk3->makeImmutable();
    auto packet6 = packet5.dup();
    assert(packet6->getByteLength() == 10);
    assert(lengthChunk3.use_count() == 3);
    delete packet6;
}

static void testReassembly()
{
    // 1. single chunk
    NewReassemblyBuffer buffer1;
    buffer1.setData(0, std::make_shared<LengthChunk>(10));
    assert(buffer1.isComplete());
    assert(buffer1.getData() != nullptr);
    // 2. consecutive chunks
    NewReassemblyBuffer buffer2;
    buffer2.setData(0, std::make_shared<LengthChunk>(10));
    buffer2.setData(10, std::make_shared<LengthChunk>(10));
    const auto& lengthChunk1 = std::dynamic_pointer_cast<LengthChunk>(buffer2.getData());
    assert(buffer2.isComplete());
    assert(lengthChunk1 != nullptr);
    // 3. consecutive slice chunks
    NewReassemblyBuffer buffer3;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->makeImmutable();
    // TODO: dup is needed to make the chunk mutable and to be able to merge it
    buffer3.setData(0, applicationHeader1->peek(0, 5)->dupShared());
    buffer3.setData(5, applicationHeader1->peek(5, 5)->dupShared());
    const auto& applicationHeader2 = std::dynamic_pointer_cast<ApplicationHeader>(buffer2.getData());
    assert(buffer3.isComplete());
    // TODO: assert(applicationHeader2 != nullptr);
    // 4. out of order consecutive chunks
    NewReassemblyBuffer buffer4;
    buffer4.setData(0, std::make_shared<LengthChunk>(10));
    buffer4.setData(20, std::make_shared<LengthChunk>(10));
    buffer4.setData(10, std::make_shared<LengthChunk>(10));
    const auto& lengthChunk2 = std::dynamic_pointer_cast<LengthChunk>(buffer4.getData());
    assert(buffer4.isComplete());
    assert(lengthChunk2 != nullptr);
    // 5. heterogeneous chunks
    NewReassemblyBuffer buffer5;
    buffer5.setData(0, std::make_shared<LengthChunk>(10));
    buffer5.setData(10, std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    assert(buffer5.isComplete());
    assert(buffer5.getData() != nullptr);
    // 6. completely overwriting a chunk
    NewReassemblyBuffer buffer6;
    buffer6.setData(1, std::make_shared<LengthChunk>(8));
    buffer6.setData(0, std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& bytesChunk1 = std::dynamic_pointer_cast<BytesChunk>(buffer6.getData());
    assert(buffer6.isComplete());
    assert(bytesChunk1 != nullptr);
    // 7. partially overwriting multiple chunks
    NewReassemblyBuffer buffer7;
    buffer7.setData(0, std::make_shared<LengthChunk>(10));
    buffer7.setData(10, std::make_shared<LengthChunk>(10));
    buffer7.setData(5, std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& sequenceChunk1 = std::dynamic_pointer_cast<SequenceChunk>(buffer7.getData());
    sequenceChunk1->makeImmutable();
    const auto& bytesChunk2 = std::dynamic_pointer_cast<BytesChunk>(sequenceChunk1->peek(5, 10));
    assert(buffer7.isComplete());
    assert(sequenceChunk1 != nullptr);
    assert(bytesChunk2 != nullptr);
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
    testReassembly();
}

} // namespace
