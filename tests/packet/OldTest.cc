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

#include "OldTest.h"

namespace inet {

Define_Module(OldTest);

void TcpSegment::copy(const TcpSegment& other)
{
    for (int i = 0; i < payload_arraysize; i++) {
        auto p = payload[i];
        auto fragment = new Fragment();
        fragment->setByteLength(p->getByteLength());
        if (p->getData())
            fragment->setData(p->getData()->dup());
        payload[i] = fragment;
    }
}

TcpSegment::~TcpSegment()
{
    for (int i = 0; i < payload_arraysize; i++) {
        auto p = payload[i];
        delete p->getData();
        delete p;
    }
}

void OldMedium::sendPacket(cPacket *packet)
{
    EV_DEBUG << "Sending packet: " << packet << std::endl;
    packets.push_back(serialize ? serializePacket(packet) : packet);
}

const std::vector<cPacket *> OldMedium::receivePackets()
{
    std::vector<cPacket *> receivedPackets;
    for (auto packet : packets)
        receivedPackets.push_back(serialize ? deserializePacket(packet) : packet->dup());
    return receivedPackets;
}

cPacket *OldMedium::serializePacket(cPacket *packet)
{
    // TODO: serialize
    return packet;
}

cPacket *OldMedium::deserializePacket(cPacket *packet)
{
    // TODO: deserialize
    return packet;
}

void OldSender::sendEthernet(cPacket *packet)
{
    auto ethernetFrame = new EthernetFrame();
    ethernetFrame->setProtocol(Protocol::Ip);
    ethernetFrame->encapsulate(packet);
    medium.sendPacket(ethernetFrame);
}

void OldSender::sendIp(cPacket *packet)
{
    auto ipDatagram = new IpDatagram();
    ipDatagram->setProtocol(Protocol::Tcp);
    ipDatagram->encapsulate(packet);
    sendEthernet(ipDatagram);
}

void OldSender::sendTcp(cPacket *packet)
{
    if (tcpSegment == nullptr)
        tcpSegment = createTcpSegment();
    int tcpSegmentSizeLimit = 35;
    if (tcpSegment->getByteLength() + packet->getByteLength() >= tcpSegmentSizeLimit) {
        int byteLength = tcpSegmentSizeLimit - tcpSegment->getByteLength();
        int size = tcpSegment->getPayloadArraySize();
        tcpSegment->setPayloadArraySize(size + 1);
        auto fragment1 = new Fragment();
        fragment1->setData(packet);
        fragment1->setByteLength(byteLength);
        tcpSegment->setPayload(size, fragment1);
        tcpSegment->setByteLength(tcpSegment->getByteLength() + byteLength);
        sendIp(tcpSegment);
        int remainingByteLength = packet->getByteLength() - byteLength;
        if (remainingByteLength == 0)
            tcpSegment = nullptr;
        else {
            tcpSegment = createTcpSegment();
            tcpSegment->setPayloadArraySize(1);
            auto fragment2 = new Fragment();
            fragment2->setData(packet->dup());
            fragment2->setByteLength(remainingByteLength);
            tcpSegment->setPayload(0, fragment2);
            tcpSegment->setByteLength(tcpSegment->getByteLength() + remainingByteLength);
        }
    }
    else {
        int size = tcpSegment->getPayloadArraySize();
        tcpSegment->setPayloadArraySize(size + 1);
        auto fragment = new Fragment();
        fragment->setData(packet);
        fragment->setByteLength(packet->getByteLength());
        tcpSegment->setPayload(size, fragment);
        tcpSegment->setByteLength(tcpSegment->getByteLength() + packet->getByteLength());
    }
}

TcpSegment *OldSender::createTcpSegment()
{
    auto tcpSegment = new TcpSegment();
    tcpSegment->setByteLength(20);
    tcpSegment->setSrcPort(1000);
    tcpSegment->setDestPort(2000);
    return tcpSegment;
}

void OldSender::sendPackets()
{
    auto byteArrayData = new ByteArrayData();
    byteArrayData->setBytesArraySize(10);
    for (int i = 0; i < 10; i++)
        byteArrayData->setBytes(i, i);
    byteArrayData->setByteLength(10);
    EV_DEBUG << "Sending application data: " << byteArrayData << std::endl;
    sendTcp(byteArrayData);

    auto applicationData = new ApplicationData();
    applicationData->setSomeData(42);
    EV_DEBUG << "Sending application data: " << applicationData << std::endl;
    sendTcp(applicationData);

    auto byteSizeData = new ByteLengthData();
    byteSizeData->setByteLength(10);
    EV_DEBUG << "Sending application data: " << byteSizeData << std::endl;
    sendTcp(byteSizeData);
}

cPacket *OldReceiver::getApplicationData(int byteLength)
{
    if (byteLength == -1)
        byteLength = applicationData[applicationDataIndex]->getData()->getByteLength();
    for (int i = applicationDataIndex; i < applicationData.size(); i++) {
        auto fragment = applicationData[i];
        byteLength -= fragment->getByteLength();
        if (byteLength == 0) {
            auto packet = fragment->getData();
            applicationDataIndex = i + 1;
            applicationDataPosition += packet->getByteLength();
            return packet;
        }
        else
            delete fragment->getData();
    }
    throw cRuntimeError("Application data not found");
}

void OldReceiver::receiveApplication(std::vector<Fragment *>& fragments)
{
    for (auto fragment : fragments) {
        EV_DEBUG << "Collecting application data: " << fragment << std::endl;
        applicationData.push_back(fragment);
        applicationDataByteLength += fragment->getByteLength();
    }
    if (applicationDataPosition == 0 && applicationDataByteLength >= 10) {
        auto byteArrayData = getApplicationData(10);
        EV_DEBUG << "Receiving application data: " << byteArrayData << std::endl;
        delete byteArrayData;
    }
    if (applicationDataPosition == 10 && applicationDataByteLength >= 20) {
        auto applicationData = getApplicationData(-1);
        EV_DEBUG << "Receiving application data: " << applicationData << std::endl;
        delete applicationData;
    }
    if (applicationDataPosition == 20 && applicationDataByteLength >= 30) {
        auto byteLengthData = getApplicationData(10);
        EV_DEBUG << "Receiving application data: " << byteLengthData << std::endl;
        delete byteLengthData;
    }
}

void OldReceiver::receiveTcp(cPacket *packet)
{
    auto tcpSegment = check_and_cast<TcpSegment *>(packet);
    auto srcPort = tcpSegment->getSrcPort();
    auto destPort = tcpSegment->getDestPort();
    if (srcPort != 1000 || destPort != 2000)
        throw std::runtime_error("Invalid TCP port");
    int size = tcpSegment->getPayloadArraySize();
    std::vector<Fragment *> fragments;
    for (int i = 0; i < size; i ++)
        fragments.push_back(tcpSegment->getPayload(i));
    tcpSegment->setPayloadArraySize(0);
    receiveApplication(fragments);
    delete packet;
}

void OldReceiver::receiveIp(cPacket *packet)
{
    auto ipDatagram = check_and_cast<IpDatagram *>(packet);
    if (ipDatagram->getProtocol() != Protocol::Tcp)
        throw std::runtime_error("Invalid IP protocol");
    receiveTcp(ipDatagram->decapsulate());
    delete packet;
}

void OldReceiver::receiveEthernet(cPacket *packet)
{
    auto ethernetFrame = check_and_cast<EthernetFrame *>(packet);
    if (ethernetFrame->getProtocol() != Protocol::Ip)
        throw std::runtime_error("Invalid Ethernet protocol");
    receiveIp(ethernetFrame->decapsulate());
    delete packet;
}

void OldReceiver::receivePackets()
{
    for (auto packet : medium.receivePackets()) {
        EV_DEBUG << "Receiving packet: " << packet << std::endl;
        receiveEthernet(packet);
    }
}

void OldTest::initialize()
{
    int repetitionCount = par("repetitionCount");
    int receiverCount = par("receiverCount");
    bool serialize = par("serialize");
    clock_t begin = clock();
    for (int i = 0; i < repetitionCount; i++) {
        OldMedium medium(serialize);
        OldSender sender(medium);
        sender.sendPackets();
        for (int j = 0; j < receiverCount; j++) {
            OldReceiver receiver(medium);
            receiver.receivePackets();
        }
    }
    clock_t end = clock();
    runtime = (double)(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Runtime: " << runtime << std::endl;
}

void OldTest::finish()
{
    recordScalar("runtime", runtime);
}

} // namespace
