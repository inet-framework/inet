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

#ifndef __INET_PACKETPRINTER_H_
#define __INET_PACKETPRINTER_H_

#include <stack>
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/printer/ProtocolPrinter.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"

namespace inet {

class PacketPrinterContext {
  public:
    bool isCorrect = true;
    std::stringstream sourceColumn;
    std::stringstream destinationColumn;
    std::string protocolColumn;
    std::stringstream lengthColumn;
    std::stringstream infoColumn;
    int infoLevel = -1;
};

class INET_API PacketPrinter : public cMessagePrinter
{
  protected:
    virtual bool isEnabledOption(const Options *options, const char *name) const;
    virtual const ProtocolPrinter& getProtocolPrinter(const Protocol *protocol) const;
    virtual void printContext(std::ostream& stream, const Options *options, PacketPrinterContext& context) const;
    virtual void printPacketInsideOut(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolLevel, const Options *options, PacketPrinterContext& context) const;
    virtual void printPacketLeftToRight(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolLevel, const Options *options, PacketPrinterContext& context) const;
    virtual void printSignal(inet::physicallayer::Signal *signal, const Options *options, PacketPrinterContext& context) const;
    virtual void printPacket(Packet *packet, const Options *options, PacketPrinterContext& context) const;

  public:
    virtual int getScoreFor(cMessage *msg) const override;

    virtual std::set<std::string> getSupportedTags() const override;
    virtual std::set<std::string> getDefaultEnabledTags() const override;
    virtual std::vector<std::string> getColumnNames(const Options *options) const override;

    virtual void printMessage(std::ostream& stream, cMessage *message) const;
    virtual void printMessage(std::ostream& stream, cMessage *message, const Options *options) const override;

    virtual void printSignal(std::ostream& stream, inet::physicallayer::Signal *signal) const;
    virtual void printSignal(std::ostream& stream, inet::physicallayer::Signal *signal, const Options *options) const;

    virtual void printPacket(std::ostream& stream, Packet *packet) const;
    virtual void printPacket(std::ostream& stream, Packet *packet, const Options *options) const;

    virtual void printIeee80211MacChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printIeee80211MgmtChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printIeee80211PhyChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printIeee8022Chunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printIpv4Chunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printIcmpChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printTcpChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
};

} // namespace

#endif // #ifndef __INET_PACKETPRINTER_H_

