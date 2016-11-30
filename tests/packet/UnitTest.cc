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
#include "inet/common/packet/SerializerRegistry.h"
#include "NewTest.h"
#include "UnitTest_m.h"
#include "UnitTest.h"

namespace inet {

Register_Serializer(CompoundHeader, CompoundHeaderSerializer);
Register_Serializer(TlvHeader, TlvHeaderSerializer);
Register_Serializer(TlvHeader1, TlvHeader1Serializer);
Register_Serializer(TlvHeader2, TlvHeader2Serializer);
Define_Module(UnitTest);

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
    // TODO: do not allow appending mutable chunks?
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
    // 2. chunk is immutable after making a packet immutable
    Packet packet2;
    auto lengthChunk2 = std::make_shared<LengthChunk>(10);
    packet2.append(lengthChunk2);
    packet2.makeImmutable();
    assert(packet2.isImmutable());
    assert(lengthChunk2->isImmutable());
}

static void testComplete()
{
    // 1. chunk is complete after construction
    auto lengthChunk1 = std::make_shared<LengthChunk>(10);
    assert(lengthChunk1->isComplete());
}

static void testIncomplete()
{
    // TODO: how does one get a variable length incomplete chunk?
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
    // TODO: what does getChunkLength() return internally for an incomplete header?
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
    // TODO: check length field and fields out of 8 bytes
}

static void testCorrect()
{
    // 1. chunk is correct after construction
    auto lengthChunk1 = std::make_shared<LengthChunk>(10);
    assert(lengthChunk1->isCorrect());
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

static void testRepresented()
{
    // 1. chunk is represented after construction
    auto lengthChunk1 = std::make_shared<LengthChunk>(10);
    assert(lengthChunk1->isRepresented());
}

static void testMisrepresented()
{
    // 1. chunk is misrepresented after deserialization of erroneous packet
    Packet packet1;
    auto ipHeader1 = std::make_shared<IpHeader>();
    packet1.append(ipHeader1);
    packet1.makeImmutable();
    assert(ipHeader1->isRepresented());
    auto bytesChunk1 = std::static_pointer_cast<BytesChunk>(packet1.peekAt<BytesChunk>(0)->dupShared());
    bytesChunk1->setByte(0, 42);
    Packet packet2(nullptr, bytesChunk1);
    const auto& ipHeader2 = packet2.peekHeader<IpHeader>();
    assert(ipHeader2->isMisrepresented());
    // TODO: somewhat dubious
}

static void testHeader()
{
    // 1. packet contains header after chunk is appended
    Packet packet1;
    packet1.pushHeader(std::make_shared<LengthChunk>(10));
    // TODO: peek default representation too
    const auto& lengthChunk1 = packet1.peekHeader<LengthChunk>();
    assert(lengthChunk1 != nullptr);
    // 2. packet moves header pointer after pop
    // TODO: check return value
    const auto& xxx = packet1.popHeader<LengthChunk>();
    assert(packet1.getHeaderPopOffset() == 10);
    // 3. packet provides headers in reverse prepend order
    Packet packet2;
    packet2.pushHeader(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet2.pushHeader(std::make_shared<LengthChunk>(10));
    const auto& lengthChunk2 = packet2.popHeader<LengthChunk>();
    const auto& bytesChunk1 = packet2.popHeader<BytesChunk>();
    assert(lengthChunk2 != nullptr);
    assert(lengthChunk2->getChunkLength() == 10);
    assert(bytesChunk1 != nullptr);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}).begin()));
}

static void testTrailer()
{
    // TODO: repeat as above
    // 1. packet contains trailer after chunk is appended
    Packet packet1;
    packet1.pushTrailer(std::make_shared<LengthChunk>(10));
    const auto& lengthChunk1 = packet1.peekTrailer<LengthChunk>();
    assert(lengthChunk1 != nullptr);
    // 2. packet moves trailer pointer after pop
    packet1.popTrailer<LengthChunk>();
    assert(packet1.getTrailerPopOffset() == 0);
    // 3. packet provides trailers in reverse order
    Packet packet2;
    packet2.pushTrailer(std::make_shared<LengthChunk>(10));
    packet2.pushTrailer(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& bytesChunk1 = packet2.popTrailer<BytesChunk>();
    const auto& lengthChunk2 = packet2.popTrailer<LengthChunk>();
    assert(bytesChunk1 != nullptr);
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}).begin()));
    assert(lengthChunk2 != nullptr);
    assert(lengthChunk2->getChunkLength() == 10);
}

static void testEncapsulation()
{
    // 1. packet contains all chunks of encapsulated packet as is
    Packet packet1;
    packet1.append(std::make_shared<LengthChunk>(10));
    packet1.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    // encapsulation packet with header and trailer
    auto& packet2 = packet1;
    packet2.pushHeader(std::make_shared<EthernetHeader>());
    packet2.pushTrailer(std::make_shared<EthernetTrailer>());
    packet2.makeImmutable();
    const auto& ethernetHeader1 = packet2.popHeader<EthernetHeader>();
    const auto& ethernetTrailer1 = packet2.popTrailer<EthernetTrailer>();
    // TODO: this test uses the default representation
    const auto& lengthChunk1 = std::dynamic_pointer_cast<LengthChunk>(packet2.peekDataAt(0, 10));
    const auto& bytesChunk1 = std::dynamic_pointer_cast<BytesChunk>(packet2.peekDataAt(10, 10));
    assert(ethernetHeader1 != nullptr);
    assert(ethernetTrailer1 != nullptr);
    assert(lengthChunk1 != nullptr);
    assert(lengthChunk1->getChunkLength() == 10);
    assert(bytesChunk1 != nullptr);
    assert(bytesChunk1->getChunkLength() == 10);
}

static void testAggregation()
{
    // 1. packet contains all chunks of aggregated packets as is
    Packet packet1;
    packet1.append(std::make_shared<LengthChunk>(10));
    packet1.makeImmutable();
    Packet packet2;
    packet2.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet2.makeImmutable();
    Packet packet3;
    packet3.append(std::make_shared<IpHeader>());
    // aggregate other packets
    // TODO: in generic version no default offset please
    packet3.append(packet1.peekAt(0, packet2.getPacketLength()));
    packet3.append(packet2.peekAt(0, packet2.getPacketLength()));
    packet3.makeImmutable();
    const auto& ipHeader1 = packet3.popHeader<IpHeader>();
    const auto& lengthChunk1 = std::dynamic_pointer_cast<LengthChunk>(packet3.peekDataAt(0, 10));
    const auto& bytesChunk1 = std::dynamic_pointer_cast<BytesChunk>(packet3.peekDataAt(10, 10));
    assert(ipHeader1 != nullptr);
    assert(lengthChunk1 != nullptr);
    assert(lengthChunk1->getChunkLength() == 10);
    assert(bytesChunk1 != nullptr);
    assert(bytesChunk1->getChunkLength() == 10);
}

static void testFragmentation()
{
    // 1. packet contains fragment of another packet
    Packet packet1;
    packet1.append(std::make_shared<LengthChunk>(10));
    packet1.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet1.makeImmutable();
    Packet packet2;
    packet2.append(std::make_shared<IpHeader>());
    // append fragment of another packet
    packet2.append(packet1.peekAt(5, 10));
    packet2.makeImmutable();
    const auto& ipHeader1 = packet2.popHeader<IpHeader>();
    const auto& fragment1 = packet2.peekDataAt(0, packet2.getDataLength());
    // TODO: dynamic cast default representation and check contents
    const auto& lengthChunk1 = fragment1->peek(0, 5);
    const auto& bytesChunk1 = fragment1->peek(5, 5);
    assert(ipHeader1 != nullptr);
    assert(fragment1 != nullptr);
    assert(lengthChunk1 != nullptr);
    assert(lengthChunk1->getChunkLength() == 5);
    assert(bytesChunk1 != nullptr);
    assert(bytesChunk1->getChunkLength() == 5);
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
    // TODO: move dynamic cast to a separate assert
    const auto& tlvHeader3 = std::dynamic_pointer_cast<TlvHeader1>(packet1.popHeader<TlvHeader>());
    assert(tlvHeader3 != nullptr);
    assert(tlvHeader3->getBoolValue());
    const auto& tlvHeader4 = std::dynamic_pointer_cast<TlvHeader2>(packet1.popHeader<TlvHeader>());
    assert(tlvHeader4 != nullptr);
    assert(tlvHeader4->getInt16Value() == 42);
    // 2. packet provides deserialized headers in a polymorphic way after serialization
    Packet packet2(nullptr, packet1.peekAt<BytesChunk>(0, packet1.getPacketLength()));
    const auto& tlvHeader5 = std::dynamic_pointer_cast<TlvHeader1>(packet2.popHeader<TlvHeader>());
    assert(tlvHeader5 != nullptr);
    assert(tlvHeader5->getBoolValue());
    const auto& tlvHeader6 = std::dynamic_pointer_cast<TlvHeader2>(packet2.popHeader<TlvHeader>());
    assert(tlvHeader6 != nullptr);
    assert(tlvHeader6->getInt16Value() == 42);
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
    // TODO: separate subtests with a new line

    // 2. serialized bytes is cached after deserialization
    ByteInputStream stream2(stream1.getBytes());
    auto totalDeserializedBytes = ChunkSerializer::totalDeserializedBytes;
    auto applicationHeader2 = std::dynamic_pointer_cast<ApplicationHeader>(Chunk::deserialize(stream2, typeid(ApplicationHeader)));
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

static void testDuality()
{
    // 1. packet provides header in both fields and bytes representation
    Packet packet1;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
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
    // TODO: compare chunks in all combinations
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
    // TODO: default representation
//    const auto& applicationHeader2 = packet2.peekHeader();
    const auto& applicationHeader2 = packet2.peekHeader<ApplicationHeader>();
    assert(applicationHeader2->isComplete());
    assert(applicationHeader2->getChunkLength() == 10);
    // 2. packet provides compacts headers
    Packet packet3;
    packet3.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet3.append(std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    packet3.makeImmutable();
    const auto& bytesChunk1 = std::dynamic_pointer_cast<BytesChunk>(packet3.peekAt(0, packet3.getPacketLength()));
    assert(bytesChunk1 != nullptr);
    assert(bytesChunk1->getChunkLength());
    assert(std::equal(bytesChunk1->getBytes().begin(), bytesChunk1->getBytes().end(), std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9}).begin()));
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
    // TODO: packet peek is misleading with BytesChunk and default length
    // TODO: consider introducing an enum to replace -1 length values: UNTIL_END, INTERNAL_REP
    Packet packet2(nullptr, packet1.peekAt<BytesChunk>(0, packet1.getPacketLength())); // TODO: 0, packet1.getPacketLength()));
    const auto& compoundHeader3 = packet2.peekHeader<CompoundHeader>();
    assert(compoundHeader3 != nullptr);
    auto it = Chunk::ForwardIterator(0, 0);
    const auto& ipHeader2 = compoundHeader3->Chunk::peek<IpHeader>(it);
    assert(ipHeader2 != nullptr);
    assert(ipHeader2->getProtocol() == Protocol::Tcp);
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
    sequenceChunk1->insertAtEnd(lengthChunk1);
    sequenceChunk1->insertAtEnd(bytesChunk1);
    sequenceChunk1->insertAtEnd(applicationHeader1);
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
    // TODO: check chunk in slice is the original sequence
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

static void testDuplication()
{
    // 1. copy of immutable packet shares data
    Packet packet1;
    std::shared_ptr<LengthChunk> lengthChunk1 = std::make_shared<LengthChunk>(10);
    packet1.append(lengthChunk1);
    packet1.makeImmutable();
    auto packet2 = packet1.dup();
    assert(packet2->getPacketLength() == 10);
    // 1 in the chunk + 2 in the packets
    assert(lengthChunk1.use_count() == 3);
    delete packet2;
    // 2. copy of mutable packet copies data
    Packet packet3;
    auto lengthChunk2 = std::make_shared<LengthChunk>(10);
    packet3.append(lengthChunk2);
    auto packet4 = packet3.dup();
    lengthChunk2->setLength(20);
    assert(packet4->getPacketLength() == 10);
    // 1 in the chunk + 1 in the original packet
    assert(lengthChunk2.use_count() == 2);
    delete packet4;
    // 3. copy of immutable chunk in mutable packet shares data
    Packet packet5;
    std::shared_ptr<LengthChunk> lengthChunk3 = std::make_shared<LengthChunk>(10);
    packet5.append(lengthChunk3);
    lengthChunk3->makeImmutable();
    auto packet6 = packet5.dup();
    assert(packet6->getPacketLength() == 10);
    // 1 in the chunk + 2 in the packets
    assert(lengthChunk3.use_count() == 3);
    delete packet6;
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
    // TODO: more complete, use ApplicationHeader and slices
}

static void testReassemblyBuffer()
{
    // 1. single chunk
    ReassemblyBuffer buffer1;
    buffer1.replace(0, std::make_shared<LengthChunk>(10));
    // TODO: rename to isContiguous()
    assert(buffer1.getNumRegions() == 1);
    assert(buffer1.getRegionData(0) != nullptr);
    // 2. consecutive chunks
    ReassemblyBuffer buffer2;
    buffer2.replace(0, std::make_shared<LengthChunk>(10));
    buffer2.replace(10, std::make_shared<LengthChunk>(10));
    const auto& lengthChunk1 = std::dynamic_pointer_cast<LengthChunk>(buffer2.getRegionData(0));
    assert(buffer2.getNumRegions() == 1);
    assert(lengthChunk1 != nullptr);
    // 3. consecutive slice chunks
    ReassemblyBuffer buffer3;
    auto applicationHeader1 = std::make_shared<ApplicationHeader>();
    applicationHeader1->makeImmutable();
    // TODO: dup is needed to make the chunk mutable and to be able to merge it
    buffer3.replace(0, applicationHeader1->peek(0, 5)->dupShared());
    buffer3.replace(5, applicationHeader1->peek(5, 5)->dupShared());
    const auto& applicationHeader2 = std::dynamic_pointer_cast<ApplicationHeader>(buffer3.getRegionData(0));
    assert(buffer3.getNumRegions() == 1);
    assert(applicationHeader2 != nullptr);
    // 4. out of order consecutive chunks
    ReassemblyBuffer buffer4;
    buffer4.replace(0, std::make_shared<LengthChunk>(10));
    buffer4.replace(20, std::make_shared<LengthChunk>(10));
    buffer4.replace(10, std::make_shared<LengthChunk>(10));
    const auto& lengthChunk2 = std::dynamic_pointer_cast<LengthChunk>(buffer4.getRegionData(0));
    assert(buffer4.getNumRegions() == 1);
    assert(lengthChunk2 != nullptr);
    // 5. heterogeneous chunks
    ReassemblyBuffer buffer5;
    buffer5.replace(0, std::make_shared<LengthChunk>(10));
    buffer5.replace(10, std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    assert(buffer5.getNumRegions() == 1);
    assert(buffer5.getRegionData(0) != nullptr);
    // 6. completely overwriting a chunk
    ReassemblyBuffer buffer6;
    buffer6.replace(1, std::make_shared<LengthChunk>(8));
    buffer6.replace(0, std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& bytesChunk1 = std::dynamic_pointer_cast<BytesChunk>(buffer6.getRegionData(0));
    assert(buffer6.getNumRegions() == 1);
    assert(bytesChunk1 != nullptr);
    // 7. partially overwriting multiple chunks
    ReassemblyBuffer buffer7;
    buffer7.replace(0, std::make_shared<LengthChunk>(10));
    buffer7.replace(10, std::make_shared<LengthChunk>(10));
    buffer7.replace(5, std::make_shared<BytesChunk>(std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})));
    const auto& sequenceChunk1 = std::dynamic_pointer_cast<SequenceChunk>(buffer7.getRegionData(0));
    sequenceChunk1->makeImmutable();
    const auto& bytesChunk2 = std::dynamic_pointer_cast<BytesChunk>(sequenceChunk1->peek(5, 10));
    assert(buffer7.getNumRegions() == 1);
    assert(sequenceChunk1 != nullptr);
    assert(bytesChunk2 != nullptr);
}

void UnitTest::initialize()
{
    testMutable();
    testImmutable();
    testComplete();
    testIncomplete();
    testCorrect();
    testIncorrect();
    testRepresented();
    testMisrepresented();
    testHeader();
    testTrailer();
    testEncapsulation();
    testAggregation();
    testFragmentation();
    testPolymorphism();
    testSerialization();
    testDuplication();
    testDuality();
    testMerging();
    testNesting();
    testPeekChunk();
    testPeekPacket();
    testPeekBuffer();
    testReassemblyBuffer();

    // TODO: error model example (think MPDU aggregation)
}

} // namespace
