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

#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/Serializer.h"
#include "NewTest_m.h"

namespace inet {

class ApplicationHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const override;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
};

class TcpHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const override;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
};

class IpHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const override;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
};

class EthernetHeaderSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const override;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
};

class EthernetTrailerSerializer : public FieldsChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const override;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
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
    std::shared_ptr<TcpHeader> createTcpHeader();

  public:
    NewSender(NewMedium& medium) : medium(medium) { }
    ~NewSender() { delete tcpSegment; }

    void sendPackets();
};

class NewReceiver
{
  protected:
    NewMedium& medium;
    ChunkQueue applicationData;

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

} // namespace

#endif // #ifndef __NEWTEST_H_

