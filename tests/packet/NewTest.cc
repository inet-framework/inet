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

#include "NewTest.h"

namespace inet {

Register_Serializer(ApplicationHeaderSerializer);
Register_Serializer(TcpHeaderSerializer);
Register_Serializer(IpHeaderSerializer);
Register_Serializer(EthernetHeaderSerializer);
Define_Module(NewTest);

static int16_t computeTcpCrc(const ByteArrayChunk& pseudoHeader, const std::shared_ptr<ByteArrayChunk>& tcpSegment)
{
    return 42;
}

void ApplicationHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& applicationHeader = std::static_pointer_cast<const ApplicationHeader>(chunk);
    int64_t position = stream.getPosition();
    stream.writeUint16(applicationHeader->getSomeData());
    stream.writeByteRepeatedly(0, applicationHeader->getByteLength() - stream.getPosition() + position);
}

std::shared_ptr<Chunk> ApplicationHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto applicationHeader = std::make_shared<ApplicationHeader>();
    int64_t position = stream.getPosition();
    applicationHeader->setSomeData(stream.readUint16());
    stream.readByteRepeatedly(0, applicationHeader->getByteLength() - stream.getPosition() + position);
    return applicationHeader;
}

void TcpHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& tcpHeader = std::static_pointer_cast<const TcpHeader>(chunk);
    int64_t position = stream.getPosition();
    if (tcpHeader->getBitError() != BIT_ERROR_CRC)
        throw cRuntimeError("Cannot serialize TCP header");
    stream.writeUint16(tcpHeader->getLengthField());
    stream.writeUint16(tcpHeader->getSrcPort());
    stream.writeUint16(tcpHeader->getDestPort());
    stream.writeUint16(tcpHeader->getCrc());
    stream.writeByteRepeatedly(0, tcpHeader->getByteLength() - stream.getPosition() + position);
}

std::shared_ptr<Chunk> TcpHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto tcpHeader = std::make_shared<TcpHeader>();
    int64_t position = stream.getPosition();
    int64_t remainingSize = stream.getRemainingSize();
    int16_t lengthField = stream.readUint16();
    if (lengthField > remainingSize)
        tcpHeader->makeIncomplete();
    int16_t byteLength = std::min(lengthField, (int16_t)remainingSize);
    tcpHeader->setByteLength(byteLength);
    tcpHeader->setLengthField(lengthField);
    tcpHeader->setSrcPort(stream.readUint16());
    tcpHeader->setDestPort(stream.readUint16());
    tcpHeader->setBitError(BIT_ERROR_CRC);
    tcpHeader->setCrc(stream.readUint16());
    stream.readByteRepeatedly(0, byteLength - stream.getPosition() + position);
    return tcpHeader;
}

void IpHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& ipHeader = std::static_pointer_cast<const IpHeader>(chunk);
    int64_t position = stream.getPosition();
    stream.writeUint16((int16_t)ipHeader->getProtocol());
    stream.writeByteRepeatedly(0, ipHeader->getByteLength() - stream.getSize() + position);
}

std::shared_ptr<Chunk> IpHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto ipHeader = std::make_shared<IpHeader>();
    int64_t position = stream.getPosition();
    ipHeader->setProtocol((Protocol)stream.readUint16());
    stream.readByteRepeatedly(0, ipHeader->getByteLength() - stream.getPosition() + position);
    return ipHeader;
}

void EthernetHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& ethernetHeader = std::static_pointer_cast<const EthernetHeader>(chunk);
    int64_t position = stream.getPosition();
    stream.writeUint16((int16_t)ethernetHeader->getProtocol());
    stream.writeByteRepeatedly(0, ethernetHeader->getByteLength() - stream.getPosition() + position);
}

std::shared_ptr<Chunk> EthernetHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto ethernetHeader = std::make_shared<EthernetHeader>();
    int64_t position = stream.getPosition();
    ethernetHeader->setProtocol((Protocol)stream.readUint16());
    stream.readByteRepeatedly(0, ethernetHeader->getByteLength() - stream.getPosition() + position);
    return ethernetHeader;
}

std::string ApplicationHeader::str() const
{
    std::ostringstream os;
    os << "ApplicationHeader, byteLength = " << getByteLength() << ", someData = " << someData;
    return os.str();
}

std::string TcpHeader::str() const
{
    std::ostringstream os;
    os << "TcpHeader, byteLength = " << getByteLength() << ", lengthField = " << getLengthField() << ", srcPort = " << srcPort << ", destPort = " << destPort << ", crc = " << crc;
    return os.str();
}

std::string IpHeader::str() const
{
    std::ostringstream os;
    os << "IpHeader, byteLength = " << getByteLength() << ", protocol = " << protocol;
    return os.str();
}

std::string EthernetHeader::str() const
{
    std::ostringstream os;
    os << "EthernetHeader, byteLength = " << getByteLength() << ", protocol = " << protocol;
    return os.str();
}

void NewMedium::sendPacket(Packet *packet)
{
    EV_DEBUG << "Sending packet: " << packet << std::endl;
    packet->makeImmutable();
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
    const auto& byteArrayChunk = packet->peekDataAt<ByteArrayChunk>(0, packet->getByteLength());
    auto serializedPacket = new Packet();
    serializedPacket->append(byteArrayChunk);
    serializedPacket->makeImmutable();
    return serializedPacket;
}

void NewSender::sendEthernet(Packet *packet)
{
    auto ethernetFrame = packet;
    auto ethernetHeader = std::make_shared<EthernetHeader>();
    ethernetHeader->setProtocol(Protocol::Ip);
    ethernetHeader->makeImmutable();
    ethernetFrame->prepend(ethernetHeader);
    medium.sendPacket(ethernetFrame);
}

void NewSender::sendIp(Packet *packet)
{
    auto ipDatagram = packet;
    auto ipHeader = std::make_shared<IpHeader>();
    ipHeader->setProtocol(Protocol::Tcp);
    ipHeader->makeImmutable();
    ipDatagram->prepend(ipHeader);
    sendEthernet(ipDatagram);
}

Packet *NewSender::createTcpSegment()
{
    auto tcpSegment = new Packet();
    auto tcpHeader = std::make_shared<TcpHeader>();
    tcpHeader->setByteLength(20);
    tcpHeader->setLengthField(20);
    tcpHeader->setSrcPort(1000);
    tcpHeader->setDestPort(2000);
    tcpHeader->makeImmutable();
    tcpSegment->prepend(tcpHeader);
    return tcpSegment;
}

void NewSender::sendTcp(Packet *packet)
{
    packet->makeImmutable();
    if (tcpSegment == nullptr)
        tcpSegment = createTcpSegment();
    int64_t tcpSegmentSizeLimit = 35;
    if (tcpSegment->getByteLength() + packet->getByteLength() >= tcpSegmentSizeLimit) {
        int64_t byteLength = tcpSegmentSizeLimit - tcpSegment->getByteLength();
        tcpSegment->append(packet->peekDataAt(0, byteLength));
        const auto& tcpHeader = tcpSegment->peekHeader<TcpHeader>();
        auto bitError = medium.getSerialize() ? BIT_ERROR_CRC : BIT_ERROR_NO;
        tcpHeader->setBitError(bitError);
        switch (bitError) {
            case BIT_ERROR_YES:
            case BIT_ERROR_NO:
                break;
            case BIT_ERROR_CRC: {
                tcpHeader->setCrc(0);
                tcpHeader->setCrc(computeTcpCrc(ByteArrayChunk(), tcpSegment->peekDataAt<ByteArrayChunk>(0, tcpSegment->getByteLength())));
                break;
            }
            default:
                throw cRuntimeError("Unknown bit error value");
        }
        sendIp(tcpSegment);
        int64_t remainingByteLength = packet->getByteLength() - byteLength;
        if (remainingByteLength == 0)
            tcpSegment = nullptr;
        else {
            tcpSegment = createTcpSegment();
            tcpSegment->append(packet->peekDataAt(byteLength, remainingByteLength));
        }
    }
    else
        tcpSegment->append(packet);
    delete packet;
}

void NewSender::sendPackets()
{
    auto byteArrayChunk = std::make_shared<ByteArrayChunk>();
    byteArrayChunk->setBytes({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    byteArrayChunk->makeImmutable();
    EV_DEBUG << "Sending application data: " << byteArrayChunk << std::endl;
    auto packet1 = new Packet();
    packet1->append(byteArrayChunk);
    sendTcp(packet1);

    auto applicationHeader = std::make_shared<ApplicationHeader>();
    applicationHeader->setSomeData(42);
    applicationHeader->makeImmutable();
    EV_DEBUG << "Sending application data: " << applicationHeader << std::endl;
    auto packet2 = new Packet();
    packet2->append(applicationHeader);
    sendTcp(packet2);

    auto byteSizeChunk = std::make_shared<ByteLengthChunk>();
    byteSizeChunk->setByteLength(10);
    byteSizeChunk->makeImmutable();
    EV_DEBUG << "Sending application data: " << byteSizeChunk << std::endl;
    auto packet3 = new Packet();
    packet3->append(byteSizeChunk);
    sendTcp(packet3);
}

void NewReceiver::receiveApplication(Packet *packet)
{
    const auto& chunk = packet->peekData();
    EV_DEBUG << "Collecting application data: " << chunk << std::endl;
    applicationData.push(chunk);
    EV_DEBUG << "Buffered application data: " << applicationData << std::endl;
    if (applicationData.getPoppedByteLength() == 0 && applicationData.has<ByteArrayChunk>(10))
        EV_DEBUG << "Receiving application data: " << applicationData.pop<ByteArrayChunk>(10) << std::endl;
    if (applicationData.getPoppedByteLength() == 10 && applicationData.has<ApplicationHeader>())
        EV_DEBUG << "Receiving application data: " << applicationData.pop<ApplicationHeader>() << std::endl;
    if (applicationData.getPoppedByteLength() == 20 && applicationData.has<ByteLengthChunk>(10))
        EV_DEBUG << "Receiving application data: " << applicationData.pop<ByteLengthChunk>(10) << std::endl;
    delete packet;
}

void NewReceiver::receiveTcp(Packet *packet)
{
    int tcpHeaderPosition = packet->getHeaderPosition();
    auto tcpSegment = packet;
    const auto& tcpHeader = tcpSegment->popHeader<TcpHeader>();
    auto srcPort = tcpHeader->getSrcPort();
    auto destPort = tcpHeader->getDestPort();
    auto crc = tcpHeader->getCrc();
    if (srcPort != 1000 || destPort != 2000)
        throw cRuntimeError("Invalid TCP port");
    switch (tcpHeader->getBitError()) {
        case BIT_ERROR_YES:
            delete packet;
            return;
        case BIT_ERROR_NO:
            break;
        case BIT_ERROR_CRC: {
            int64_t byteLength = packet->getByteLength() - tcpHeaderPosition - packet->getTrailerPosition();
            if (crc != computeTcpCrc(ByteArrayChunk(), packet->peekHeaderAt<ByteArrayChunk>(tcpHeaderPosition, byteLength))) {
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
}

void NewTest::finish()
{
    recordScalar("runtime", runtime);
}

} // namespace
