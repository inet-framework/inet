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

#ifndef __NEWTEST_H_
#define __NEWTEST_H_

#include "inet/common/packet/Buffer.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/Serializer.h"
#include "NewTest_m.h"

class ApplicationHeaderSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const override {
        int64_t position = stream.getPosition();
        stream.writeInt16(static_cast<const ApplicationHeader&>(chunk).getSomeData());
        stream.writeByteRepeatedly(0, chunk.getByteLength() - stream.getPosition() + position);
    }

    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        int64_t position = stream.getPosition();
        static_cast<ApplicationHeader&>(chunk).setSomeData(stream.readInt16());
        stream.readByteRepeatedly(0, chunk.getByteLength() - stream.getPosition() + position);
    }
};

class TcpHeaderSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const override {
        int64_t position = stream.getPosition();
        auto& tcpHeader = static_cast<const TcpHeader&>(chunk);
        if (tcpHeader.getBitError() != BIT_ERROR_CRC)
            throw cRuntimeError("Cannot serialize TCP header");
        stream.writeInt16(tcpHeader.getLengthField());
        stream.writeInt16(tcpHeader.getSrcPort());
        stream.writeInt16(tcpHeader.getDestPort());
        stream.writeInt16(tcpHeader.getCrc());
        stream.writeByteRepeatedly(0, chunk.getByteLength() - stream.getPosition() + position);
    }

    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        int64_t position = stream.getPosition();
        auto& tcpHeader = static_cast<TcpHeader&>(chunk);
        int64_t remainingSize = stream.getRemainingSize();
        int16_t lengthField = stream.readInt16();
        if (lengthField > remainingSize)
            chunk.makeIncomplete();
        int16_t byteLength = std::min(lengthField, (int16_t)remainingSize);
        tcpHeader.setByteLength(byteLength);
        tcpHeader.setLengthField(lengthField);
        tcpHeader.setSrcPort(stream.readInt16());
        tcpHeader.setDestPort(stream.readInt16());
        tcpHeader.setBitError(BIT_ERROR_CRC);
        tcpHeader.setCrc(stream.readInt16());
        stream.readByteRepeatedly(0, byteLength - stream.getPosition() + position);
    }
};

class IpHeaderSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const override {
        int64_t position = stream.getPosition();
        stream.writeInt16((int16_t)static_cast<const IpHeader&>(chunk).getProtocol());
        stream.writeByteRepeatedly(0, chunk.getByteLength() - stream.getSize() + position);
    }

    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        int64_t position = stream.getPosition();
        static_cast<IpHeader&>(chunk).setProtocol((Protocol)stream.readInt16());
        stream.readByteRepeatedly(0, chunk.getByteLength() - stream.getPosition() + position);
    }
};

class EthernetHeaderSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const Chunk &chunk) const override {
        int64_t position = stream.getPosition();
        stream.writeInt16((int16_t)static_cast<const EthernetHeader&>(chunk).getProtocol());
        stream.writeByteRepeatedly(0, chunk.getByteLength() - stream.getPosition() + position);
    }

    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        int64_t position = stream.getPosition();
        static_cast<EthernetHeader&>(chunk).setProtocol((Protocol)stream.readInt16());
        stream.readByteRepeatedly(0, chunk.getByteLength() - stream.getPosition() + position);
    }
};

class CompoundHeaderSerializer : public SequenceChunkSerializer
{
  public:
    virtual void deserialize(ByteInputStream& stream, Chunk &chunk) override {
        auto& compoundHeader = static_cast<CompoundHeader&>(chunk);
        IpHeaderSerializer ipHeaderSerializer;
        auto ipHeader = std::make_shared<IpHeader>();
        ipHeaderSerializer.deserialize(stream, *ipHeader);
        compoundHeader.append(ipHeader);
    }
};

class NewMedium
{
  protected:
    bool serialize = false;
    std::vector<Packet *> packets;

  protected:
    Packet *serializePacket(Packet *packet);

  public:
    NewMedium(bool serialize) : serialize(serialize) { }
    ~NewMedium() { for (auto packet : packets) delete packet; }

    bool getSerialize() const { return serialize; }

    void sendPacket(Packet *packet);
    const std::vector<Packet *> receivePackets();
};

class NewSender
{
  protected:
    NewMedium& medium;
    Packet *tcpSegment = nullptr;

  protected:
    void sendEthernet(Packet *packet);
    void sendIp(Packet *packet);
    void sendTcp(Packet *packet);
    Packet *createTcpSegment();

  public:
    NewSender(NewMedium& medium) : medium(medium) { }
    ~NewSender() { delete tcpSegment; }

    void sendPackets();
};

class NewReceiver
{
  protected:
    NewMedium& medium;
    Buffer applicationData;
    int applicationDataPosition = 0;

  protected:
    void receiveApplication(Packet *packet);
    void receiveTcp(Packet *packet);
    void receiveIp(Packet *packet);
    void receiveEthernet(Packet *packet);

  public:
    NewReceiver(NewMedium& medium) : medium(medium) { }
    ~NewReceiver() { }

    void receivePackets();
};

class NewTest : public cSimpleModule
{
  protected:
    double runtime = -1;

  protected:
    void initialize() override;
    void finish() override;
};

#endif // #ifndef __NEWTEST_H_

