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

Register_Class(CompoundHeaderSerializer);
Register_Class(TlvHeaderSerializer);
Register_Class(TlvHeader1Serializer);
Register_Class(TlvHeader2Serializer);
Define_Module(UnitTest);

std::shared_ptr<Chunk> CompoundHeaderSerializer::deserialize(ByteInputStream& stream)
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

std::shared_ptr<Chunk> TlvHeaderSerializer::deserialize(ByteInputStream& stream)
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

std::shared_ptr<Chunk> TlvHeader1Serializer::deserialize(ByteInputStream& stream)
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

std::shared_ptr<Chunk> TlvHeader2Serializer::deserialize(ByteInputStream& stream)
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
    const auto& byteArrayChunk1 = packet1.peekHeaderAt<ByteArrayChunk>(0);
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
    // 1. TODO
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
    // 2. TODO
    const auto& byteArrayChunk1 = packet1.peekHeaderAt<ByteArrayChunk>(0);
    Packet packet2;
    packet2.append(byteArrayChunk1);
    const auto& tlvHeader5 = std::dynamic_pointer_cast<TlvHeader1>(packet2.popHeader<TlvHeader>());
    assert(tlvHeader5 != nullptr);
    assert(tlvHeader5->getBoolValue());
    const auto& tlvHeader6 = std::dynamic_pointer_cast<TlvHeader2>(packet2.popHeader<TlvHeader>());
    assert(tlvHeader6 != nullptr);
    assert(tlvHeader6->getInt16Value() == 42);
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
}

} // namespace
