//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCAPRECORDER_H
#define __INET_PCAPRECORDER_H

#include "inet/common/SimpleModule.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/recorder/IPcapWriter.h"

namespace inet {

/**
 * Dumps every packet using the IPacketWriter and PacketDump classes
 */
class INET_API PcapRecorder : public SimpleModule, protected cListener, public PacketDissector::ICallback
{
  public:
    class INET_API IHelper {
      public:
        virtual ~IHelper() {}

        /// returns pcapLinkType for given protocol or returns LINKTYPE_INVALID. Protocol storable as or convertable to pcapLinkType.
        virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const = 0;

        /// returns true when the protocol storable as pcapLinkType without conversion.
        virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const = 0;

        /// Create a new Packet or return nullptr. The new packet contains the original packet converted to pcapLinkType format.
        virtual Packet *tryConvertToLinkType(const Packet *packet, b frontOffset, b backOffset, PcapLinkType pcapLinkType, const Protocol *protocol) const = 0;
    };

  protected:
    typedef std::map<simsignal_t, Direction> SignalList;
    std::vector<const Protocol *> dumpProtocols;
    SignalList signalList;
    IPcapWriter *pcapWriter = nullptr;
    unsigned int snaplen = 0;
    bool dumpBadFrames = false;
    PacketFilter packetFilter;
    int numRecorded = 0;
    bool verbose = false;
    bool recordEmptyPackets = false;
    bool enableConvertingPackets = true;
    bool recordPcap = false;
    std::vector<IHelper *> helpers;
    PacketPrinter packetPrinter;

    static simsignal_t packetRecordedSignal;

    b frontOffset;
    b backOffset;
    const Protocol *dumpProtocol = nullptr;

  public:
    PcapRecorder();
    virtual ~PcapRecorder();
    virtual std::string resolveDirective(char directive) const override;

    virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override;

    virtual void startProtocolDataUnit(const Protocol *protocol) override { }
    virtual void endProtocolDataUnit(const Protocol *protocol) override { }
    virtual void markIncorrect() override { }

    virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void recordPacket(const cPacket *msg, Direction direction, cComponent *source);
    virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const;
    virtual Packet *tryConvertToLinkType(const Packet *packet, b frontOffset, b backOffset, PcapLinkType pcapLinkType, const Protocol *protocol) const;
    virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const;
    virtual void writePacket(const Protocol *protocol, const Packet *packet, b frontOffset, b backOffset, Direction direction, NetworkInterface *networkInterface);
};

} // namespace inet

#endif

