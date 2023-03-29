//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

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
#include "inet/common/packet/recorder/PcapDump.h"
#include "inet/common/packet/ReorderBuffer.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/common/TimeTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/tun/TunSocket.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Socket.h"
#include "inet/networklayer/contract/ipv6/Ipv6Socket.h"
#include "inet/networklayer/contract/L3Socket.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
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
  packet->insertAtFront(header); // insert header into packet
  auto trailer = makeShared<MacTrailer>(); // create new trailer
  trailer->setChunkLength(B(4)); // set chunk length to 4 bytes
  trailer->setFcsMode(FCS_MODE_DECLARED); // set trailer fields
  packet->insertAtBack(trailer); // insert trailer into packet
}
//!End

//!PacketDecapsulationExample
void Mac::decapsulate(Packet *packet)
{
  auto header = packet->popAtFront<MacHeader>(); // pop header from packet
  auto lengthField = header->getLengthField();
  cout << header->getChunkLength() << endl; // print chunk length
  cout << lengthField << endl; // print header length field
  cout << header->getReceiverAddress() << endl; // print other header fields
  auto trailer = packet->popAtBack<MacTrailer>(); // pop trailer from packet
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
    fragment->insertAtFront(header); // insert header into fragment
    auto data = packet->peekAt(offset, size); // get data part from packet
    fragment->insertAtBack(data); // insert data part into fragment
    auto trailer = makeShared<MacTrailer>(); // create new trailer
    fragment->insertAtBack(trailer); // insert trailer into fragment
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
    fragment->popAtFront<MacHeader>(); // pop header from fragment
    fragment->popAtBack<MacTrailer>(); // pop trailer from fragment
    packet->insertAtBack(fragment->peekData()); // concatenate fragment data
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
    aggregate->insertAtBack(header); // insert subheader into aggregate
    auto data = packet->peekData(); // get packet data
    aggregate->insertAtBack(data); // insert data into aggregate
  }
  auto header = makeShared<MacHeader>(); // create new header
  header->setAggregate(true); // set aggregate flag
  aggregate->insertAtFront(header); // insert header into aggregate
  auto trailer = makeShared<MacTrailer>(); // create new trailer
  aggregate->insertAtBack(trailer); // insert trailer into aggregate
  return aggregate;
}
//!End

//!PacketDisaggregationExample
vector<Packet *> *Mac::disaggregate(Packet *aggregate)
{
  aggregate->popAtFront<MacHeader>(); // pop header from packet
  aggregate->popAtBack<MacTrailer>(); // pop trailer from packet
  vector<Packet *> *packets = new vector<Packet *>(); // result collection
  b offset = aggregate->getFrontOffset(); // start after header
  while (offset != aggregate->getBackOffset()) { // up to trailer
    auto header = aggregate->peekAt<SubHeader>(offset); // peek sub header
    offset += header->getChunkLength(); // increment with header length
    auto size = header->getLengthField(); // get length field from header
    auto data = aggregate->peekAt(offset, size); // peek following data part
    auto packet = new Packet("Original"); // create new packet
    packet->insertAtBack(data); // insert data into packet
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
  auto data = packet->peekAllAsBytes(); // convert to a sequence of bytes
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

class TransportHeader : public FieldsChunk
{
public:
  B getSequenceNumber() const;
  void setSequenceNumber(B sequenceNumber);
};

//!PacketQueueingExample
class TransportSendQueue
{
  ChunkQueue queue; // stores application data
  B sequenceNumber; // position in stream

  void enqueueApplicationData(Packet *packet);
  Packet *createSegment(b length);
};

void TransportSendQueue::enqueueApplicationData(Packet *packet)
{
  queue.push(packet->peekData()); // store received data
}

Packet *TransportSendQueue::createSegment(b maxLength)
{
  auto packet = new Packet("Segment"); // create new segment
  auto header = makeShared<TransportHeader>(); // create new header
  header->setSequenceNumber(sequenceNumber); // store sequence number for reordering
  packet->insertAtFront(header); // insert header into segment
  if (queue.getLength() < maxLength)
    maxLength = queue.getLength(); // reduce length if necessary
  auto data = queue.pop(maxLength); // pop requested amount of data
  packet->insertAtBack(data); // insert data into segment
  sequenceNumber += data->getChunkLength(); // increase sequence number
  return packet;
}
//!End

//!PacketReorderingExample
class TransportReceiveQueue
{
  ReorderBuffer buffer; // stores receive data
  B sequenceNumber;

  void processSegment(Packet *packet);
  Packet *getAvailableData();
};

void TransportReceiveQueue::processSegment(Packet *packet)
{
  auto header = packet->popAtFront<TransportHeader>(); // pop transport header
  auto sequenceNumber = header->getSequenceNumber();
  auto data = packet->peekData(); // get all packet data
  buffer.replace(sequenceNumber, data); // overwrite data in buffer
}

Packet *TransportReceiveQueue::getAvailableData()
{
  if (buffer.getAvailableDataLength() == b(0)) // if no data available
    return nullptr;
  auto data = buffer.popAvailableData(); // remove all available data
  return new Packet("Data", data);
}
//!End

class NetworkProtocolHeader : public FieldsChunk
{
public:
  b getFragmentOffset() const;
};

//!PacketReassemblingExample
class NetworkProtocolDefragmentation
{
  ReassemblyBuffer buffer; // stores received data

  void processDatagram(Packet *packet); // processes incoming packes
  Packet *getReassembledDatagram(); // reassembles the original packet
};

void NetworkProtocolDefragmentation::processDatagram(Packet *packet)
{
  auto header = packet->popAtFront<NetworkProtocolHeader>(); // remove header
  auto fragmentOffset = header->getFragmentOffset(); // determine offset
  auto data = packet->peekData(); // get data from packet
  buffer.replace(fragmentOffset, data); // overwrite data in buffer
}

Packet *NetworkProtocolDefragmentation::getReassembledDatagram()
{
  if (!buffer.isComplete()) // if reassembly isn't complete
    return nullptr; // there's nothing to return
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

//!CorruptingPacketsExample
Packet *ErrorModel::corruptPacket(Packet *packet, double ber)
{
  auto length = packet->getTotalLength();
  auto hasErrors = hasProbabilisticError(length, ber); // decide randomly
  auto corruptedPacket = packet->dup(); // cheap operation
  corruptedPacket->setBitError(hasErrors); // set bit error flag
  return corruptedPacket;
}
//!End

//!CorruptingChunksExample
Packet *ErrorModel::corruptChunks(Packet *packet, double ber)
{
  b offset = b(0); // start from the beginning
  auto corruptedPacket = new Packet("Corrupt"); // create new packet
  while (auto chunk = packet->peekAt(offset)->dupShared()) { // for each chunk
    auto length = chunk->getChunkLength();
    auto hasErrors = hasProbabilisticError(length, ber); // decide randomly
    if (hasErrors) // if erroneous
      chunk->markIncorrect(); // set incorrect bit
    corruptedPacket->insertAtBack(chunk); // append chunk to corrupt packet
    offset += chunk->getChunkLength(); // increment offset with chunk length
  }
  return corruptedPacket;
}
//!End

//!CorruptingBytesExample
Packet *ErrorModel::corruptBytes(Packet *packet, double ber)
{
  vector<uint8_t> corruptedBytes; // bytes of corrupted packet
  auto data = packet->peekAllAsBytes(); // data of original packet
  for (auto byte : data->getBytes()) { // for each original byte do
    if (hasProbabilisticError(B(1), ber)) // if erroneous
      byte = ~byte; // invert byte (simplified corruption)
    corruptedBytes.push_back(byte); // store byte in corrupted data
  }
  auto corruptedData = makeShared<BytesChunk>(); // create new data
  corruptedData->setBytes(corruptedBytes); // store corrupted bits
  return new Packet("Corrupt", corruptedData); // create new packet
}
//!End

Packet *ErrorModel::corruptBits(Packet *packet, double ber)
{
  vector<bool> corruptedBits; // bits of corrupted packet
  auto data = packet->peekAllAsBits(); // data of original packet
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

class ClientApp
{
  private:
  TcpSocket socket;

public:
  void send();
};

class ServerApp
{
  private:
  TcpSocket socket;

public:
  void receive(Packet *packet);
  void updateStatistic(b offset, b length, simtime_t age);
};

//!RegionTaggingSendExample
void ClientApp::send()
{
  auto data = makeShared<ByteCountChunk>(); // create new data chunk
  auto creationTimeTag = data->addTag<CreationTimeTag>(); // add new tag
  creationTimeTag->setCreationTime(simTime()); // store current time
  auto packet = new Packet("Data", data); // create new packet
  socket.send(packet); // send packet using TCP socket
}
//!End

//!RegionTaggingReceiveExample
void ServerApp::receive(Packet *packet)
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
  void initialize(int stage);
  const Protocol *getNextProtocol(Packet *packet);
  void sendDown(Packet *packet, Ipv4Address nextHopAddress, int interfaceId);
  MacAddress resolveMacAddress(Ipv4Address a);
};

//!ProtocolRegistrationExample
void Ipv4::initialize(int stage)
{
    if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::ipv4, gate("transportIn"), gate("transportOut"));
        registerProtocol(Protocol::ipv4, gate("queueOut"), gate("queueIn"));
    }
}
//!End

//!NextProtocolExample
const Protocol *Ipv4::getNextProtocol(Packet *packet)
{
    auto ipv4Header = packet->peekAtFront<Ipv4Header>();
    auto ipProtocolId = ipv4Header->getProtocolId();
    return ProtocolGroup::getIpProtocolGroup()->getProtocol(ipProtocolId);
}
//!End

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
  req->setProtocol(&Protocol::ipv4); // set destination protocol
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
sequence->insertAtBack(makeShared<UdpHeader>()); // append UDP header
sequence->insertAtBack(makeShared<ByteCountChunk>(B(10), 0)); // 10 bytes
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
sequence->insertAtBack(firstHalf); // append first half
sequence->insertAtBack(secondHalf); // append second half
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
dataPacket->insertAtBack(moreData); // insert more data at the end
auto udpHeader = makeShared<UdpHeader>(); // create new UDP header
dataPacket->insertAtFront(udpHeader); // insert header into packet
//!End
  delete emptyPacket;
}

void packetProcessingExample()
{
  auto packet = new Packet();
//!PacketProcessingExample
packet->popAtFront<MacHeader>(); // pop specific header from packet
packet->popAtBack<MacTrailer>(); // pop specific trailer from packet
auto data = packet->peekData(); // peek remaining data in packet
//!End
}

/*
void signalConstructionExample()
{
  double duration = 0;
  auto packet = new Packet();
  ITransmission *transmission = nullptr;
//!SignalConstructionExample
auto signal = new Signal(transmission);
signal->setDuration(duration);
signal->encapsulate(packet);
//!End
}
*/

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
void dissectPacket(Packet *packet, Protocol *protocol);
//!End
};

void packetDissectionExample()
{
  auto packet = new Packet();
  PacketDissectorCallback callback;
//!PacketDissectionExample
auto& registry = ProtocolDissectorRegistry::getInstance();
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
PacketFilter filter; // patterns for the whole packet and for the data
filter.setPattern("ping*", "Ipv4Header and sourceAddress(10.0.0.*)");
filter.matches(packet); // returns boolean value
//!End
}

void packetPrintingExample()
{
  auto packet = new Packet();
//!PacketPrintingExample
PacketPrinter printer; // turns packets into human readable strings
printer.printPacket(std::cout, packet); // print to standard output
//!End
}

void pcapRecordingExample()
{
    auto packet = new Packet();
//!PCAPRecoringExample
PcapDump dump;
dump.openPcap("out.pcap", 65535, 0); // maximum length and PCAP type
dump.writePacket(simTime(), packet); // record with current time
//!End
}

class App : public cSimpleModule, public UdpSocket::ICallback, public TcpSocket::ICallback
{
public:
  void socketConfigureExample();
  void socketSendExample();
  void socketProcessExample();
  void socketReceiveExample();
  void socketFindExample();
  void socketCloseExample();
  void udpSocketExample();
  void tcpSocketExample();
  void sctpSocketExample();
  void ipv4SocketExample();
  void ipv6SocketExample();
  void l3SocketExample();
  void tunSocketExample();
};

void App::socketConfigureExample()
{
    UdpSocket socket;
//!SocketConfigureExample
socket.setOutputGate(gate("socketOut")); // configure socket output gate
socket.setCallback(this); // set callback interface for message processing
//!End
}

void App::socketSendExample()
{
    UdpSocket socket;
    auto packet = new Packet();
//!SocketSendExample
socket.send(packet); // by means of the underlying communication protocol
//!End
}

//!SocketCallbackInterfaceExample
class ICallback // usually the inner class of the socket
{
    void socketDataArrived(ISocket *socket, Packet *packet);
};
//!End

void App::socketProcessExample()
{
    ISocket& socket = *new Ipv4Socket();
    cMessage *message = nullptr;
//!SocketProcessExample
if (socket.belongsToSocket(message)) // match message and socket
    socket.processMessage(message); // invoke callback interface
//!End
}

namespace socket {
//!SocketReceiveExample
class App : public cSimpleModule, public ICallback
{
    void socketDataArrived(ISocket *socket, Packet *packet);
};

void App::socketDataArrived(ISocket *socket, Packet *packet)
{
    EV << packet->peekData() << endl;
}
//!End
} // namespace socket

void App::socketFindExample()
{
    cMessage *message = nullptr;
    SocketMap socketMap;
//!SocketFindExample
auto socket = socketMap.findSocketFor(message); // lookup socket to process
socket->processMessage(message); // dispatch message to callback interface
//!End
}

void App::socketCloseExample()
{
    TcpSocket socket;
//!SocketCloseExample
socket.close(); // release allocated local and remote network resources
//!End
}

void App::udpSocketExample()
{
    UdpSocket socket;
//!UdpSocketBindExample
socket.bind(Ipv4Address("10.0.0.42"), 42); // local address/port
//!End
//!UdpSocketConnectExample
socket.connect(Ipv4Address("10.0.0.42"), 42); // remote address/port
//!End
//!UdpSocketConfigureExample
socket.setTimeToLive(16); // change default TTL
socket.setBroadcast(true); // receive all broadcasts
socket.joinMulticastGroup(Ipv4Address("224.0.0.9")); // receive multicasts
//!End
    auto packet1 = new Packet();
    auto packet42 = new Packet();
//!UdpSocketSendToExample
socket.sendTo(packet42, Ipv4Address("10.0.0.42"), 42); // remote address/port
//!End
//!UdpSocketSendExample
socket.connect(Ipv4Address("10.0.0.42"), 42); // remote address/port
socket.send(packet1); // send packets via connected socket
// ...
socket.send(packet42);
//!End
}

namespace udp {
//!UdpSocketCallbackInterface
class ICallback // inner class of UdpSocket
{
    void socketDataArrived(UdpSocket *socket, Packet *packet);
    void socketErrorArrived(UdpSocket *socket, Indication *indication);
};
//!End
} // namespace

//!UdpSocketReceiveExample
class UdpApp : public cSimpleModule, public UdpSocket::ICallback
{
    void socketDataArrived(UdpSocket *socket, Packet *packet);
};

void UdpApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    EV << packet->peekData() << endl;
}
//!End

void App::tcpSocketExample()
{
TcpSocket socket;
//!TcpSocketListenExample
socket.bind(Ipv4Address("10.0.0.42"), 42); // local address/port
socket.listen(); // start listening for incoming connections
//!End
//!TcpSocketConnectExample
socket.connect(Ipv4Address("10.0.0.42"), 42); // remote address/port
//!End
//!TcpSocketConfigureExample
socket.setTCPAlgorithmClass("TcpReno");
//!End
    auto packet1 = new Packet();
    auto packet42 = new Packet();
//!TcpSocketSendExample
socket.send(packet1);
// ...
socket.send(packet42);
//!End
}

//!TcpSocketAcceptExample
class TcpServerApp : public cSimpleModule, public TcpSocket::ICallback
{
    TcpSocket serverSocket; // server socket listening for connections
    SocketMap socketMap; // container for all accepted connections

    void socketAvailable(TcpSocket *socket, TcpAvailableInfo *info);
};

void TcpServerApp::socketAvailable(TcpSocket *socket, TcpAvailableInfo *info)
{
    auto newSocket = new TcpSocket(info); // create socket using received info
    // ...
    socketMap.addSocket(newSocket); // store accepted connection
    serverSocket.accept(info->getNewSocketId()); // notify Tcp module
}
//!End

namespace tcp {
//!TcpSocketCallbackInterface
class ICallback // inner class of TcpSocket
{
    void socketDataArrived(TcpSocket* socket, Packet *packet, bool urgent);
    void socketAvailable(TcpSocket *socket, TcpAvailableInfo *info);
    void socketEstablished(TcpSocket *socket);
    // ...
    void socketClosed(TcpSocket *socket);
    void socketFailure(TcpSocket *socket, int code);
};
//!End
} // namespace

//!TcpSocketReceiveExample
class TcpApp : public cSimpleModule, public TcpSocket::ICallback
{
    void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent);
};

void TcpApp::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    EV << packet->peekData() << endl;
}
//!End

void App::sctpSocketExample()
{
SctpSocket socket;
//!SctpSocketConfigureExample
socket.setOutboundStreams(2);
socket.setStreamPriority(1);
socket.setEnableHeartbeats(true);
// ...
//!End
//!SctpSocketListenExample
socket.bind(Ipv4Address("10.0.0.42"), 42); // local address/port
socket.listen(true); // start listening for incoming connections
//!End
//!SctpSocketConnectExample
socket.connect(Ipv4Address("10.0.0.42"), 42);
//!End
auto packet1 = new Packet();
auto packet42 = new Packet();
//!SctpSocketSendExample
socket.send(packet1);
// ...
socket.send(packet42);
//!End
}

//!SctpSocketAcceptExample
class SctpServerApp : public cSimpleModule, public SctpSocket::ICallback
{
    SocketMap socketMap;
    SctpSocket serverSocket;

    void socketAvailable(SctpSocket *socket, SctpAvailableInfo *info);
};

void SctpServerApp::socketAvailable(SctpSocket *socket, SctpAvailableInfo *info)
{
    auto newSocket = new SctpSocket(info);
    // ...
    socketMap.addSocket(newSocket);
    serverSocket.accept(info->getNewSocketId());
}
//!End

namespace sctp {
//!SctpSocketCallbackInterface
class ICallback // inner class of SctpSocket
{
    void socketDataArrived(SctpSocket* socket, Packet *packet, bool urgent);
    void socketEstablished(SctpSocket *socket);
    // ...
    void socketClosed(SctpSocket *socket);
    void socketFailure(SctpSocket *socket, int code);
};
//!End
} // namespace

//!SctpSocketReceiveExample
class SctpApp : public cSimpleModule, public SctpSocket::ICallback
{
    void socketDataArrived(SctpSocket *socket, Packet *packet, bool urgent);
};

void SctpApp::socketDataArrived(SctpSocket *socket, Packet *packet, bool urgent)
{
    EV << packet->peekData() << endl;
}
//!End

void App::ipv4SocketExample()
{
    Ipv4Socket socket;
//!Ipv4SocketBindExample
socket.bind(&Protocol::icmpv4, Ipv4Address()); // filter for ICMPv4 messages
//!End
//!Ipv4SocketConnectExample
socket.connect(Ipv4Address("10.0.0.42")); // filter for remote address
//!End
    auto packet = new Packet();
    auto packet1 = new Packet();
    auto packet42 = new Packet();
//!Ipv4SocketSendToExample
socket.sendTo(packet, Ipv4Address("10.0.0.42")); // remote address
//!End
//!Ipv4SocketSendExample
socket.connect(Ipv4Address("10.0.0.42")); // remote address
socket.send(packet1);
// ...
socket.send(packet42);
//!End
}

namespace ipv4 {
//!Ipv4SocketCallbackInterface
class ICallback // inner class of Ipv4Socket
{
    void socketDataArrived(Ipv4Socket *socket, Packet *packet);
};
//!End
} // namespace ipv4

//!Ipv4SocketReceiveExample
class Ipv4App : public cSimpleModule, public Ipv4Socket::ICallback
{
    void socketDataArrived(Ipv4Socket *socket, Packet *packet);
};

void Ipv4App::socketDataArrived(Ipv4Socket *socket, Packet *packet)
{
    EV << packet->peekData() << endl;
}
//!End

void App::ipv6SocketExample()
{
    Ipv6Socket socket;
//!Ipv6SocketBindExample
socket.bind(&Protocol::icmpv6, Ipv6Address()); // filter for ICMPv6 messages
//!End
//!Ipv6SocketConnectExample
socket.connect(Ipv6Address("10:0:0:0:0:0:0:42")); // filter for remote address
//!End
    auto packet = new Packet();
    auto packet1 = new Packet();
    auto packet42 = new Packet();
//!Ipv6SocketSendAtExample
socket.sendTo(packet, Ipv6Address("10:0:0:0:0:0:0:42")); // remote address
//!End
//!Ipv6SocketSendExample
socket.connect(Ipv6Address("10:0:0:0:0:0:0:42")); // remote address
socket.send(packet1);
// ...
socket.send(packet42);
//!End
}

namespace ipv6 {
//!Ipv6SocketCallbackInterface
class ICallback // inner class of Ipv6Socket
{
    void socketDataArrived(Ipv6Socket *socket, Packet *packet);
};
//!End
} // namespace ipv6

//!Ipv6SocketReceiveExample
class Ipv6App : public cSimpleModule, public Ipv6Socket::ICallback
{
    void socketDataArrived(Ipv6Socket *socket, Packet *packet);
};

void Ipv6App::socketDataArrived(Ipv6Socket *socket, Packet *packet)
{
    EV << packet->peekData() << endl;
}
//!End

void App::l3SocketExample()
{
//!L3SocketProtocolExample
L3Socket socket(&Protocol::flooding);
//!End
//!L3SocketBindExample
socket.bind(&Protocol::echo, ModuleIdAddress(42));
//!End
//!L3SocketConnectExample
socket.connect(ModuleIdAddress(42)); // filter for remote interface
//!End
    auto packet = new Packet();
    auto packet1 = new Packet();
    auto packet42 = new Packet();
//!L3SocketSendToExample
socket.sendTo(packet, ModuleIdAddress(42)); // remote interface
//!End
//!L3SocketSendExample
socket.connect(ModuleIdAddress(42)); // remote interface
socket.send(packet1);
//..
socket.send(packet42);
//!End
}

namespace l3 {
//!L3SocketCallbackInterface
class ICallback // inner class of L3Socket
{
    void socketDataArrived(L3Socket *socket, Packet *packet);
};
//!End
} // namespace l3

//!L3SocketReceiveExample
class L3App : public cSimpleModule, public L3Socket::ICallback
{
    void socketDataArrived(L3Socket *socket, Packet *packet);
};

void L3App::socketDataArrived(L3Socket *socket, Packet *packet)
{
    EV << packet->peekData() << endl;
}
//!End

void App::tunSocketExample()
{
    TunSocket socket;
    NetworkInterface *interface = nullptr;
//!TunSocketOpenExample
socket.open(interface->getId());
//!End
    auto packet = new Packet();
//!TunSocketSendExample
socket.send(packet);
//!End
}

namespace tun {
//!TunSocketCallbackInterface
class ICallback // inner class of TunSocket
{
    void socketDataArrived(TunSocket *socket, Packet *packet);
};
//!End
} // namespace tun

//!TunSocketReceiveExample
class TunApp : public cSimpleModule, public TunSocket::ICallback
{
    void socketDataArrived(TunSocket *socket, Packet *packet);
};

void TunApp::socketDataArrived(TunSocket *socket, Packet *packet)
{
    EV << packet->peekData() << endl;
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

/*
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
  auto transmittedHeader = packet->peekAtFront<Ieee80211MacHeader>();
  auto dataOrMgmtHeader = dynamicPtrCast<Ieee80211DataOrMgmtHeader>(transmittedHeader);
  if (dataOrMgmtHeader) {
    if (originatorAckPolicy->isAckNeeded(dataOrMgmtHeader)) {
      ackHandler->processTransmittedDataOrMgmtFrame(dataOrMgmtHeader);
    }
  }
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
*/

} // namespace inet

