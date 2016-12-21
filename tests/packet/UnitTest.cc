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

#include "inet/common/packet/ByteCountChunk.h"
#include "inet/common/packet/BytesChunk.h"
#include "inet/common/packet/ChunkBuffer.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/ReassemblyBuffer.h"
#include "inet/common/packet/ReorderBuffer.h"
#include "inet/common/packet/SerializerRegistry.h"
#include "NewTest.h"
#include "UnitTest_m.h"
#include "UnitTest.h"

namespace inet {

Register_Serializer(CompoundHeader, CompoundHeaderSerializer);
Register_Serializer(TlvHeader, TlvHeaderSerializer);
Register_Serializer(TlvHeaderBool, TlvHeaderBoolSerializer);
Register_Serializer(TlvHeaderInt, TlvHeaderIntSerializer);
Define_Module(UnitTest);

static std::vector<uint8_t> makeVector(int length)
{
    std::vector<uint8_t> bytes;
    for (int i = 0; i < length; i++)
        bytes.push_back(i);
    return bytes;
}

static std::shared_ptr<ByteCountChunk> makeImmutableByteCountChunk(int length)
{
    auto chunk = std::make_shared<ByteCountChunk>(length);
    chunk->markImmutable();
    return chunk;
}

static std::shared_ptr<BytesChunk> makeImmutableBytesChunk(const std::vector<uint8_t>& bytes)
{
    auto chunk = std::make_shared<BytesChunk>(bytes);
    chunk->markImmutable();
    return chunk;
}

static std::shared_ptr<ApplicationHeader> makeImmutableApplicationHeader(int someData)
{
    auto chunk = std::make_shared<ApplicationHeader>();
    chunk->setSomeData(someData);
    chunk->markImmutable();
    return chunk;
}

static std::shared_ptr<TcpHeader> makeImmutableTcpHeader()
{
    auto chunk = std::make_shared<TcpHeader>();
    chunk->markImmutable();
    return chunk;
}

static std::shared_ptr<IpHeader> makeImmutableIpHeader()
{
    auto chunk = std::make_shared<IpHeader>();
    chunk->markImmutable();
    return chunk;
}

static std::shared_ptr<EthernetHeader> makeImmutableEthernetHeader()
{
    auto chunk = std::make_shared<EthernetHeader>();
    chunk->markImmutable();
    return chunk;
}

static std::shared_ptr<EthernetTrailer> makeImmutableEthernetTrailer()
{
    auto chunk = std::make_shared<EthernetTrailer>();
    chunk->markImmutable();
    return chunk;
}

std::shared_ptr<Chunk> CompoundHeaderSerializer::deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const
{
    auto compoundHeader = std::make_shared<CompoundHeader>();
    IpHeaderSerializer ipHeaderSerializer;
    auto ipHeader = ipHeaderSerializer.deserialize(stream);
    compoundHeader->insertAtEnd(ipHeader);
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
            return TlvHeaderBoolSerializer().deserialize(stream);
        case 2:
            return TlvHeaderIntSerializer().deserialize(stream);
        default:
            throw cRuntimeError("Invalid TLV type");
    }
}

void TlvHeaderBoolSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& tlvHeader = std::static_pointer_cast<const TlvHeaderBool>(chunk);
    stream.writeUint8(tlvHeader->getType());
    stream.writeUint8(tlvHeader->getChunkLength());
    stream.writeUint8(tlvHeader->getBoolValue());
}

std::shared_ptr<Chunk> TlvHeaderBoolSerializer::deserialize(ByteInputStream& stream) const
{
    auto tlvHeader = std::make_shared<TlvHeaderBool>();
    assert(tlvHeader->getType() == stream.readUint8());
    assert(tlvHeader->getChunkLength() == stream.readUint8());
    tlvHeader->setBoolValue(stream.readUint8());
    return tlvHeader;
}

void TlvHeaderIntSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& tlvHeader = std::static_pointer_cast<const TlvHeaderInt>(chunk);
    stream.writeUint8(tlvHeader->getType());
    stream.writeUint8(tlvHeader->getChunkLength());
    stream.writeUint16(tlvHeader->getInt16Value());
}

std::shared_ptr<Chunk> TlvHeaderIntSerializer::deserialize(ByteInputStream& stream) const
{
    auto tlvHeader = std::make_shared<TlvHeaderInt>();
    assert(tlvHeader->getType() == stream.readUint8());
    assert(tlvHeader->getChunkLength() == stream.readUint8());
    tlvHeader->setInt16Value(stream.readUint16());
    return tlvHeader;
}

static void testMutable()
{
    // 1. chunk is mutable after construction
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    assert(byteCountChunk1->isMutable());
}

static void testImmutable()
{
    // 1. chunk is immutable after marking it immutable
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    byteCountChunk1->markImmutable();
    assert(byteCountChunk1->isImmutable());
}

static void testComplete()
{
    // 1. chunk is complete after construction
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    assert(byteCountChunk1->isComplete());
}

static void testIncomplete()
{
    // 1. packet doesn't provide incomplete header if complete is requested but there's not enough data
    Packet packet1;
    packet1.append(makeImmutableApplicationHeader(42));
    Packet fragment1;
    fragment1.append(packet1.peekAt(0, 5));
    assert(!fragment1.hasHeader<ApplicationHeader>());
    const auto& applicationHeader2 = fragment1.peekHeader<ApplicationHeader>();
    assert(applicationHeader2 == nullptr);

    // 2. packet provides incomplete variable length header if requested
    Packet packet2;
    auto tcpHeader1 = std::make_shared<TcpHeader>();
    tcpHeader1->setChunkLength(16);
    tcpHeader1->setLengthField(16);
    tcpHeader1->setCrcMode(CRC_COMPUTED);
    tcpHeader1->setSrcPort(1000);
    tcpHeader1->setDestPort(1000);
    tcpHeader1->markImmutable();
    packet2.append(tcpHeader1);
    const auto& tcpHeader2 = packet2.popHeader<TcpHeader>(4);
    assert(tcpHeader2->isIncomplete());
    assert(tcpHeader2->getChunkLength() == 4);
    assert(tcpHeader2->getCrcMode() == CRC_COMPUTED);
    assert(tcpHeader2->getSrcPort() == 1000);
    assert(tcpHeader2->getDestPort() != 1000);
}

static void testCorrect()
{
    // 1. chunk is correct after construction
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    assert(byteCountChunk1->isCorrect());
}

static void testIncorrect()
{
    // 1. chunk is incorrect after marking it incorrect
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    applicationHeader1->markIncorrect();
    assert(applicationHeader1->isIncorrect());
}

static void testProperlyRepresented()
{
    // 1. chunk is proper after construction
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    assert(byteCountChunk1->isProperlyRepresented());
}

static void testImproperlyRepresented()
{
    // 1. chunk is improperly represented after deserialization of a non-representable packet
    Packet packet1;
    auto ipHeader1 = std::make_shared<IpHeader>();
    ipHeader1->markImmutable();
    packet1.append(ipHeader1);
    assert(ipHeader1->isProperlyRepresented());
    auto bytesChunk1 = std::static_pointer_cast<BytesChunk>(packet1.peekAt<BytesChunk>(0, packet1.getPacketLength())->dupShared());
    bytesChunk1->setByte(0, 42);
    bytesChunk1->markImmutable();
    Packet packet2(nullptr, bytesChunk1);
    const auto& ipHeader2 = packet2.peekHeader<IpHeader>();
    assert(ipHeader2->isImproperlyRepresented());
}

static void testHeader()
{
    // 1. packet contains header after chunk is appended
    Packet packet1;
    packet1.pushHeader(makeImmutableByteCountChunk(10));
    const auto& chunk1 = packet1.peekHeader();
    assert(chunk1 != nullptr);
    assert(chunk1->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk1) != nullptr);
    const auto& chunk2 = packet1.peekHeader<ByteCountChunk>();
    assert(chunk2 != nullptr);
    assert(chunk2->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk2) != nullptr);

    // 2. packet moves header pointer after pop
    const auto& chunk3 = packet1.popHeader<ByteCountChunk>();
    assert(chunk3 != nullptr);
    assert(chunk3->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk3) != nullptr);
    assert(packet1.getHeaderPopOffset() == 10);

    // 3. packet provides headers in reverse prepend order
    Packet packet2;
    packet2.pushHeader(makeImmutableBytesChunk(makeVector(10)));
    packet2.pushHeader(makeImmutableByteCountChunk(10));
    const auto& chunk4 = packet2.popHeader<ByteCountChunk>();
    const auto& chunk5 = packet2.popHeader<BytesChunk>();
    assert(chunk4 != nullptr);
    assert(chunk4->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk4) != nullptr);
    assert(chunk5 != nullptr);
    assert(chunk5->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk5) != nullptr);
    const auto& bytesChunk1 = std::static_pointer_cast<BytesChunk>(chunk5);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(10).begin()));
}

static void testTrailer()
{
    // 1. packet contains trailer after chunk is appended
    Packet packet1;
    packet1.pushTrailer(makeImmutableByteCountChunk(10));
    const auto& chunk1 = packet1.peekTrailer();
    assert(chunk1 != nullptr);
    assert(chunk1->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk1) != nullptr);
    const auto& chunk2 = packet1.peekTrailer<ByteCountChunk>();
    assert(chunk2 != nullptr);
    assert(chunk2->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk2) != nullptr);

    // 2. packet moves trailer pointer after pop
    const auto& chunk3 = packet1.popTrailer<ByteCountChunk>();
    assert(chunk3 != nullptr);
    assert(chunk3->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk3) != nullptr);
    assert(packet1.getTrailerPopOffset() == 0);

    // 3. packet provides trailers in reverse order
    Packet packet2;
    packet2.pushTrailer(makeImmutableBytesChunk(makeVector(10)));
    packet2.pushTrailer(makeImmutableByteCountChunk(10));
    const auto& chunk4 = packet2.popTrailer<ByteCountChunk>();
    const auto& chunk5 = packet2.popTrailer<BytesChunk>();
    assert(chunk4 != nullptr);
    assert(chunk4->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk4) != nullptr);
    assert(chunk5 != nullptr);
    assert(chunk5->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk5) != nullptr);
    const auto& bytesChunk1 = std::static_pointer_cast<BytesChunk>(chunk5);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(10).begin()));
}

static void testEncapsulation()
{
    // 1. packet contains all chunks of encapsulated packet as is
    Packet packet1;
    packet1.append(makeImmutableByteCountChunk(10));
    packet1.append(makeImmutableBytesChunk(makeVector(10)));
    // encapsulation packet with header and trailer
    auto& packet2 = packet1;
    packet2.pushHeader(makeImmutableEthernetHeader());
    packet2.pushTrailer(makeImmutableEthernetTrailer());
    const auto& ethernetHeader1 = packet2.popHeader<EthernetHeader>();
    const auto& ethernetTrailer1 = packet2.popTrailer<EthernetTrailer>();
    const auto& byteCountChunk1 = packet2.peekDataAt(0, 10);
    const auto& bytesChunk1 = packet2.peekDataAt(10, 10);
    const auto& dataChunk1 = packet2.peekDataAt<BytesChunk>(0, packet2.getDataLength());
    assert(ethernetHeader1 != nullptr);
    assert(ethernetTrailer1 != nullptr);
    assert(byteCountChunk1 != nullptr);
    assert(bytesChunk1 != nullptr);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(byteCountChunk1) != nullptr);
    assert(std::dynamic_pointer_cast<BytesChunk>(bytesChunk1) != nullptr);
    assert(byteCountChunk1->getChunkLength() == 10);
    assert(bytesChunk1->getChunkLength() == 10);
    assert(dataChunk1->getChunkLength() == 20);
}

static void testAggregation()
{
    // 1. packet contains all chunks of aggregated packets as is
    Packet packet1;
    packet1.append(makeImmutableByteCountChunk(10));
    Packet packet2;
    packet2.append(makeImmutableBytesChunk(makeVector(10)));
    Packet packet3;
    packet3.append(makeImmutableIpHeader());
    // aggregate other packets
    packet3.append(packet1.peekAt(0, packet2.getPacketLength()));
    packet3.append(packet2.peekAt(0, packet2.getPacketLength()));
    const auto& ipHeader1 = packet3.popHeader<IpHeader>();
    const auto& chunk1 = packet3.peekDataAt(0, 10);
    const auto& chunk2 = packet3.peekDataAt(10, 10);
    assert(ipHeader1 != nullptr);
    assert(chunk1 != nullptr);
    assert(chunk1->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk1) != nullptr);
    assert(chunk2 != nullptr);
    assert(chunk2->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk2) != nullptr);
    const auto& bytesChunk1 = std::static_pointer_cast<BytesChunk>(chunk2);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(10).begin()));
}

static void testFragmentation()
{
    // 1. packet contains fragment of another packet
    Packet packet1;
    packet1.append(makeImmutableByteCountChunk(10));
    packet1.append(makeImmutableBytesChunk(makeVector(10)));
    Packet packet2;
    packet2.append(makeImmutableIpHeader());
    // append fragment of another packet
    packet2.append(packet1.peekAt(7, 10));
    const auto& ipHeader1 = packet2.popHeader<IpHeader>();
    const auto& fragment1 = packet2.peekDataAt(0, packet2.getDataLength());
    const auto& chunk1 = fragment1->peek(0, 3);
    const auto& chunk2 = fragment1->peek(3, 7);
    assert(packet2.getPacketLength() == 30);
    assert(ipHeader1 != nullptr);
    assert(std::dynamic_pointer_cast<IpHeader>(ipHeader1) != nullptr);
    assert(fragment1 != nullptr);
    assert(fragment1->getChunkLength() == 10);
    assert(chunk1 != nullptr);
    assert(chunk1->getChunkLength() == 3);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk1) != nullptr);
    assert(chunk2 != nullptr);
    assert(chunk2->getChunkLength() == 7);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk2) != nullptr);
    const auto& bytesChunk1 = std::static_pointer_cast<BytesChunk>(chunk2);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(7).begin()));
}

static void testPolymorphism()
{
    // 1. packet provides headers in a polymorphic way without serialization
    Packet packet1;
    auto tlvHeader1 = std::make_shared<TlvHeaderBool>();
    tlvHeader1->setBoolValue(true);
    tlvHeader1->markImmutable();
    packet1.append(tlvHeader1);
    auto tlvHeader2 = std::make_shared<TlvHeaderInt>();
    tlvHeader2->setInt16Value(42);
    tlvHeader2->markImmutable();
    packet1.append(tlvHeader2);
    const auto& tlvHeader3 = packet1.popHeader<TlvHeader>();
    assert(tlvHeader3 != nullptr);
    assert(tlvHeader3->getChunkLength() == 3);
    assert(std::dynamic_pointer_cast<TlvHeaderBool>(tlvHeader3) != nullptr);
    const auto & tlvHeaderBool1 = std::static_pointer_cast<TlvHeaderBool>(tlvHeader3);
    assert(tlvHeaderBool1->getBoolValue());
    const auto& tlvHeader4 = packet1.popHeader<TlvHeader>();
    assert(tlvHeader4 != nullptr);
    assert(tlvHeader4->getChunkLength() == 4);
    assert(std::dynamic_pointer_cast<TlvHeaderInt>(tlvHeader4) != nullptr);
    const auto & tlvHeaderInt1 = std::static_pointer_cast<TlvHeaderInt>(tlvHeader4);
    assert(tlvHeaderInt1->getInt16Value() == 42);

    // 2. packet provides deserialized headers in a polymorphic way after serialization
    Packet packet2(nullptr, packet1.peekAt<BytesChunk>(0, packet1.getPacketLength()));
    const auto& tlvHeader5 = packet2.popHeader<TlvHeader>();
    assert(tlvHeader5 != nullptr);
    assert(tlvHeader5->getChunkLength() == 3);
    assert(std::dynamic_pointer_cast<TlvHeaderBool>(tlvHeader5) != nullptr);
    const auto & tlvHeaderBool2 = std::static_pointer_cast<TlvHeaderBool>(tlvHeader5);
    assert(tlvHeaderBool2->getBoolValue());
    const auto& tlvHeader6 = packet2.popHeader<TlvHeader>();
    assert(tlvHeader6 != nullptr);
    assert(tlvHeader6->getChunkLength() == 4);
    assert(std::dynamic_pointer_cast<TlvHeaderInt>(tlvHeader6) != nullptr);
    const auto & tlvHeaderInt2 = std::static_pointer_cast<TlvHeaderInt>(tlvHeader6);
    assert(tlvHeaderInt2->getInt16Value() == 42);
}

static void testSerialization()
{
    // 1. serialized bytes is cached after serialization
    ByteOutputStream stream1;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    auto totalSerializedBytes = ChunkSerializer::totalSerializedBytes;
    Chunk::serialize(stream1, applicationHeader1);
    auto size = stream1.getSize();
    assert(size != 0);
    assert(totalSerializedBytes + size == ChunkSerializer::totalSerializedBytes);
    totalSerializedBytes = ChunkSerializer::totalSerializedBytes;
    Chunk::serialize(stream1, applicationHeader1);
    assert(stream1.getSize() == 2 * size);
    assert(totalSerializedBytes == ChunkSerializer::totalSerializedBytes);

    // 2. serialized bytes is cached after deserialization
    ByteInputStream stream2(stream1.getBytes());
    auto totalDeserializedBytes = ChunkSerializer::totalDeserializedBytes;
    const auto& chunk1 = Chunk::deserialize(stream2, typeid(ApplicationHeader));
    assert(chunk1 != nullptr);
    assert(chunk1->getChunkLength() == size);
    assert(std::dynamic_pointer_cast<ApplicationHeader>(chunk1) != nullptr);
    auto applicationHeader2 = std::static_pointer_cast<ApplicationHeader>(chunk1);
    assert(totalDeserializedBytes + size == ChunkSerializer::totalDeserializedBytes);
    totalSerializedBytes = ChunkSerializer::totalSerializedBytes;
    Chunk::serialize(stream1, applicationHeader2);
    assert(stream1.getSize() == 3 * size);
    assert(totalSerializedBytes == ChunkSerializer::totalSerializedBytes);

    // 3. serialized bytes is deleted after change
    applicationHeader1->setSomeData(42);
    totalSerializedBytes = ChunkSerializer::totalSerializedBytes;
    Chunk::serialize(stream1, applicationHeader1);
    assert(totalSerializedBytes + size == ChunkSerializer::totalSerializedBytes);
    applicationHeader2->setSomeData(42);
    totalSerializedBytes = ChunkSerializer::totalSerializedBytes;
    Chunk::serialize(stream1, applicationHeader2);
    assert(totalSerializedBytes + size == ChunkSerializer::totalSerializedBytes);
}

static void testIteration()
{
    // 1. packet provides appended chunks
    Packet packet1;
    packet1.append(makeImmutableByteCountChunk(10));
    packet1.append(makeImmutableBytesChunk(makeVector(10)));
    packet1.append(makeImmutableApplicationHeader(42));
    int index1 = 0;
    auto chunk1 = packet1.popHeader();
    while (chunk1 != nullptr) {
        assert(chunk1 != nullptr);
        assert(chunk1->getChunkLength() == 10);
        index1++;
        chunk1 = packet1.popHeader();
    }
    assert(index1 == 3);

    // 2. SequenceChunk optimizes forward iteration to indexing
    auto sequenceChunk1 = std::make_shared<SequenceChunk>();
    sequenceChunk1->insertAtEnd(makeImmutableByteCountChunk(10));
    sequenceChunk1->insertAtEnd(makeImmutableBytesChunk(makeVector(10)));
    sequenceChunk1->insertAtEnd(makeImmutableApplicationHeader(42));
    sequenceChunk1->markImmutable();
    int index2 = 0;
    auto iterator2 = Chunk::ForwardIterator(0, 0);
    auto chunk2 = sequenceChunk1->peek(iterator2);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk2) != nullptr);
    while (chunk2 != nullptr) {
        assert(iterator2.getIndex() == index2);
        assert(iterator2.getPosition() == index2 * 10);
        assert(chunk2 != nullptr);
        assert(chunk2->getChunkLength() == 10);
        index2++;
        if (chunk2 != nullptr)
            sequenceChunk1->moveIterator(iterator2, chunk2->getChunkLength());
        chunk2 = sequenceChunk1->peek(iterator2);
    }
    assert(index2 == 3);

    // 3. SequenceChunk optimizes backward iteration to indexing
    auto sequenceChunk2 = std::make_shared<SequenceChunk>();
    sequenceChunk2->insertAtEnd(makeImmutableByteCountChunk(10));
    sequenceChunk2->insertAtEnd(makeImmutableBytesChunk(makeVector(10)));
    sequenceChunk2->insertAtEnd(makeImmutableApplicationHeader(42));
    sequenceChunk2->markImmutable();
    int index3 = 0;
    auto iterator3 = Chunk::BackwardIterator(0, 0);
    auto chunk3 = sequenceChunk1->peek(iterator3);
    assert(std::dynamic_pointer_cast<ApplicationHeader>(chunk3) != nullptr);
    while (chunk3 != nullptr) {
        assert(iterator3.getIndex() == index3);
        assert(iterator3.getPosition() == index3 * 10);
        assert(chunk3 != nullptr);
        assert(chunk3->getChunkLength() == 10);
        index3++;
        if (chunk3 != nullptr)
            sequenceChunk1->moveIterator(iterator3, chunk3->getChunkLength());
        chunk3 = sequenceChunk1->peek(iterator3);
    }
    assert(index2 == 3);
}

static void testCorruption()
{
    // 1. data corruption with constant bit error rate
    double random[] = {0.1, 0.7, 0.9};
    double ber = 1E-2;
    Packet packet1;
    const auto& chunk1 = makeImmutableByteCountChunk(10);
    const auto& chunk2 = makeImmutableBytesChunk(makeVector(10));
    const auto& chunk3 = makeImmutableApplicationHeader(42);
    packet1.append(chunk1);
    packet1.append(chunk2);
    packet1.append(chunk3);
    int index = 0;
    auto chunk = packet1.popHeader();
    while (chunk != nullptr) {
        int64_t bitLength = 8 * chunk->getChunkLength();
        if (random[index++] >= std::pow(1 - ber, bitLength))
            chunk->markIncorrect();
        chunk = packet1.popHeader();
    }
    assert(chunk1->isCorrect());
    assert(chunk2->isIncorrect());
    assert(chunk3->isIncorrect());
}

static void testDuplication()
{
    // 1. copy of immutable packet shares chunk
    Packet packet1;
    std::shared_ptr<ByteCountChunk> byteCountChunk1 = makeImmutableByteCountChunk(10);
    packet1.append(byteCountChunk1);
    auto packet2 = packet1.dup();
    assert(packet2->getPacketLength() == 10);
    assert(byteCountChunk1.use_count() == 3); // 1 here + 2 in the packets
    delete packet2;
}

static void testDuality()
{
    // 1. packet provides header in both fields and bytes representation
    Packet packet1;
    packet1.append(makeImmutableApplicationHeader(42));
    const auto& applicationHeader1 = packet1.peekHeader<ApplicationHeader>();
    const auto& bytesChunk1 = packet1.peekHeader<BytesChunk>(10);
    assert(applicationHeader1 != nullptr);
    assert(applicationHeader1->getChunkLength() == 10);
    assert(bytesChunk1 != nullptr);
    assert(bytesChunk1->getChunkLength() == 10);

    // 2. packet provides header in both fields and bytes representation after serialization
    Packet packet2(nullptr, packet1.peekAt<BytesChunk>(0, packet1.getPacketLength()));
    const auto& applicationHeader2 = packet2.peekHeader<ApplicationHeader>();
    const auto& bytesChunk2 = packet2.peekHeader<BytesChunk>(10);
    assert(applicationHeader2 != nullptr);
    assert(applicationHeader2->getChunkLength() == 10);
    assert(bytesChunk2 != nullptr);
    assert(bytesChunk2->getChunkLength() == 10);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), bytesChunk1->getBytes().begin()));
    assert(applicationHeader2->getSomeData() == applicationHeader2->getSomeData());
}

static void testMerging()
{
    // 1. packet provides complete merged header if the whole header is available
    Packet packet1;
    packet1.append(makeImmutableApplicationHeader(42));
    Packet packet2;
    packet2.append(packet1.peekAt(0, 5));
    packet2.append(packet1.peekAt(5, 5));
    const auto& chunk1 = packet2.peekHeader();
    assert(chunk1 != nullptr);
    assert(chunk1->isComplete());
    assert(chunk1->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ApplicationHeader>(chunk1) != nullptr);
    const auto& chunk2 = packet2.peekHeader<ApplicationHeader>();
    assert(chunk2->isComplete());
    assert(chunk2->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ApplicationHeader>(chunk2) != nullptr);

    // 2. packet provides compacts ByteCountChunks
    Packet packet3;
    packet3.append(makeImmutableByteCountChunk(5));
    packet3.append(makeImmutableByteCountChunk(5));
    const auto& chunk3 = packet3.peekAt(0, packet3.getPacketLength());
    const auto& chunk4 = packet3.peekAt<ByteCountChunk>(0, packet3.getPacketLength());
    assert(chunk3 != nullptr);
    assert(chunk3->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk3) != nullptr);
    assert(chunk4 != nullptr);
    assert(chunk4->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk4) != nullptr);

    // 2. packet provides compacts ByteChunks
    Packet packet4;
    packet4.append(makeImmutableBytesChunk(makeVector(5)));
    packet4.append(makeImmutableBytesChunk(makeVector(5)));
    const auto& chunk5 = packet4.peekAt(0, packet4.getPacketLength());
    const auto& chunk6 = packet4.peekAt<BytesChunk>(0, packet4.getPacketLength());
    assert(chunk5 != nullptr);
    assert(chunk5->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk5) != nullptr);
    const auto& bytesChunk1 = std::static_pointer_cast<BytesChunk>(chunk5);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 0, 1, 2, 3, 4}).begin()));
    assert(chunk6 != nullptr);
    assert(chunk6->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk6) != nullptr);
    const auto& bytesChunk2 = std::static_pointer_cast<BytesChunk>(chunk6);
    assert(std::equal(bytesChunk2->getBytes().begin(), bytesChunk2->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 0, 1, 2, 3, 4}).begin()));
}

static void testSlicing()
{
    // 1. ByteCountChunk always returns ByteCountChunk
    auto byteCountChunk1 = makeImmutableByteCountChunk(10);
    const auto& chunk1 = byteCountChunk1->peek(0, 10);
    const auto& chunk2 = byteCountChunk1->peek(0, 5);
    assert(chunk1 == byteCountChunk1);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk1) != nullptr);
    assert(chunk2 != nullptr);
    assert(chunk2->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk2) != nullptr);

    // 2. BytesChunk always returns BytesChunk
    auto bytesChunk1 = makeImmutableBytesChunk(makeVector(10));
    const auto& chunk3 = bytesChunk1->peek(0, 10);
    const auto& chunk4 = bytesChunk1->peek(0, 5);
    assert(chunk3 != nullptr);
    assert(chunk3->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk3) != nullptr);
    const auto& bytesChunk2 = std::static_pointer_cast<BytesChunk>(chunk3);
    assert(std::equal(bytesChunk2->getBytes().begin(), bytesChunk2->getBytes().end(), makeVector(10).begin()));
    assert(chunk4 != nullptr);
    assert(chunk4->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk4) != nullptr);
    const auto& bytesChunk3 = std::static_pointer_cast<BytesChunk>(chunk4);
    assert(std::equal(bytesChunk3->getBytes().begin(), bytesChunk3->getBytes().end(), makeVector(5).begin()));

    // 3a. SliceChunk returns a SliceChunk containing the requested slice of the chunk that is used by the original SliceChunk
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    auto sliceChunk1 = std::make_shared<SliceChunk>(applicationHeader1, 0, 10);
    sliceChunk1->markImmutable();
    const auto& chunk5 = sliceChunk1->peek(5, 5);
    assert(chunk5 != nullptr);
    assert(chunk5->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<SliceChunk>(chunk5) != nullptr);
    auto sliceChunk2 = std::static_pointer_cast<SliceChunk>(chunk5);
    assert(sliceChunk2->getChunk() == sliceChunk1->getChunk());
    assert(sliceChunk2->getOffset() == 5);
    assert(sliceChunk2->getLength() == 5);

    // 4a. SequenceChunk may return an element chunk
    auto sequenceChunk1 = std::make_shared<SequenceChunk>();
    sequenceChunk1->insertAtEnd(byteCountChunk1);
    sequenceChunk1->insertAtEnd(bytesChunk1);
    sequenceChunk1->insertAtEnd(applicationHeader1);
    sequenceChunk1->markImmutable();
    const auto& chunk6 = sequenceChunk1->peek(0, 10);
    const auto& chunk7 = sequenceChunk1->peek(10, 10);
    const auto& chunk8 = sequenceChunk1->peek(20, 10);
    assert(chunk6 != nullptr);
    assert(chunk6->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk6) != nullptr);
    assert(chunk7 != nullptr);
    assert(chunk7->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk7) != nullptr);
    assert(chunk8 != nullptr);
    assert(chunk8->getChunkLength() == 10);
    assert(std::dynamic_pointer_cast<ApplicationHeader>(chunk8) != nullptr);

    // 4b. SequenceChunk may return a (simplified) SliceChunk of an element chunk
    const auto& chunk9 = sequenceChunk1->peek(0, 5);
    const auto& chunk10 = sequenceChunk1->peek(15, 5);
    const auto& chunk11 = sequenceChunk1->peek(20, 5);
    assert(chunk9 != nullptr);
    assert(chunk9->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk9) != nullptr);
    assert(chunk10 != nullptr);
    assert(chunk10->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk10) != nullptr);
    assert(chunk11 != nullptr);
    assert(chunk11->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<SliceChunk>(chunk11) != nullptr);

    // 4c. SequenceChunk may return a SliceChunk using the original SequenceChunk
    const auto& chunk12 = sequenceChunk1->peek(5, 20);
    assert(chunk12 != nullptr);
    assert(chunk12->getChunkLength() == 20);
    assert(std::dynamic_pointer_cast<SliceChunk>(chunk12) != nullptr);
    const auto& sliceChunk3 = std::static_pointer_cast<SliceChunk>(chunk12);
    assert(sliceChunk3->getChunk() == sequenceChunk1);

    // 5. any other chunk returns a SliceChunk
    auto applicationHeader2 = makeImmutableApplicationHeader(42);
    const auto& chunk13 = applicationHeader2->peek(0, 5);
    assert(chunk13 != nullptr);
    assert(chunk13->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<SliceChunk>(chunk13) != nullptr);
    const auto& sliceChunk4 = std::dynamic_pointer_cast<SliceChunk>(chunk13);
    assert(sliceChunk4->getChunk() == applicationHeader2);
    assert(sliceChunk4->getOffset() == 0);
    assert(sliceChunk4->getLength() == 5);
}

static void testNesting()
{
    // 1. packet contains compound header as is
    Packet packet1;
    auto ipHeader1 = std::make_shared<IpHeader>();
    ipHeader1->setProtocol(Protocol::Tcp);
    auto compoundHeader1 = std::make_shared<CompoundHeader>();
    compoundHeader1->insertAtEnd(ipHeader1);
    compoundHeader1->markImmutable();
    packet1.append(compoundHeader1);
    const auto& compoundHeader2 = packet1.peekHeader<CompoundHeader>();
    assert(compoundHeader2 != nullptr);

    // 2. packet provides compound header after serialization
    Packet packet2(nullptr, packet1.peekAt<BytesChunk>(0, packet1.getPacketLength()));
    const auto& compoundHeader3 = packet2.peekHeader<CompoundHeader>();
    assert(compoundHeader3 != nullptr);
    auto it = Chunk::ForwardIterator(0, 0);
    const auto& ipHeader2 = compoundHeader3->Chunk::peek<IpHeader>(it);
    assert(ipHeader2 != nullptr);
    assert(ipHeader2->getProtocol() == Protocol::Tcp);
}

static void testPeeking()
{
    // 1. packet provides ByteCountChunks by default if it contains a ByteCountChunk only
    Packet packet1;
    packet1.append(makeImmutableByteCountChunk(10));
    packet1.append(makeImmutableByteCountChunk(10));
    packet1.append(makeImmutableByteCountChunk(10));
    const auto& chunk1 = packet1.popHeader(15);
    const auto& chunk2 = packet1.popHeader(15);
    assert(chunk1 != nullptr);
    assert(chunk1->getChunkLength() == 15);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk1) != nullptr);
    assert(chunk2 != nullptr);
    assert(chunk2->getChunkLength() == 15);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk2) != nullptr);

    // 2. packet provides BytesChunks by default if it contains a BytesChunk only
    Packet packet2;
    packet2.append(makeImmutableBytesChunk(makeVector(10)));
    packet2.append(makeImmutableBytesChunk(makeVector(10)));
    packet2.append(makeImmutableBytesChunk(makeVector(10)));
    const auto& chunk3 = packet2.popHeader(15);
    const auto& chunk4 = packet2.popHeader(15);
    assert(chunk3 != nullptr);
    assert(chunk3->getChunkLength() == 15);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk3) != nullptr);
    assert(chunk4 != nullptr);
    assert(chunk4->getChunkLength() == 15);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk4) != nullptr);
}

static void testSequence()
{
    // 1. sequence merges immutable slices
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    auto sequenceChunk1 = std::make_shared<SequenceChunk>();
    sequenceChunk1->insertAtEnd(applicationHeader1->peek(0, 5));
    sequenceChunk1->insertAtEnd(applicationHeader1->peek(5, 5));
    const auto& chunk1 = sequenceChunk1->peek(0);
    assert(std::dynamic_pointer_cast<ApplicationHeader>(chunk1) != nullptr);

    // 2. sequence merges mutable slices
    auto sequenceChunk2 = std::make_shared<SequenceChunk>();
    sequenceChunk2->insertAtEnd(std::make_shared<SliceChunk>(applicationHeader1, 0, 5));
    sequenceChunk2->insertAtEnd(std::make_shared<SliceChunk>(applicationHeader1, 5, 5));
    const auto& chunk2 = sequenceChunk2->peek(0);
    assert(std::dynamic_pointer_cast<ApplicationHeader>(chunk2) != nullptr);
}

static void testChunkQueue()
{
    // 1. queue provides ByteCountChunks by default if it contains a ByteCountChunk only
    ChunkQueue queue1;
    auto byteCountChunk1 = makeImmutableByteCountChunk(10);
    queue1.push(byteCountChunk1);
    queue1.push(byteCountChunk1);
    queue1.push(byteCountChunk1);
    const auto& byteCountChunk2 = std::dynamic_pointer_cast<ByteCountChunk>(queue1.pop(15));
    const auto& byteCountChunk3 = std::dynamic_pointer_cast<ByteCountChunk>(queue1.pop(15));
    assert(byteCountChunk2 != nullptr);
    assert(byteCountChunk3 != nullptr);

    // 2. queue provides BytesChunks by default if it contains a BytesChunk only
    ChunkQueue queue2;
    auto bytesChunk1 = makeImmutableBytesChunk(makeVector(10));
    queue2.push(bytesChunk1);
    queue2.push(bytesChunk1);
    queue2.push(bytesChunk1);
    const auto& bytesChunk2 = std::dynamic_pointer_cast<BytesChunk>(queue2.pop(15));
    const auto& bytesChunk3 = std::dynamic_pointer_cast<BytesChunk>(queue2.pop(15));
    assert(bytesChunk2 != nullptr);
    assert(bytesChunk3 != nullptr);

    // 3. queue provides reassembled header
    ChunkQueue queue3;
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    queue3.push(applicationHeader1->peek(0, 5));
    queue3.push(applicationHeader1->peek(5, 5));
    assert(queue3.has<ApplicationHeader>());
    const auto& applicationHeader2 = queue3.pop<ApplicationHeader>();
    assert(applicationHeader2 != nullptr);
    assert(applicationHeader2->getSomeData() == 42);
}

static void testChunkBuffer()
{
    // 1. single chunk
    ChunkBuffer buffer1;
    auto byteCountChunk1 = makeImmutableByteCountChunk(10);
    buffer1.replace(0, byteCountChunk1);
    assert(buffer1.getNumRegions() == 1);
    assert(buffer1.getRegionData(0) != nullptr);

    // 2. consecutive chunks
    ChunkBuffer buffer2;
    buffer2.replace(0, byteCountChunk1);
    buffer2.replace(10, byteCountChunk1);
    const auto& byteCountChunk2 = std::dynamic_pointer_cast<ByteCountChunk>(buffer2.getRegionData(0));
    assert(buffer2.getNumRegions() == 1);
    assert(byteCountChunk2 != nullptr);
    assert(byteCountChunk2->getChunkLength() == 20);

    // 3. consecutive slice chunks
    ChunkBuffer buffer3;
    auto applicationHeader1 = makeImmutableApplicationHeader(42);
    buffer3.replace(0, applicationHeader1->peek(0, 5));
    buffer3.replace(5, applicationHeader1->peek(5, 5));
    const auto& applicationHeader2 = std::dynamic_pointer_cast<ApplicationHeader>(buffer3.getRegionData(0));
    assert(buffer3.getNumRegions() == 1);
    assert(applicationHeader2 != nullptr);
    assert(applicationHeader2->getSomeData() == 42);

    // 4. out of order consecutive chunks
    ChunkBuffer buffer4;
    buffer4.replace(0, byteCountChunk1);
    buffer4.replace(20, byteCountChunk1);
    buffer4.replace(10, byteCountChunk1);
    const auto& byteCountChunk3 = std::dynamic_pointer_cast<ByteCountChunk>(buffer4.getRegionData(0));
    assert(buffer4.getNumRegions() == 1);
    assert(byteCountChunk3 != nullptr);
    assert(byteCountChunk3->getChunkLength() == 30);

    // 5. out of order consecutive slice chunks
    ChunkBuffer buffer5;
    buffer5.replace(0, applicationHeader1->peek(0, 3));
    buffer5.replace(7, applicationHeader1->peek(7, 3));
    buffer5.replace(3, applicationHeader1->peek(3, 4));
    const auto& applicationHeader3 = std::dynamic_pointer_cast<ApplicationHeader>(buffer5.getRegionData(0));
    assert(buffer5.getNumRegions() == 1);
    assert(applicationHeader3 != nullptr);
    assert(applicationHeader3->getSomeData() == 42);

    // 6. heterogeneous chunks
    ChunkBuffer buffer6;
    auto byteArrayChunk1 = makeImmutableBytesChunk(makeVector(10));
    buffer6.replace(0, byteCountChunk1);
    buffer6.replace(10, byteArrayChunk1);
    assert(buffer6.getNumRegions() == 1);
    assert(buffer6.getRegionData(0) != nullptr);

    // 7. completely overwriting a chunk
    ChunkBuffer buffer7;
    auto byteCountChunk4 = makeImmutableByteCountChunk(8);
    buffer7.replace(1, byteCountChunk4);
    buffer7.replace(0, byteArrayChunk1);
    const auto& bytesChunk1 = std::dynamic_pointer_cast<BytesChunk>(buffer7.getRegionData(0));
    assert(buffer7.getNumRegions() == 1);
    assert(bytesChunk1 != nullptr);

    // 8. partially overwriting multiple chunks
    ChunkBuffer buffer8;
    buffer8.replace(0, byteCountChunk1);
    buffer8.replace(10, byteCountChunk1);
    buffer8.replace(3, byteArrayChunk1);
    assert(buffer8.getNumRegions() == 1);
    const auto& sequenceChunk1 = std::dynamic_pointer_cast<SequenceChunk>(buffer8.getRegionData(0));
    assert(sequenceChunk1 != nullptr);
    sequenceChunk1->markImmutable();
    const auto& byteCountChunk5 = std::dynamic_pointer_cast<ByteCountChunk>(sequenceChunk1->peek(0, 3));
    assert(byteCountChunk5 != nullptr);
    assert(byteCountChunk5->getChunkLength() == 3);
    const auto& byteCountChunk6 = std::dynamic_pointer_cast<ByteCountChunk>(sequenceChunk1->peek(13, 7));
    assert(byteCountChunk6 != nullptr);
    assert(byteCountChunk6->getChunkLength() == 7);
    const auto& bytesChunk2 = std::dynamic_pointer_cast<BytesChunk>(sequenceChunk1->peek(3, 10));
    assert(bytesChunk2 != nullptr);
    assert(std::equal(bytesChunk2->getBytes().begin(), bytesChunk2->getBytes().end(), makeVector(10).begin()));

    // 9. random test
    bool debug = false;
    cLCG32 random;
    uint64_t bufferSize = 1000;
    uint64_t maxChunkLength = 100;
    ChunkBuffer buffer9;
    int *buffer10 = new int[bufferSize];
    memset(buffer10, -1, bufferSize * sizeof(int));
    for (int c = 0; c < 1000; c++) {
        // replace data
        int64_t chunkOffset = random.intRand(bufferSize - maxChunkLength);
        int64_t chunkLength = random.intRand(maxChunkLength) + 1;
        auto chunk = std::make_shared<BytesChunk>();
        std::vector<uint8_t> bytes;
        for (int i = 0; i < chunkLength; i++)
            bytes.push_back(i & 0xFF);
        chunk->setBytes(bytes);
        chunk->markImmutable();
        if (debug)
            std::cout << "Replace " << c << ": offset = " << chunkOffset << ", chunk = " << chunk << std::endl;
        buffer9.replace(chunkOffset, chunk);
        for (int i = 0; i < chunkLength; i++)
            *(buffer10 + chunkOffset + i) = i & 0xFF;

        // clear data
        chunkOffset = random.intRand(bufferSize - maxChunkLength);
        chunkLength = random.intRand(maxChunkLength) + 1;
        buffer9.clear(chunkOffset, chunkLength);
        for (int i = 0; i < chunkLength; i++)
            *(buffer10 + chunkOffset + i) = -1;

        // compare data
        if (debug) {
            std::cout << "ChunkBuffer: " << buffer9 << std::endl;
            std::cout << "PlainBuffer: ";
            for (int i = 0; i < bufferSize; i++)
                printf("%d", *(buffer10 + i));
            std::cout << std::endl << std::endl;
        }
        int64_t previousEndOffset = 0;
        for (int i = 0; i < buffer9.getNumRegions(); i++) {
            auto data = std::dynamic_pointer_cast<BytesChunk>(buffer9.getRegionData(i));
            auto startOffset = buffer9.getRegionStartOffset(i);
            for (int j = previousEndOffset; j < startOffset; j++)
                assert(*(buffer10 + j) == -1);
            for (int j = 0; j < data->getChunkLength(); j++)
                assert(data->getByte(j) == *(buffer10 + startOffset + j));
            previousEndOffset = startOffset + data->getChunkLength();
        }
        for (int j = previousEndOffset; j < bufferSize; j++)
            assert(*(buffer10 + j) == -1);
    }
    delete [] buffer10;
}

static void testReassemblyBuffer()
{
    // 1. single chunk
    ReassemblyBuffer buffer1(10);
    auto byteCountChunk1 = makeImmutableByteCountChunk(10);
    buffer1.replace(0, byteCountChunk1);
    assert(buffer1.isComplete());
    const auto& data1 = buffer1.getData();
    assert(data1 != nullptr);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(data1) != nullptr);
    assert(data1->getChunkLength() == 10);

    // 2. consecutive chunks
    ReassemblyBuffer buffer2(20);
    buffer2.replace(0, byteCountChunk1);
    assert(!buffer2.isComplete());
    buffer2.replace(10, byteCountChunk1);
    assert(buffer2.isComplete());
    const auto& data2 = buffer2.getData();
    assert(data2 != nullptr);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(data2) != nullptr);
    assert(data2->getChunkLength() == 20);

    // 3. out of order consecutive chunks
    ReassemblyBuffer buffer3(30);
    buffer3.replace(0, byteCountChunk1);
    assert(!buffer3.isComplete());
    buffer3.replace(20, byteCountChunk1);
    assert(!buffer3.isComplete());
    buffer3.replace(10, byteCountChunk1);
    assert(buffer3.isComplete());
    const auto& data3 = buffer3.getData();
    assert(data3 != nullptr);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(data3) != nullptr);
    assert(data3->getChunkLength() == 30);
}

static void testReorderBuffer()
{
    // 1. single chunk
    ReorderBuffer buffer1(1000);
    auto byteCountChunk1 = makeImmutableByteCountChunk(10);
    buffer1.replace(1000, byteCountChunk1);
    const auto& data1 = buffer1.popData();
    assert(data1 != nullptr);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(data1) != nullptr);
    assert(data1->getChunkLength() == 10);
    assert(buffer1.getExpectedOffset() == 1010);

    // 2. consecutive chunks
    ReorderBuffer buffer2(1000);
    buffer2.replace(1000, byteCountChunk1);
    buffer2.replace(1010, byteCountChunk1);
    const auto& data2 = buffer2.popData();
    assert(data2 != nullptr);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(data2) != nullptr);
    assert(data2->getChunkLength() == 20);
    assert(buffer2.getExpectedOffset() == 1020);

    // 3. out of order consecutive chunks
    ReorderBuffer buffer3(1000);
    buffer3.replace(1020, byteCountChunk1);
    assert(buffer2.popData() == nullptr);
    buffer3.replace(1000, byteCountChunk1);
    buffer3.replace(1010, byteCountChunk1);
    const auto& data3 = buffer3.popData();
    assert(data3 != nullptr);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(data3) != nullptr);
    assert(data3->getChunkLength() == 30);
    assert(buffer3.getExpectedOffset() == 1030);
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
    testHeader();
    testTrailer();
    testEncapsulation();
    testAggregation();
    testFragmentation();
    testPolymorphism();
    testSerialization();
    testIteration();
    testCorruption();
    testDuplication();
    testDuality();
    testMerging();
    testSlicing();
    testNesting();
    testPeeking();
    testSequence();
    testChunkQueue();
    testChunkBuffer();
    testReassemblyBuffer();
    testReorderBuffer();
}

} // namespace
