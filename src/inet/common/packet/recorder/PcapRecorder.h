//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2011 OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_PCAPRECORDER_H
#define __INET_PCAPRECORDER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/recorder/IPcapWriter.h"

namespace inet {

/**
 * Dumps every packet using the IPacketWriter and PacketDump classes
 */
class INET_API PcapRecorder : public cSimpleModule, protected cListener
{
  public:
    class INET_API IHelper {
      public:
        virtual ~IHelper() { }

        /// returns pcapLinkType for given protocol or returns LINKTYPE_INVALID. Protocol storable as or convertable to pcapLinkType.
        virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const = 0;

        /// returns true when the protocol storable as pcapLinkType without conversion.
        virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const = 0;

        /// Create a new Packet or return nullptr. The new packet contains the original packet converted to pcapLinkType format.
        virtual Packet *tryConvertToLinkType(const Packet* packet, PcapLinkType pcapLinkType, const Protocol *protocol) const = 0;
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
    bool recordPcap = false;
    std::vector<IHelper *> helpers;
    PacketPrinter packetPrinter;

    static simsignal_t packetRecordedSignal;

  public:
    PcapRecorder();
    virtual ~PcapRecorder();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void recordPacket(const cPacket *msg, Direction direction, cComponent *source);
    virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const;
    virtual Packet *tryConvertToLinkType(const Packet* packet, PcapLinkType pcapLinkType, const Protocol *protocol) const;
    virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const;
};

} // namespace inet

#endif

