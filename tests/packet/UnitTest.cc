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
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/RegionedChunkBuffer.h"
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
    // 1. packet is mutable after construction
    Packet packet1;
    assert(packet1.isMutable());

    // 2. chunk is mutable after construction
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    assert(byteCountChunk1->isMutable());

    // 3. packet and chunk are still mutable after appending
    packet1.append(byteCountChunk1);
    assert(packet1.isMutable());
    assert(byteCountChunk1->isMutable());
}

static void testImmutable()
{
    // 1. chunk is immutable after making it immutable
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    byteCountChunk1->makeImmutable();
    assert(byteCountChunk1->isImmutable());

    // 2. chunk is immutable after making a packet immutable
    Packet packet2;
    auto byteCountChunk2 = std::make_shared<ByteCountChunk>(10);
    packet2.append(byteCountChunk2);
    packet2.makeImmutable();
    assert(packet2.isImmutable());
    assert(byteCountChunk2->isImmutable());
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
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    packet1.append(applicationHeader1);
    packet1.makeImmutable();
    Packet fragment1;
    fragment1.append(packet1.peekAt(0, 5));
    fragment1.makeImmutable();
    assert(!fragment1.hasHeader<ApplicationHeader>());
    const auto& applicationHeader2 = fragment1.peekHeader<ApplicationHeader>();
    assert(applicationHeader2 == nullptr);

    // 2. packet provides incomplete variable length header if requested
    Packet packet2;
    auto tcpHeader1 = std::make_shared<TcpHeader>();
    tcpHeader1->setChunkLength(16);
    tcpHeader1->setLengthField(16);
    tcpHeader1->setBitError(BIT_ERROR_CRC);
    tcpHeader1->setSrcPort(1000);
    tcpHeader1->setDestPort(1000);
    packet2.append(tcpHeader1);
    packet2.makeImmutable();
    const auto& tcpHeader2 = packet2.popHeader<TcpHeader>(4);
    assert(tcpHeader2->isIncomplete());
    assert(tcpHeader2->getChunkLength() == 4);
    assert(tcpHeader2->getBitError() == BIT_ERROR_CRC);
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
    // 1. chunk is incorrect after going through lossy channel
    Packet packet1;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    packet1.append(applicationHeader1);
    packet1.makeImmutable();
    applicationHeader1->makeIncorrect();
    assert(applicationHeader1->isIncorrect());
}

static void testProper()
{
    // 1. chunk is proper after construction
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    assert(byteCountChunk1->isProper());
}

static void testImproper()
{
    // 1. chunk is improper after deserialization of a non-representable packet
    Packet packet1;
    auto ipHeader1 = std::make_shared<IpHeader>();
    packet1.append(ipHeader1);
    packet1.makeImmutable();
    assert(ipHeader1->isProper());
    auto bytesChunk1 = std::static_pointer_cast<BytesChunk>(packet1.peekAt<BytesChunk>(0, packet1.getPacketLength())->dupShared());
    bytesChunk1->setByte(0, 42);
    Packet packet2(nullptr, bytesChunk1);
    const auto& ipHeader2 = packet2.peekHeader<IpHeader>();
    assert(ipHeader2->isImproper());
}

static void testHeader()
{
    // 1. packet contains header after chunk is appended
    Packet packet1;
    packet1.pushHeader(std::make_shared<ByteCountChunk>(10));
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
    packet2.pushHeader(std::make_shared<BytesChunk>(makeVector(10)));
    packet2.pushHeader(std::make_shared<ByteCountChunk>(10));
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
    packet1.pushTrailer(std::make_shared<ByteCountChunk>(10));
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
    packet2.pushTrailer(std::make_shared<BytesChunk>(makeVector(10)));
    packet2.pushTrailer(std::make_shared<ByteCountChunk>(10));
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
    packet1.append(std::make_shared<ByteCountChunk>(10));
    packet1.append(std::make_shared<BytesChunk>(makeVector(10)));
    const auto& dataChunk1 = packet1.peekDataAt<BytesChunk>(0, packet1.getDataLength());
    // encapsulation packet with header and trailer
    auto& packet2 = packet1;
    packet2.pushHeader(std::make_shared<EthernetHeader>());
    packet2.pushTrailer(std::make_shared<EthernetTrailer>());
    packet2.makeImmutable();
    const auto& ethernetHeader1 = packet2.popHeader<EthernetHeader>();
    const auto& ethernetTrailer1 = packet2.popTrailer<EthernetTrailer>();
    const auto& byteCountChunk1 = packet2.peekDataAt(0, 10);
    const auto& bytesChunk1 = packet2.peekDataAt(10, 10);
    const auto& dataChunk2 = packet2.peekDataAt<BytesChunk>(0, packet2.getDataLength());
    assert(ethernetHeader1 != nullptr);
    assert(ethernetTrailer1 != nullptr);
    assert(byteCountChunk1 != nullptr);
    assert(bytesChunk1 != nullptr);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(byteCountChunk1) != nullptr);
    assert(std::dynamic_pointer_cast<BytesChunk>(bytesChunk1) != nullptr);
    assert(byteCountChunk1->getChunkLength() == 10);
    assert(bytesChunk1->getChunkLength() == 10);
    assert(std::equal(dataChunk1->getBytes().begin(), dataChunk1->getBytes().end(), dataChunk2->getBytes().begin()));
}

static void testAggregation()
{
    // 1. packet contains all chunks of aggregated packets as is
    Packet packet1;
    packet1.append(std::make_shared<ByteCountChunk>(10));
    packet1.makeImmutable();
    Packet packet2;
    packet2.append(std::make_shared<BytesChunk>(makeVector(10)));
    packet2.makeImmutable();
    Packet packet3;
    packet3.append(std::make_shared<IpHeader>());
    // aggregate other packets
    packet3.append(packet1.peekAt(0, packet2.getPacketLength()));
    packet3.append(packet2.peekAt(0, packet2.getPacketLength()));
    packet3.makeImmutable();
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
    packet1.append(std::make_shared<ByteCountChunk>(10));
    packet1.append(std::make_shared<BytesChunk>(makeVector(10)));
    packet1.makeImmutable();
    Packet packet2;
    packet2.append(std::make_shared<IpHeader>());
    // append fragment of another packet
    packet2.append(packet1.peekAt(5, 10));
    packet2.makeImmutable();
    const auto& ipHeader1 = packet2.popHeader<IpHeader>();
    const auto& fragment1 = packet2.peekDataAt(0, packet2.getDataLength());
    const auto& chunk1 = fragment1->peek(0, 5);
    const auto& chunk2 = fragment1->peek(5, 5);
    assert(ipHeader1 != nullptr);
    assert(std::dynamic_pointer_cast<IpHeader>(ipHeader1) != nullptr);
    assert(fragment1 != nullptr);
    assert(fragment1->getChunkLength() == 10);
    assert(chunk1 != nullptr);
    assert(chunk1->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk1) != nullptr);
    assert(chunk2 != nullptr);
    assert(chunk2->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<BytesChunk>(chunk2) != nullptr);
    const auto& bytesChunk1 = std::static_pointer_cast<BytesChunk>(chunk2);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), makeVector(5).begin()));
}

static void testPolymorphism()
{
    // 1. packet provides headers in a polymorphic way without serialization
    Packet packet1;
    auto tlvHeader1 = std::make_shared<TlvHeaderBool>();
    tlvHeader1->setBoolValue(true);
    packet1.append(tlvHeader1);
    auto tlvHeader2 = std::make_shared<TlvHeaderInt>();
    tlvHeader2->setInt16Value(42);
    packet1.append(tlvHeader2);
    packet1.makeImmutable();
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
    packet1.append(std::make_shared<ByteCountChunk>(10));
    packet1.append(std::make_shared<BytesChunk>(makeVector(10)));
    packet1.append(std::make_shared<ApplicationHeader>());
    packet1.makeImmutable();
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
    sequenceChunk1->insertAtEnd(std::make_shared<ByteCountChunk>(10));
    sequenceChunk1->insertAtEnd(std::make_shared<BytesChunk>(makeVector(10)));
    sequenceChunk1->insertAtEnd(std::make_shared<ApplicationHeader>());
    sequenceChunk1->makeImmutable();
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
    sequenceChunk2->insertAtEnd(std::make_shared<ByteCountChunk>(10));
    sequenceChunk2->insertAtEnd(std::make_shared<BytesChunk>(makeVector(10)));
    sequenceChunk2->insertAtEnd(std::make_shared<ApplicationHeader>());
    sequenceChunk2->makeImmutable();
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
    const auto& chunk1 = std::make_shared<ByteCountChunk>(10);
    const auto& chunk2 = std::make_shared<BytesChunk>(makeVector(10));
    const auto& chunk3 = std::make_shared<ApplicationHeader>();
    packet1.append(chunk1);
    packet1.append(chunk2);
    packet1.append(chunk3);
    packet1.makeImmutable();
    int index = 0;
    auto chunk = packet1.popHeader();
    while (chunk != nullptr) {
        int64_t bitLength = 8 * chunk->getChunkLength();
        if (random[index++] >= std::pow(1 - ber, bitLength))
            chunk->makeIncorrect();
        chunk = packet1.popHeader();
    }
    assert(chunk1->isCorrect());
    assert(chunk2->isIncorrect());
    assert(chunk3->isIncorrect());
}

static void testDuplication()
{
    // 1. copy of immutable packet shares data
    Packet packet1;
    std::shared_ptr<ByteCountChunk> byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    packet1.append(byteCountChunk1);
    packet1.makeImmutable();
    auto packet2 = packet1.dup();
    assert(packet2->getPacketLength() == 10);
    assert(byteCountChunk1.use_count() == 3); // 1 in the chunk + 2 in the packets
    delete packet2;

    // 2. copy of mutable packet copies data
    Packet packet3;
    auto byteCountChunk2 = std::make_shared<ByteCountChunk>(10);
    packet3.append(byteCountChunk2);
    auto packet4 = packet3.dup();
    byteCountChunk2->setLength(20);
    assert(packet4->getPacketLength() == 10);
    assert(byteCountChunk2.use_count() == 2); // 1 in the chunk + 1 in the original packet
    delete packet4;

    // 3. copy of immutable chunk in mutable packet shares data
    Packet packet5;
    std::shared_ptr<ByteCountChunk> byteCountChunk3 = std::make_shared<ByteCountChunk>(10);
    packet5.append(byteCountChunk3);
    byteCountChunk3->makeImmutable();
    auto packet6 = packet5.dup();
    assert(packet6->getPacketLength() == 10);
    assert(byteCountChunk3.use_count() == 3); // 1 in the chunk + 2 in the packets
    delete packet6;
}

static void testDuality()
{
    // 1. packet provides header in both fields and bytes representation
    Packet packet1;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->setSomeData(42);
    packet1.append(applicationHeader1);
    packet1.makeImmutable();
    const auto& applicationHeader2 = packet1.peekHeader<ApplicationHeader>();
    const auto& bytesChunk1 = packet1.peekHeader<BytesChunk>(10);
    assert(applicationHeader2 != nullptr);
    assert(applicationHeader2->getChunkLength() == 10);
    assert(bytesChunk1 != nullptr);
    assert(bytesChunk1->getChunkLength() == 10);

    // 2. packet provides header in both fields and bytes representation after serialization
    Packet packet2(nullptr, packet1.peekAt<BytesChunk>(0, packet1.getPacketLength()));
    const auto& applicationHeader3 = packet2.peekHeader<ApplicationHeader>();
    const auto& bytesChunk2 = packet2.peekHeader<BytesChunk>(10);
    assert(applicationHeader3 != nullptr);
    assert(applicationHeader3->getChunkLength() == 10);
    assert(bytesChunk2 != nullptr);
    assert(bytesChunk2->getChunkLength() == 10);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), bytesChunk1->getBytes().begin()));
    assert(applicationHeader2->getSomeData() == applicationHeader3->getSomeData());
}

static void testMerging()
{
    // 1. packet provides complete merged header if the whole header is available
    Packet packet1;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    packet1.append(applicationHeader1);
    packet1.makeImmutable();
    const auto& header1 = packet1.peekAt(0, 5);
    const auto& header2 = packet1.peekAt(5, 5);
    Packet packet2;
    packet2.append(header1);
    packet2.append(header2);
    packet2.makeImmutable();
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
    packet3.append(std::make_shared<ByteCountChunk>(5));
    packet3.append(std::make_shared<ByteCountChunk>(5));
    packet3.makeImmutable();
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
    packet4.append(std::make_shared<BytesChunk>(makeVector(5)));
    packet4.append(std::make_shared<BytesChunk>(makeVector(5)));
    packet4.makeImmutable();
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
    auto byteCountChunk1 = std::make_shared<ByteCountChunk>(10);
    byteCountChunk1->makeImmutable();
    const auto& chunk1 = byteCountChunk1->peek(0, 10);
    const auto& chunk2 = byteCountChunk1->peek(0, 5);
    assert(chunk1 == byteCountChunk1);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk1) != nullptr);
    assert(chunk2 != nullptr);
    assert(chunk2->getChunkLength() == 5);
    assert(std::dynamic_pointer_cast<ByteCountChunk>(chunk2) != nullptr);

    // 2. BytesChunk always returns BytesChunk
    auto bytesChunk1 = std::make_shared<BytesChunk>(makeVector(10));
    bytesChunk1->makeImmutable();
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
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->makeImmutable();
    auto sliceChunk1 = std::make_shared<SliceChunk>(applicationHeader1, 0, 10);
    sliceChunk1->makeImmutable();
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
    sequenceChunk1->makeImmutable();
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
    auto applicationHeader2 = std::make_shared<ApplicationHeader>();
    applicationHeader2->makeImmutable();
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
    packet1.append(compoundHeader1);
    packet1.makeImmutable();
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
    packet1.append(std::make_shared<ByteCountChunk>(10));
    packet1.append(std::make_shared<ByteCountChunk>(10));
    packet1.append(std::make_shared<ByteCountChunk>(10));
    packet1.makeImmutable();
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
    packet2.append(std::make_shared<BytesChunk>(makeVector(10)));
    packet2.append(std::make_shared<BytesChunk>(makeVector(10)));
    packet2.append(std::make_shared<BytesChunk>(makeVector(10)));
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
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->makeImmutable();
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

static void testFifoChunkBuffer()
{
    // 1. buffer provides ByteCountChunks by default if it contains a ByteCountChunk only
    FifoChunkBuffer buffer1;
    buffer1.push(std::make_shared<ByteCountChunk>(10));
    buffer1.push(std::make_shared<ByteCountChunk>(10));
    buffer1.push(std::make_shared<ByteCountChunk>(10));
    const auto& byteCountChunk1 = std::dynamic_pointer_cast<ByteCountChunk>(buffer1.pop(15));
    const auto& byteCountChunk2 = std::dynamic_pointer_cast<ByteCountChunk>(buffer1.pop(15));
    assert(byteCountChunk1 != nullptr);
    assert(byteCountChunk2 != nullptr);

    // 2. buffer provides BytesChunks by default if it contains a BytesChunk only
    FifoChunkBuffer buffer2;
    buffer2.push(std::make_shared<BytesChunk>(makeVector(10)));
    buffer2.push(std::make_shared<BytesChunk>(makeVector(10)));
    buffer2.push(std::make_shared<BytesChunk>(makeVector(10)));
    const auto& byteCountChunk3 = std::dynamic_pointer_cast<BytesChunk>(buffer2.pop(15));
    const auto& byteCountChunk4 = std::dynamic_pointer_cast<BytesChunk>(buffer2.pop(15));
    assert(byteCountChunk3 != nullptr);
    assert(byteCountChunk4 != nullptr);

    // 3. buffer provides reassembled header
    FifoChunkBuffer buffer3;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->setSomeData(42);
    applicationHeader1->makeImmutable();
    buffer3.push(applicationHeader1->peek(0, 5));
    buffer3.push(applicationHeader1->peek(5, 5));
    assert(buffer3.has<ApplicationHeader>());
    const auto& applicationHeader2 = buffer3.pop<ApplicationHeader>();
    assert(applicationHeader2 != nullptr);
    assert(applicationHeader2->getSomeData() == 42);
}

static void testRegionedChunkBuffer()
{
    // 1. single chunk
    RegionedChunkBuffer buffer1;
    buffer1.replace(0, std::make_shared<ByteCountChunk>(10));
    assert(buffer1.getNumRegions() == 1);
    assert(buffer1.getRegionData(0) != nullptr);

    // 2. consecutive chunks
    RegionedChunkBuffer buffer2;
    buffer2.replace(0, std::make_shared<ByteCountChunk>(10));
    buffer2.replace(10, std::make_shared<ByteCountChunk>(10));
    const auto& byteCountChunk1 = std::dynamic_pointer_cast<ByteCountChunk>(buffer2.getRegionData(0));
    assert(buffer2.getNumRegions() == 1);
    assert(byteCountChunk1 != nullptr);

    // 3. consecutive slice chunks
    RegionedChunkBuffer buffer3;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->setSomeData(42);
    applicationHeader1->makeImmutable();
    buffer3.replace(0, applicationHeader1->peek(0, 5));
    buffer3.replace(5, applicationHeader1->peek(5, 5));
    const auto& applicationHeader2 = std::dynamic_pointer_cast<ApplicationHeader>(buffer3.getRegionData(0));
    assert(buffer3.getNumRegions() == 1);
    assert(applicationHeader2 != nullptr);
    assert(applicationHeader2->getSomeData() == 42);

    // 4. out of order consecutive chunks
    RegionedChunkBuffer buffer4;
    buffer4.replace(0, std::make_shared<ByteCountChunk>(10));
    buffer4.replace(20, std::make_shared<ByteCountChunk>(10));
    buffer4.replace(10, std::make_shared<ByteCountChunk>(10));
    const auto& byteCountChunk2 = std::dynamic_pointer_cast<ByteCountChunk>(buffer4.getRegionData(0));
    assert(buffer4.getNumRegions() == 1);
    assert(byteCountChunk2 != nullptr);

    // 5. out of order consecutive chunks
    RegionedChunkBuffer buffer5;
    buffer5.replace(0, applicationHeader1->peek(0, 3));
    buffer5.replace(7, applicationHeader1->peek(7, 3));
    buffer5.replace(3, applicationHeader1->peek(3, 4));
    const auto& applicationHeader3 = std::dynamic_pointer_cast<ApplicationHeader>(buffer5.getRegionData(0));
    assert(buffer5.getNumRegions() == 1);
    assert(applicationHeader3 != nullptr);
    assert(applicationHeader3->getSomeData() == 42);

    // 6. heterogeneous chunks
    RegionedChunkBuffer buffer6;
    buffer6.replace(0, std::make_shared<ByteCountChunk>(10));
    buffer6.replace(10, std::make_shared<BytesChunk>(makeVector(10)));
    assert(buffer6.getNumRegions() == 1);
    assert(buffer6.getRegionData(0) != nullptr);

    // 7. completely overwriting a chunk
    RegionedChunkBuffer buffer7;
    buffer7.replace(1, std::make_shared<ByteCountChunk>(8));
    buffer7.replace(0, std::make_shared<BytesChunk>(makeVector(10)));
    const auto& bytesChunk1 = std::dynamic_pointer_cast<BytesChunk>(buffer7.getRegionData(0));
    assert(buffer7.getNumRegions() == 1);
    assert(bytesChunk1 != nullptr);

    // 8. partially overwriting multiple chunks
    RegionedChunkBuffer buffer8;
    buffer8.replace(0, std::make_shared<ByteCountChunk>(10));
    buffer8.replace(10, std::make_shared<ByteCountChunk>(10));
    buffer8.replace(5, std::make_shared<BytesChunk>(makeVector(10)));
    const auto& sequenceChunk1 = std::dynamic_pointer_cast<SequenceChunk>(buffer8.getRegionData(0));
    sequenceChunk1->makeImmutable();
    const auto& bytesChunk2 = std::dynamic_pointer_cast<BytesChunk>(sequenceChunk1->peek(5, 10));
    assert(buffer7.getNumRegions() == 1);
    assert(sequenceChunk1 != nullptr);
    assert(bytesChunk2 != nullptr);
    assert(std::equal(bytesChunk2->getBytes().begin(), bytesChunk2->getBytes().end(), makeVector(10).begin()));
}

void UnitTest::initialize()
{
    testMutable();
    testImmutable();
    testComplete();
    testIncomplete();
    testCorrect();
    testIncorrect();
    testProper();
    testImproper();
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
    testFifoChunkBuffer();
    testRegionedChunkBuffer();
}

} // namespace
