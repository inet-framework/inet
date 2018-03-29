//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

/*
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/ReassemblyBuffer.h"
#include "inet/common/packet/ReorderBuffer.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

#define FCS_MODE_DECLARED 1

namespace inet {

using namespace std;
using namespace inet::physicallayer;
using namespace inet::ieee80211;

class Mac : public cSimpleModule
{
  public:
    MacAddress selfAddress;

  public:
    void sendUp(Packet *packet);
    void encapsulate(Packet *packet);
    void decapsulate(Packet *packet);
    vector<Packet *> *fragment(Packet *packet, vector<b>& sizes);
    Packet *defragment(vector<Packet *>& fragments);
    Packet *aggregate(vector<Packet *>& packets);
    vector<Packet *> *disaggregate(Packet *aggregate);
    void initialize(int stage);
    void registerInterface(MacAddress address);
    bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *callback);
    bool handleNodeStart(int stage, IDoneCallback *callback);
    bool handleNodeShutdown(int stage, IDoneCallback *callback);
};

class MacHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    void serialize(MemoryOutputStream& stream, Ptr<Chunk>& chunk);
    Ptr<Chunk> deserialize(MemoryInputStream& stream);
};

class MacHeader : public FieldsChunk
{
  public:
    void setAggregate(bool f);
    void setFragmentOffset(b o);
    int getType() const;
    void setType(int t);
    MacAddress getTransmitterAddress() const;
    void setTransmitterAddress(MacAddress m);
    MacAddress getReceiverAddress() const;
    void setReceiverAddress(MacAddress m);
    void setLengthField(b l);
    b getLengthField() const;
};

class MacTrailer : public FieldsChunk
{
  public:
    void setFcsMode(int x);
    int getFcsMode() const;
};

class SubHeader : public FieldsChunk
{
  public:
    void setLengthField(b l);
    b getLengthField() const;
};

//!PacketEncapsulationExample
void Mac::encapsulate(Packet *packet)
{
    auto header = makeShared<MacHeader>(); // create new header
    header->setChunkLength(B(8)); // set chunk length to 8 bytes
    header->setLengthField(packet->getDataLength()); // set length field
    header->setTransmitterAddress(selfAddress); // set other header fields
    packet->insertFront(header); // insert header into packet
    auto trailer = makeShared<MacTrailer>(); // create new trailer
    trailer->setChunkLength(B(4)); // set chunk length to 4 bytes
    trailer->setFcsMode(FCS_MODE_DECLARED); // set trailer fields
    packet->insertBack(trailer); // insert trailer into packet
}
//!End

//!PacketDecapsulationExample
void Mac::decapsulate(Packet *packet)
{
    auto header = packet->popFront<MacHeader>(); // pop header from packet
    auto lengthField = header->getLengthField();
    cout << header->getChunkLength() << endl; // print chunk length
    cout << lengthField << endl; // print header length field
    cout << header->getReceiverAddress() << endl; // print other header fields
    auto trailer = packet->popBack<MacTrailer>(); // pop trailer from packet
    cout << trailer->getFcsMode() << endl; // print trailer fields
    assert(packet->getDataLength() == lengthField); // if the packet is correct
}
//!End

//!PacketFragmentationExample
vector<Packet *> *Mac::fragment(Packet *packet, vector<b>& sizes)
{
    auto offset = b(0); // start from the packet's beginning
    auto fragments = new vector<Packet *>(); // result collection
    for (auto size : sizes) { // for each received size do
        auto fragment = new Packet("Fragment"); // header + data part + trailer
        auto header = makeShared<MacHeader>(); // create new header
        header->setFragmentOffset(offset); // set fragment offset for reassembly
        fragment->insertFront(header); // insert header into fragment
        auto data = packet->peekAt(offset, size); // get data part from packet
        fragment->insertBack(data); // insert data part into fragment
        auto trailer = makeShared<MacTrailer>(); // create new trailer
        fragment->insertBack(trailer); // insert trailer into fragment
        fragments->push_back(fragment); // collect fragment into result
        offset += size; // increment offset with size of data part
    }
    return fragments;
}
//!End

//!PacketDefragmentationExample
Packet *Mac::defragment(vector<Packet *>& fragments)
{
    auto packet = new Packet("Original"); // create new concatenated packet
    for (auto fragment : fragments) {
        fragment->popFront<MacHeader>(); // pop header from fragment
        fragment->popBack<MacTrailer>(); // pop trailer from fragment
        packet->insertBack(fragment->peekData()); // concatenate fragment data
    }
    return packet;
}
//!End

//!PacketAggregationExample
Packet *Mac::aggregate(vector<Packet *>& packets)
{
    auto aggregate = new Packet("Aggregate"); // create concatenated packet
    for (auto packet : packets) { // for each received packet do
        auto header = makeShared<SubHeader>(); // create new subheader
        header->setLengthField(packet->getDataLength()); // set subframe length
        aggregate->insertBack(header); // insert subheader into aggregate
        auto data = packet->peekData(); // get packet data
        aggregate->insertBack(data); // insert data into aggregate
    }
    auto header = makeShared<MacHeader>(); // create new header
    header->setAggregate(true); // set aggregate flag
    aggregate->insertFront(header); // insert header into aggregate
    auto trailer = makeShared<MacTrailer>(); // create new trailer
    aggregate->insertBack(trailer); // insert trailer into aggregate
    return aggregate;
}
//!End

//!PacketDisaggregationExample
vector<Packet *> *Mac::disaggregate(Packet *aggregate)
{
    aggregate->popFront<MacHeader>(); // pop header from packet
    aggregate->popBack<MacTrailer>(); // pop trailer from packet
    vector<Packet *> *packets = new vector<Packet *>(); // result collection
    b offset = aggregate->getHeaderPopOffset(); // start after header
    while (offset != aggregate->getTrailerPopOffset()) { // up to trailer
        auto header = aggregate->peekAt<SubHeader>(offset); // peek sub header
        offset += header->getChunkLength(); // increment with header length
        auto size = header->getLengthField(); // get length field from header
        auto data = aggregate->peekAt(offset, size); // peek following data part
        auto packet = new Packet("Original"); // create new packet
        packet->insertBack(data); // insert data into packet
        packets->push_back(packet); // collect packet into result
        offset += size; // increment offset with data size
    }
    return packets;
}
//!End

//!PacketSerializationExample
void MacHeaderSerializer::serialize
    (MemoryOutputStream& stream, Ptr<Chunk>& chunk)
{
    auto header = staticPtrCast<MacHeader>(chunk);
    stream.writeUint16Be(header->getType()); // unsigned 16 bits, big endian
    stream.writeMacAddress(header->getTransmitterAddress());
    stream.writeMacAddress(header->getReceiverAddress());
}
//!End

//!PacketDeserializationExample
Ptr<Chunk> MacHeaderSerializer::deserialize(MemoryInputStream& stream)
{
    auto header = makeShared<MacHeader>(); // create new header
    header->setType(stream.readUint16Be()); // unsigned 16 bits, big endian
    header->setTransmitterAddress(stream.readMacAddress());
    header->setReceiverAddress(stream.readMacAddress());
    return header;
}
//!End

class ExternalInterface
{
  public:
    const vector<uint8_t>& prepareToSend(Packet *packet);
    Packet *prepareToReceive(vector<uint8_t>& bytes);
};

const
//!EmulationPacketSendingExample
vector<uint8_t>& ExternalInterface::prepareToSend(Packet *packet)
{
    auto data = packet->peekAllBytes(); // convert to a sequence of bytes
    return data->getBytes(); // actual bytes to send
}
//!End

//!EmulationPacketReceivingExample
Packet *ExternalInterface::prepareToReceive(vector<uint8_t>& bytes)
{
    auto data = makeShared<BytesChunk>(bytes); // create chunk with bytes
    return new Packet("Emulation", data); // create packet with data
}
//!End

class TcpHeader : public FieldsChunk
{
  public:
    B getSequenceNumber() const;
    void setSequenceNumber(B sequenceNumber);
};

//!PacketQueueingExample
class TcpSendQueue
{
  public:
    ChunkQueue queue; // stores application data
    B sequenceNumber; // position in stream

  public:
    void enqueueApplicationData(Packet *packet);
    Packet *createSegment(b length);
};

void TcpSendQueue::enqueueApplicationData(Packet *packet)
{
    queue.push(packet->peekData()); // store received data
}

Packet *TcpSendQueue::createSegment(b maxLength)
{
    auto packet = new Packet("Segment"); // create new segment
    auto header = makeShared<TcpHeader>(); // create new header
    header->setSequenceNumber(sequenceNumber); // store sequence number for reordering
    packet->insertFront(header); // insert header into segment
    if (queue.getLength() < maxLength)
        maxLength = queue.getLength(); // reduce length if necessary
    auto data = queue.pop(maxLength); // pop requested amount of data
    packet->insertBack(data); // insert data into segment
    sequenceNumber += data->getChunkLength(); // increase sequence number
    return packet;
}
//!End

//!PacketReorderingExample
class TcpReceiveQueue
{
    ReorderBuffer buffer; // stores receive data
    B sequenceNumber;

  public:
    void processSegment(Packet *packet);
    Packet *getAvailableData();
};

void TcpReceiveQueue::processSegment(Packet *packet)
{
    auto header = packet->popFront<TcpHeader>(); // pop TCP header
    auto sequenceNumber = header->getSequenceNumber();
    auto data = packet->peekData(); // get all packet data
    buffer.replace(sequenceNumber, data); // overwrite data in buffer
}

Packet *TcpReceiveQueue::getAvailableData()
{
    if (buffer.getAvailableDataLength() == b(0)) // if no data available
        return nullptr;
    auto data = buffer.popAvailableData(); // remove all available data
    return new Packet("Data", data);
}
//!End

class Ipv4Header : public FieldsChunk
{
  public:
    b getFragmentOffset() const;
};

//!PacketReassemblingExample
class Ipv4Defragmentation
{
    ReassemblyBuffer buffer; // stores received data

  public:
    void processDatagram(Packet *packet);
    Packet *getReassembledDatagram();
};

void Ipv4Defragmentation::processDatagram(Packet *packet)
{
    auto header = packet->popFront<Ipv4Header>();
    auto fragmentOffset = header->getFragmentOffset(); // determine offset
    auto data = packet->peekData();
    buffer.replace(fragmentOffset, data); // overwrite data in buffer
}

Packet *Ipv4Defragmentation::getReassembledDatagram()
{
    if (!buffer.isComplete()) // if datagram isn't complete
        return nullptr; // nothing to send yet
    auto data = buffer.getReassembledData(); // complete reassembly
    return new Packet("Datagram", data); // create new packet
}
//!End

class ErrorModel
{
  public:
    bool hasProbabilisticError(b l, double per);
    Packet *corruptPacket(Packet *packet, double ber);
    Packet *corruptChunks(Packet *packet, double ber);
    Packet *corruptBytes(Packet *packet, double ber);
    Packet *corruptBits(Packet *packet, double ber);
};

//!ErrorRepresentationExample
Packet *ErrorModel::corruptPacket(Packet *packet, double ber)
{
    auto length = packet->getTotalLength();
    auto hasErrors = hasProbabilisticError(length, ber); // decide randomly
    auto corruptedPacket = packet->dup(); // cheap operation
    corruptedPacket->setBitError(hasErrors); // set bit error flag
    return corruptedPacket;
}

Packet *ErrorModel::corruptChunks(Packet *packet, double ber)
{
    b offset = b(0); // start from the beginning
    auto corruptedPacket = new Packet("Corrupt"); // create new packet
    while (auto chunk = packet->peekAt(offset)->dupShared()) { // for each chunk
        auto length = chunk->getChunkLength();
        auto hasErrors = hasProbabilisticError(length, ber); // decide randomly
        if (hasErrors) // if erroneous
           chunk->markIncorrect(); // set incorrect bit
        corruptedPacket->insertBack(chunk); // append chunk to corrupt packet
        offset += chunk->getChunkLength(); // increment offset with chunk length
    }
    return corruptedPacket;
}

Packet *ErrorModel::corruptBytes(Packet *packet, double ber)
{
    vector<uint8_t> corruptedBytes; // bytes of corrupted packet
    auto data = packet->peekAllBytes(); // data of original packet
    for (auto byte : data->getBytes()) { // for each original byte do
        if (hasProbabilisticError(B(1), ber)) // if erroneous
            byte = ~byte; // invert byte (simplified corruption)
        corruptedBytes.push_back(byte); // store byte in corrupted data
    }
    auto corruptedData = makeShared<BytesChunk>(); // create new data
    corruptedData->setBytes(corruptedBytes); // store corrupted bits
    return new Packet("Corrupt", corruptedData); // create new packet
}

Packet *ErrorModel::corruptBits(Packet *packet, double ber)
{
    vector<bool> corruptedBits; // bits of corrupted packet
    auto data = packet->peekAllBits(); // data of original packet
    for (auto bit : data->getBits()) { // for each original bit do
        if (hasProbabilisticError(b(1), ber)) // if erroneous
            bit = !bit; // flip bit
        corruptedBits.push_back(bit); // store bit in corrupted data
    }
    auto corruptedData = makeShared<BitsChunk>(); // create new data
    corruptedData->setBits(corruptedBits); // store corrupted bits
    return new Packet("Corrupt", corruptedData); // create new packet
}
//!End

class TcpClientApp
{
  private:
    TcpSocket socket;

  public:
    void send();
};

class TcpServerApp
{
  private:
    TcpSocket socket;

  public:
    void receive(Packet *packet);
    void updateStatistic(b offset, b length, simtime_t age);
};

//!RegionTaggingExample
void TcpClientApp::send()
{
    auto data = makeShared<ByteCountChunk>(); // create new data chunk
    auto creationTimeTag = data->addTag<CreationTimeTag>(); // add new tag
    creationTimeTag->setCreationTime(simTime()); // store current time
    auto packet = new Packet("Data"); // create new packet
    packet->insertBack(data);
    socket.send(packet); // send packet using TCP socket
}

void TcpServerApp::receive(Packet *packet)
{
    auto data = packet->peekData(); // get all data from the packet
    auto regions = data->getAllTags<CreationTimeTag>(); // get all tag regions
    for (auto& region : regions) { // for each region do
        auto creationTime = region.getTag()->getCreationTime(); // original time
        auto delay = simTime() - creationTime; // compute delay
        cout << region.getOffset() << region.getLength() << delay; // use data
    }
}
//!End

class Ipv4 : public cSimpleModule
{
  private:
    MacAddress selfAddress;

  public:
    void sendDown(Packet *packet, Ipv4Address nextHopAddress, int interfaceId);
    MacAddress resolveMacAddress(Ipv4Address a);
};

//!PacketTaggingExample
void Ipv4::sendDown(Packet *packet, Ipv4Address nextHopAddr, int interfaceId)
{
    auto macAddressReq = packet->addTag<MacAddressReq>(); // add new tag for MAC
    macAddressReq->setSrcAddress(selfAddress); // source is our MAC address
    auto nextHopMacAddress = resolveMacAddress(nextHopAddr); // simplified ARP
    macAddressReq->setDestAddress(nextHopMacAddress); // destination is next hop
    auto interfaceReq = packet->addTag<InterfaceReq>(); // add tag for dispatch
    interfaceReq->setInterfaceId(interfaceId); // set designated interface
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ipv4); // set protocol of packet
    send(packet, "out"); // send to MAC protocol module of designated interface
}
//!End

//!PacketDispatchingExample
void Mac::sendUp(Packet *packet)
{
    auto req = packet->addTagIfAbsent<DispatchProtocolReq>();
    req->setProtocol(&Protocol::ipv4); // set designated protocol
    req->setServicePrimitive(SP_INDICATION); // determine receiving gate
    send(packet, "upperLayerOut");
}
//!End

class UdpHeader : public FieldsChunk
{
  public:
    void setSrcPort(int p) const;
};

void chunkConstructionExample()
{
//!ChunkConstructionExample
    auto bitCountData = makeShared<BitCountChunk>(b(3), 0); // 3 zero bits
    auto byteCountData = makeShared<ByteCountChunk>(B(10), '?'); // 10 '?' bytes
    auto rawBitsData = makeShared<BitsChunk>();
    rawBitsData->setBits({1, 0, 1}); // 3 raw bits
    auto rawBytesData = makeShared<BytesChunk>(); // 10 raw bytes
    rawBytesData->setBytes({243, 74, 19, 84, 81, 134, 216, 61, 4, 8});
    auto fieldBasedHeader = makeShared<UdpHeader>(); // create new UDP header
    fieldBasedHeader->setSrcPort(1000); // set some fields
//!End
}

void chunkConcatenationExample()
{
//!ChunkConcatenationExample
    auto sequence = makeShared<SequenceChunk>(); // create empty sequence
    sequence->insertBack(makeShared<UdpHeader>()); // append UDP header
    sequence->insertBack(makeShared<ByteCountChunk>(B(10), 0)); // 10 bytes
//!End
}

void chunkSlicingExample()
{
//!ChunkSlicingExample
    auto udpHeader = makeShared<UdpHeader>(); // create 8 bytes UDP header
    auto firstHalf = udpHeader->peek(B(0), B(4)); // first 4 bytes of header
    auto secondHalf = udpHeader->peek(B(4), B(4)); // second 4 bytes of header
//!End
//}
//void merging()
//{
//!ChunkMergingExample
    auto sequence = makeShared<SequenceChunk>(); // create empty sequence
    sequence->insertBack(firstHalf); // append first half
    sequence->insertBack(secondHalf); // append second half
    auto merged = sequence->peek(B(0), B(8)); // automatically merge slices
//!End
//}
//void conversion()
//{
//!ChunkConversionExample
    auto raw = merged->peek<BytesChunk>(B(0), B(8)); // auto serialization
    auto original = raw->peek<UdpHeader>(B(0), B(8)); // auto deserialization
//!End
}

void packetConstructionExample()
{
//!PacketConstructionExample
    auto emptyPacket = new Packet("ACK"); // create empty packet
    auto data = makeShared<ByteCountChunk>(B(1000));
    auto dataPacket = new Packet("DATA", data); // create new packet with data
    auto moreData = makeShared<ByteCountChunk>(B(1000));
    dataPacket->insertBack(moreData); // insert more data at the end
    auto udpHeader = makeShared<UdpHeader>(); // create new UDP header
    dataPacket->insertFront(udpHeader); // insert header into packet
//!End
}

void packetProcessingExample()
{
    auto packet = new Packet();
//!PacketProcessingExample
    packet->popFront<MacHeader>(); // pop specific header from packet
    auto data = packet->peekData(); // peek remaining data in packet
    packet->popBack<MacTrailer>(); // pop specific trailer from packet
//!End
}

void signalConstructionExample()
{
    double duration = 0;
    auto packet = new Packet();
//!SignalConstructionExample
    auto signal = new Signal();
    signal->setDuration(duration);
    signal->encapsulate(packet);
//!End
}

class PacketDissectorCallback : public PacketDissector::ICallback
{
  public:
    void startProtocolDataUnit(const Protocol *protocol) override;
    void endProtocolDataUnit(const Protocol *protocol) override;
    void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
//!PacketDissectorCallbackInterface
    void startProtocolDataUnit(Protocol *protocol);
    void endProtocolDataUnit(Protocol *protocol);
    void markIncorrect();
    void visitChunk(Ptr<Chunk>& chunk, Protocol *protocol);
//!End
};

void packetDissectionExample()
{
    auto packet = new Packet();
    PacketDissectorCallback callback;
//!PacketDissectionExample
    auto& registry = ProtocolDissectorRegistry::globalRegistry;
    PacketDissector dissector(registry, callback);
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag->getProtocol();
    dissector.dissectPacket(packet, protocol);
//!End
}

void packetFilteringExample()
{
    auto packet = new Packet();
//!PacketFilteringExample
    PacketFilter filter;
    filter.setPattern("ping*", "Ipv4Header and sourceAddress(10.0.0.1)");
    filter.matches(packet);
//!End
}

void packetPrintingExample()
{
    auto packet = new Packet();
//!PacketPrintingExample
    PacketPrinter printer;
    printer.printPacket(std::cout, packet);
//!End
}

class App : public cSimpleModule
{
  public:
    void udpSocketUsageExample();
    void tcpSocketUsageExample();
};

void App::udpSocketUsageExample()
{
    auto packet = new Packet();
//!UDPSocketUsageExample
    UdpSocket socket;
    socket.setOutputGate(gate("out")); // configure application output gate
    socket.sendTo(packet, Ipv4Address("10.0.0.42"), 42); // send to address/port
    //!End
}

void App::tcpSocketUsageExample()
{
//!TCPSocketUsageExample
    TcpSocket socket;
    socket.setOutputGate(gate("out")); // configure application output gate
    socket.bind(Ipv4Address("10.0.0.42"), 42); // bind to address/port
    socket.listen(); // start listening for incoming connections
//!End
}

class Dcf {
  public:
    void channelGranted(IChannelAccess *channelAccess);
    void frameSequenceFinished();
    void originatorProcessTransmittedFrame(Packet *packet);

//!CoordinationFunctionInterface
    void processUpperFrame(Packet *packet, Ptr<Ieee80211DataOrMgmtHeader>& header);
    void processLowerFrame(Packet *packet, Ptr<Ieee80211MacHeader>& header);
    void corruptedFrameReceived();
//!End
};

//!DcfProcessUpperFrame
void Dcf::processUpperFrame(Packet *packet, Ptr<Ieee80211DataOrMgmtHeader>& header)
{
    if (pendingQueue->insert(packet)) { // packet queuing
        // if the insertion succeeded, request channel access
        dcfChannelAccess->requestChannel(this);
    }
    else {
        // if the queue is full, drop the packet
        delete packet;
    }
}
//!End

//!DcfChannelGranted
void Dcf::channelGranted(IChannelAccess *channelAccess)
{
    if (!frameSequenceHandler->isSequenceRunning()) {
        frameSequenceHandler->startFrameSequence(new DcfFs(), buildContext(), this);
    }
}
//!End

//!DcfFrameSequenceFinished
void Dcf::frameSequenceFinished()
{
    dcfChannelAccess->releaseChannel(this);
    if (hasFrameToTransmit())
        dcfChannelAccess->requestChannel(this);
}
//!End

//!DcfOriginatorProcessTransmittedFrame
void Dcf::originatorProcessTransmittedFrame(Packet *packet)
{
    auto transmittedHeader = packet->peekFront<Ieee80211MacHeader>();
    auto dataOrMgmtHeader = dynamicPtrCast<Ieee80211DataOrMgmtHeader>(transmittedHeader);
    if (dataOrMgmtHeader) {
        if (originatorAckPolicy->isAckNeeded(dataOrMgmtHeader)) {
            ackHandler->processTransmittedDataOrMgmtFrame(dataOrMgmtHeader);
        }
    }
}
//!End

static MacAddress MacAddress(const char *address)
{
    return MacAddress(address);
}

//!ModuleInitializationExample
void Mac::initialize(int stage) // standard OMNeT++ API
{
    if (stage == INITSTAGE_LOCAL) // operations independent from other modules
        selfAddress = MacAddress(par("address")); // store configured address
    else if (stage == INITSTAGE_NETWORK_LAYER) // after physical before network
        registerInterface(selfAddress); // register interface in InterfaceTable
}
//!End

//!LifecycleOperationExample
bool Mac::handleOperationStage
    (LifecycleOperation *operation, int stage, IDoneCallback *callback)
{
    if (dynamic_cast<NodeStartOperation *>(operation))
        handleNodeStart(stage, callback);
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
        handleNodeShutdown(stage, callback);
    return true;
}
//!End

//!DcfProcessLowerFrame
void Dcf::processLowerFrame(Packet *packet, Ptr<Ieee80211MacHeader>& header)
{
    // check if there is a running frame exchange sequence
    if (frameSequenceHandler->isSequenceRunning()) {
        // pass the packet to the frame sequence handler
        frameSequenceHandler->processResponse(packet);
        // cancel startRxTimer since we got the response packet in time
        cancelEvent(startRxTimer);
    }
    else if (isForUs(header)) // we don't have a running frame sequence
        // process packet individually
        recipientProcessReceivedFrame(packet, header);
    else {
        // drop the packet
    }
}
//!End

//!DcfRecipientProcessReceivedFrame
void Dcf::recipientProcessReceivedFrame(Packet *packet, Ptr<Ieee80211MacHeader>& header)
{
    if (auto dataOrMgmtHeader = dynamicPtrCast<Ieee80211DataOrMgmtHeader>(header))
        recipientAckProcedure->processReceivedFrame(packet, dataOrMgmtHeader, recipientAckPolicy, this);
    if (auto dataHeader = dynamicPtrCast<Ieee80211DataHeader>(header))
        sendUp(recipientDataService->dataFrameReceived(packet, dataHeader));
    else if (auto mgmtHeader = dynamicPtrCast<Ieee80211MgmtHeader>(header))
        sendUp(recipientDataService->managementFrameReceived(packet, mgmtHeader));
    else {
        sendUp(recipientDataService->controlFrameReceived(packet, header));
        recipientProcessControlFrame(packet, header);
        delete packet;
    }
}
//!End

} // namespace inet
*/
