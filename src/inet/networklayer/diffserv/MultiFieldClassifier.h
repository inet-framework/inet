//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#ifndef __INET_MULTIFIELDCLASSIFIER_H
#define __INET_MULTIFIELDCLASSIFIER_H

#include "inet/common/INETDefs.h"
#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/Packet.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif

namespace inet {

/**
 * Absolute dropper.
 */
class INET_API MultiFieldClassifier : public queueing::PacketClassifierBase
{
  protected:
    class INET_API PacketDissectorCallback : public PacketDissector::ICallback
    {
      protected:
        bool dissect = true;
        bool matchesL3 = false;
        bool matchesL4 = false;

      public:
        int gateIndex = -1;

        L3Address srcAddr;
        int srcPrefixLength = 0;
        L3Address destAddr;
        int destPrefixLength = 0;
        int protocolId = -1;
        int dscp = -1;
        int tos = -1;
        int tosMask = 0;
        int srcPortMin = -1;
        int srcPortMax = -1;
        int destPortMin = -1;
        int destPortMax = -1;

      public:
        PacketDissectorCallback() {}
        virtual ~PacketDissectorCallback() {}

        bool matches(const Packet *packet);

        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override;
        virtual void startProtocolDataUnit(const Protocol *protocol) override {}
        virtual void endProtocolDataUnit(const Protocol *protocol) override {}
        virtual void markIncorrect() override {}
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

  protected:
    int numOutGates = 0;
    std::vector<PacketDissectorCallback> filters;

    int numRcvd = 0;

    static simsignal_t pkClassSignal;

  protected:
    void addFilter(const PacketDissectorCallback& filter);
    void configureFilters(cXMLElement *config);

  public:
    MultiFieldClassifier() {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void refreshDisplay() const override;

    virtual int classifyPacket(Packet *packet) override;
};

} // namespace inet

#endif // ifndef __INET_MULTIFIELDCLASSIFIER_H

