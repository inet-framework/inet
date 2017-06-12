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
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "NewTest.h"

namespace inet {

Register_Serializer(ApplicationHeader, ApplicationHeaderSerializer);
Register_Serializer(TcpHeader, TcpHeaderSerializer);
Register_Serializer(IpHeader, IpHeaderSerializer);
Register_Serializer(EthernetHeader, EthernetHeaderSerializer);
Register_Serializer(EthernetTrailer, EthernetTrailerSerializer);
Define_Module(NewTest);

static int16_t computeTcpCrc(const BytesChunk& pseudoHeader, const Ptr<TcpHeader>& tcpHeader, const Ptr<BytesChunk>& tcpSegment)
{
    return 42;
}

void ApplicationHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& applicationHeader = std::static_pointer_cast<const ApplicationHeader>(chunk);
    auto position = stream.getLength();
    stream.writeUint16Be(applicationHeader->getSomeData());
    stream.writeByteRepeatedly(0, byte(applicationHeader->getChunkLength() - stream.getLength() + position).get());
}

Ptr<Chunk> ApplicationHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto applicationHeader = std::make_shared<ApplicationHeader>();
    auto position = stream.getPosition();
    applicationHeader->setSomeData(stream.readUint16Be());
    stream.readByteRepeatedly(0, byte(applicationHeader->getChunkLength() - stream.getPosition() + position).get());
    return applicationHeader;
}

void TcpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& tcpHeader = std::static_pointer_cast<const TcpHeader>(chunk);
    auto position = stream.getLength();
    if (tcpHeader->getCrcMode() != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize TCP header");
    stream.writeUint16Be(tcpHeader->getLengthField());
    stream.writeUint16Be(tcpHeader->getSrcPort());
    stream.writeUint16Be(tcpHeader->getDestPort());
    stream.writeUint16Be(tcpHeader->getCrc());
    stream.writeByteRepeatedly(0, byte(tcpHeader->getChunkLength() - stream.getLength() + position).get());
}

Ptr<Chunk> TcpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto tcpHeader = std::make_shared<TcpHeader>();
    auto position = stream.getPosition();
    auto remainingLength = stream.getRemainingLength();
    auto lengthField = byte(stream.readUint16Be());
    if (lengthField > remainingLength)
        tcpHeader->markIncomplete();
    auto length = lengthField < remainingLength ? lengthField : byte(remainingLength);
    tcpHeader->setChunkLength(byte(length));
    tcpHeader->setLengthField(byte(lengthField).get());
    tcpHeader->setSrcPort(stream.readUint16Be());
    tcpHeader->setDestPort(stream.readUint16Be());
    tcpHeader->setCrcMode(CRC_COMPUTED);
    tcpHeader->setCrc(stream.readUint16Be());
    stream.readByteRepeatedly(0, byte(length - stream.getPosition() + position).get());
    return tcpHeader;
}

void IpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& ipHeader = std::static_pointer_cast<const IpHeader>(chunk);
    auto position = stream.getLength();
    stream.writeUint16Be((int16_t)ipHeader->getProtocol());
    stream.writeByteRepeatedly(0, byte(ipHeader->getChunkLength() - stream.getLength() + position).get());
}

Ptr<Chunk> IpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ipHeader = std::make_shared<IpHeader>();
    auto position = stream.getPosition();
    Protocol protocol = (Protocol)stream.readUint16Be();
    if (protocol != Protocol::Tcp && protocol != Protocol::Ip && protocol != Protocol::Ethernet)
        ipHeader->markImproperlyRepresented();
    ipHeader->setProtocol(protocol);
    stream.readByteRepeatedly(0, byte(ipHeader->getChunkLength() - stream.getPosition() + position).get());
    return ipHeader;
}

void EthernetHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& ethernetHeader = std::static_pointer_cast<const EthernetHeader>(chunk);
    auto position = stream.getLength();
    stream.writeUint16Be((int16_t)ethernetHeader->getProtocol());
    stream.writeByteRepeatedly(0, byte(ethernetHeader->getChunkLength() - stream.getLength() + position).get());
}

Ptr<Chunk> EthernetHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ethernetHeader = std::make_shared<EthernetHeader>();
    auto position = stream.getPosition();
    ethernetHeader->setProtocol((Protocol)stream.readUint16Be());
    stream.readByteRepeatedly(0, byte(ethernetHeader->getChunkLength() - stream.getPosition() + position).get());
    return ethernetHeader;
}

void EthernetTrailerSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& ethernetTrailer = std::static_pointer_cast<const EthernetTrailer>(chunk);
    auto position = stream.getLength();
    stream.writeUint16Be(ethernetTrailer->getCrc());
    stream.writeByteRepeatedly(0, byte(ethernetTrailer->getChunkLength() - stream.getLength() + position).get());
}

Ptr<Chunk> EthernetTrailerSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ethernetTrailer = std::make_shared<EthernetTrailer>();
    auto position = stream.getPosition();
    ethernetTrailer->setCrc(stream.readUint16Be());
    stream.readByteRepeatedly(0, byte(ethernetTrailer->getChunkLength() - stream.getPosition() + position).get());
    return ethernetTrailer;
}

std::string ApplicationHeader::str() const
{
    std::ostringstream os;
    os << "ApplicationHeader, length = " << byte(getChunkLength()) << ", someData = " << someData;
    return os.str();
}

std::string TcpHeader::str() const
{
    std::ostringstream os;
    os << "TcpHeader, length = " << byte(getChunkLength()) << ", lengthField = " << getLengthField() << ", srcPort = " << srcPort << ", destPort = " << destPort << ", crc = " << crc;
    return os.str();
}

std::string IpHeader::str() const
{
    std::ostringstream os;
    os << "IpHeader, length = " << byte(getChunkLength()) << ", protocol = " << protocol;
    return os.str();
}

std::string EthernetHeader::str() const
{
    std::ostringstream os;
    os << "EthernetHeader, length = " << byte(getChunkLength()) << ", protocol = " << protocol;
    return os.str();
}

std::string EthernetTrailer::str() const
{
    std::ostringstream os;
    os << "EthernetTrailer, length = " << byte(getChunkLength()) << ", crc = " << crc;
    return os.str();
}

void NewMedium::sendPacket(Packet *packet)
{
    EV_DEBUG << "Sending packet: " << packet << std::endl;
    packets.push_back(serialize ? serializePacket(packet) : packet);
}

const std::vector<Packet *> NewMedium::receivePackets()
{
    std::vector<Packet *> receivedPackets;
    for (auto packet : packets)
        receivedPackets.push_back(packet->dup());
    return receivedPackets;
}

Packet *NewMedium::serializePacket(Packet *packet)
{
    const auto& bytesChunk = packet->peekAllBytes();
    auto serializedPacket = new Packet();
    serializedPacket->append(bytesChunk);
    return serializedPacket;
}

void NewSender::sendEthernet(Packet *packet)
{
    auto ethernetFrame = packet;
    auto ethernetHeader = std::make_shared<EthernetHeader>();
    ethernetHeader->setProtocol(Protocol::Ip);
    ethernetHeader->markImmutable();
    ethernetFrame->prepend(ethernetHeader);
    medium.sendPacket(ethernetFrame);
}

void NewSender::sendIp(Packet *packet)
{
    auto ipDatagram = packet;
    auto ipHeader = std::make_shared<IpHeader>();
    ipHeader->setProtocol(Protocol::Tcp);
    ipHeader->markImmutable();
    ipDatagram->prepend(ipHeader);
    sendEthernet(ipDatagram);
}

Ptr<TcpHeader> NewSender::createTcpHeader()
{
    auto tcpHeader = std::make_shared<TcpHeader>();
    tcpHeader->setChunkLength(byte(20));
    tcpHeader->setLengthField(20);
    tcpHeader->setSrcPort(1000);
    tcpHeader->setDestPort(2000);
    return tcpHeader;
}

void NewSender::sendTcp(Packet *packet)
{
    if (tcpSegment == nullptr)
        tcpSegment = new Packet();
    byte tcpSegmentSizeLimit = byte(15);
    if (tcpSegment->getTotalLength() + packet->getTotalLength() >= tcpSegmentSizeLimit) {
        bit length = tcpSegmentSizeLimit - tcpSegment->getTotalLength();
        tcpSegment->append(packet->peekAt(bit(0), length));
        auto tcpHeader = createTcpHeader();
        auto crcMode = medium.getSerialize() ? CRC_COMPUTED : CRC_DECLARED_CORRECT;
        tcpHeader->setCrcMode(crcMode);
        switch (crcMode) {
            case CRC_DECLARED_INCORRECT:
            case CRC_DECLARED_CORRECT:
                break;
            case CRC_COMPUTED: {
                tcpHeader->setCrc(0);
                tcpHeader->setCrc(computeTcpCrc(BytesChunk(), tcpHeader, tcpSegment->peekAt<BytesChunk>(bit(0), tcpSegment->getTotalLength())));
                break;
            }
            default:
                throw cRuntimeError("Unknown bit error value");
        }
        tcpHeader->markImmutable();
        tcpSegment->prepend(tcpHeader);
        sendIp(tcpSegment);
        bit remainingLength = packet->getTotalLength() - length;
        if (remainingLength == bit(0))
            tcpSegment = nullptr;
        else {
            tcpSegment = new Packet();
            tcpSegment->append(packet->peekAt(length, remainingLength));
        }
    }
    else
        tcpSegment->append(packet->peekAt(bit(0)));
    delete packet;
}

void NewSender::sendPackets()
{
    auto bytesChunk = std::make_shared<BytesChunk>();
    bytesChunk->setBytes({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    bytesChunk->markImmutable();
    EV_DEBUG << "Sending application data: " << bytesChunk << std::endl;
    auto packet1 = new Packet();
    packet1->append(bytesChunk);
    sendTcp(packet1);

    auto applicationHeader = std::make_shared<ApplicationHeader>();
    applicationHeader->setSomeData(42);
    applicationHeader->markImmutable();
    EV_DEBUG << "Sending application data: " << applicationHeader << std::endl;
    auto packet2 = new Packet();
    packet2->append(applicationHeader);
    sendTcp(packet2);

    auto byteSizeChunk = std::make_shared<ByteCountChunk>();
    byteSizeChunk->setLength(byte(10));
    byteSizeChunk->markImmutable();
    EV_DEBUG << "Sending application data: " << byteSizeChunk << std::endl;
    auto packet3 = new Packet();
    packet3->append(byteSizeChunk);
    sendTcp(packet3);
}

void NewReceiver::receiveApplication(Packet *packet)
{
    const auto& chunk = packet->peekDataAt(bit(0));
    EV_DEBUG << "Collecting application data: " << chunk << std::endl;
    applicationData.push(chunk);
    EV_DEBUG << "Buffering application data: " << applicationData << std::endl;
    if (applicationData.getPoppedLength() == byte(0) && applicationData.has<BytesChunk>(byte(10))) {
        const auto& chunk = applicationData.pop<BytesChunk>(byte(10));
        EV_DEBUG << "Receiving application data: " << chunk << std::endl;
    }
    if (applicationData.getPoppedLength() == byte(10) && applicationData.has<ApplicationHeader>()) {
        const auto& chunk = applicationData.pop<ApplicationHeader>();
        EV_DEBUG << "Receiving application data: " << chunk << std::endl;
    }
    if (applicationData.getPoppedLength() == byte(20) && applicationData.has<ByteCountChunk>(byte(10))) {
        const auto& chunk = applicationData.pop<ByteCountChunk>(byte(10));
        EV_DEBUG << "Receiving application data: " << chunk << std::endl;
    }
    delete packet;
}

void NewReceiver::receiveTcp(Packet *packet)
{
    bit tcpHeaderPopOffset = packet->getHeaderPopOffset();
    auto tcpSegment = packet;
    const auto& tcpHeader = tcpSegment->popHeader<TcpHeader>();
    auto srcPort = tcpHeader->getSrcPort();
    auto destPort = tcpHeader->getDestPort();
    auto crc = tcpHeader->getCrc();
    if (srcPort != 1000 || destPort != 2000)
        throw cRuntimeError("Invalid TCP port");
    switch (tcpHeader->getCrcMode()) {
        case CRC_DECLARED_INCORRECT:
            delete packet;
            return;
        case CRC_DECLARED_CORRECT:
            break;
        case CRC_COMPUTED: {
            bit length = packet->getTotalLength() - tcpHeaderPopOffset - packet->getTrailerPopOffset();
            if (crc != computeTcpCrc(BytesChunk(), tcpHeader, packet->peekAt<BytesChunk>(tcpHeaderPopOffset, length))) {
                delete packet;
                return;
            }
            break;
        }
        default:
            throw cRuntimeError("Unknown bit error value");
    }
    receiveApplication(tcpSegment);
}

void NewReceiver::receiveIp(Packet *packet)
{
    auto ipDatagram = packet;
    const auto& ipHeader = ipDatagram->popHeader<IpHeader>();
    if (ipHeader->getProtocol() != Protocol::Tcp)
        throw cRuntimeError("Invalid IP protocol");
    receiveTcp(ipDatagram);
}

void NewReceiver::receiveEthernet(Packet *packet)
{
    auto ethernetFrame = packet;
    const auto& ethernetHeader = ethernetFrame->popHeader<EthernetHeader>();
    if (ethernetHeader->getProtocol() != Protocol::Ip)
        throw cRuntimeError("Invalid Ethernet protocol");
    receiveIp(ethernetFrame);
}

void NewReceiver::receivePackets()
{
    for (auto packet : medium.receivePackets()) {
        EV_DEBUG << "Receiving packet: " << packet << std::endl;
        receiveEthernet(packet);
    }
}

void NewTest::initialize()
{
    int repetitionCount = par("repetitionCount");
    int receiverCount = par("receiverCount");
    bool serialize = par("serialize");
    clock_t begin = clock();
    for (int i = 0; i < repetitionCount; i++) {
        NewMedium medium(serialize);
        NewSender sender(medium);
        sender.sendPackets();
        for (int j = 0; j < receiverCount; j++) {
            NewReceiver receiver(medium);
            receiver.receivePackets();
        }
    }
    clock_t end = clock();
    runtime = (double)(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Runtime: " << runtime << std::endl;
}

void NewTest::finish()
{
    recordScalar("runtime", runtime);
}

} // namespace
